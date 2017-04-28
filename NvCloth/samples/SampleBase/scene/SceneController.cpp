/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "SceneController.h"
#include "RenderUtils.h"
#include "Utils.h"

#include "Renderer.h"

#include "CommonUIController.h"

// initialization of NvCloth dll
#include <NvCloth/Callbacks.h>

// example allocator callbacks
#include "CallbackImplementations.h"

// low level cloth
//#include <NvCloth/Factory.h>
#include <NvCloth/Fabric.h>
#include <NvCloth/Cloth.h>
#include <NvCloth/Solver.h>

#include <NvClothExt/ClothFabricCooker.h>
#include "ClothMeshGenerator.h"

#include <algorithm>
#include <imgui.h>
#include <sstream>
#include <tuple>

#include "scene/Scene.h"
#include "scene/scenes/SimpleScene.h"
#include "scene/scenes/WindScene.h"

#include "utils/DebugLineRenderBuffer.h"

JobManager SceneController::sJobManager;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//													Controller
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SceneController::SceneController() : mTimeScale(1.0f), mStartDelay(0.f)
{
	mActivePlatform = (int)nv::cloth::Platform::CPU;
	mCUDAInitialized = false;
	mDXInitialized = false;
	mLeftOverTime = 0.0;
	mPaused = false;
	mSingleStep = 0;
}

SceneController::~SceneController()
{
}

void SceneController::onSampleStart()
{
	// setup camera
	CFirstPersonCamera* camera = &getRenderer().getCamera();
	DirectX::XMVECTORF32 lookAtPt = { 0, 10, 0, 0 };
	DirectX::XMVECTORF32 eyePt = { 0, 15, 25, 0 };
	camera->SetViewParams(eyePt, lookAtPt);
	camera->SetRotateButtons(false, false, true, false);
	camera->SetEnablePositionMovement(true);

	mStartDelay = 3.f;

	mPhysXPrimitiveRenderMaterial = new RenderMaterial(getRenderer().getResourceManager(), "physx_primitive", "");
	mPhysXPlaneRenderMaterial = new RenderMaterial(getRenderer().getResourceManager(), "physx_primitive_plane", "");

	/*{
		IRenderMesh* mesh = getRenderer().getPrimitiveRenderMesh(PrimitiveRenderMeshType::Plane);
		mPlane = getRenderer().createRenderable(*mesh, *mPhysXPlaneRenderMaterial);
		mPlane->setTransform(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxPiDivTwo, PxVec3(0.f, 0.f, 1.f))));
		mPlane->setScale(PxVec3(1000.f));
	}*/

	for (uint32_t i = 0; i < 0; ++i)
	{
		IRenderMesh* mesh = getRenderer().getPrimitiveRenderMesh(PrimitiveRenderMeshType::Box);
		auto box = getRenderer().createRenderable(*mesh, *mPhysXPrimitiveRenderMaterial);
		box->setColor(getRandomPastelColor());
		box->setScale(PxVec3(0.2f));
		mBoxes.push_back(box);
	}


	///

	mActiveScene = new SimpleScene(this);
	mActiveScene->onInitialize();
	mActiveSceneIndex = 0;
}

void SceneController::onInitialize()
{
	NvClothEnvironment::AllocateEnv();

	mFactories[(int)nv::cloth::Platform::CPU] = NvClothCreateFactoryCPU();
	mFactories[(int)nv::cloth::Platform::CUDA] = nullptr;
	mFactories[(int)nv::cloth::Platform::DX11] = nullptr;
	
	//CUDA
	do
	{
		assert(mCUDAInitialized == false);
		cuInit(0);
		int deviceCount = 0;
		mCUDAInitialized = true;
		mCUDAInitialized = (cuDeviceGetCount(&deviceCount) == CUDA_SUCCESS);
		mCUDAInitialized &= deviceCount >= 1;
		if(!mCUDAInitialized)
			break;
		mCUDAInitialized = cuCtxCreate(&mCUDAContext, 0, 0) == CUDA_SUCCESS;
		if(!mCUDAInitialized)
			break;

		mFactories[(int)nv::cloth::Platform::CUDA] = NvClothCreateFactoryCUDA(mCUDAContext);
	} while(false);

	//DX11
	do
	{
		assert(mDXInitialized == false);
		mDXInitialized = true;
		mDXDevice = GetDeviceManager()->GetDevice();

		mGraphicsContextManager = new DxContextManagerCallbackImpl(mDXDevice);
		mDXInitialized &= mGraphicsContextManager != nullptr;
		if(!mDXInitialized)
			break;

		mFactories[(int)nv::cloth::Platform::DX11] = NvClothCreateFactoryDX11(mGraphicsContextManager);
		mDXInitialized &= mFactories[(int)nv::cloth::Platform::DX11] != nullptr;
	} while(false);

	mDebugLineRenderBuffer = new DebugLineRenderBuffer;
}

void SceneController::onSampleStop()
{
	mActiveScene->onTerminate();
	delete mActiveScene;

	for (auto b : mBoxes)
		SAFE_DELETE(b);

	mBoxes.clear();

	NvClothDestroyFactory(mFactories[(int)nv::cloth::Platform::CPU]);
	NvClothDestroyFactory(mFactories[(int)nv::cloth::Platform::CUDA]);
	NvClothDestroyFactory(mFactories[(int)nv::cloth::Platform::DX11]);

	//SAFE_DELETE(mPlane);
	SAFE_DELETE(mPhysXPlaneRenderMaterial);
	SAFE_DELETE(mPhysXPrimitiveRenderMaterial);
}

void SceneController::onTerminate() 
{
}

void SceneController::changeScene(int index)
{
	mActiveScene->onTerminate();
	delete mActiveScene;

	mActiveSceneIndex = index;

	if(index < Scene::GetSceneCount())
		mActiveScene = Scene::CreateScene(index, this);
	else
	{
		mActiveSceneIndex = 0;
		mActiveScene = Scene::CreateScene(0, this);
	}
	mActiveScene->onInitialize();
}

void SceneController::Animate(double dt)
{
	if(mPaused && (mSingleStep <= 0))
	{
		//Re render debug lines from last frame
		getRenderer().queueRenderBuffer(mDebugLineRenderBuffer);
		return;
	}
	if(mSingleStep > 0)
	{
		mSingleStep--;
		dt = 1.0 / 60.0;
	}

	mDebugLineRenderBuffer->clear();

	mLeftOverTime += dt * mTimeScale;

	double simulationStep = 0.0;
	while(mLeftOverTime > 1.0 / 60.0)
		simulationStep += 1.0 / 60.0, mLeftOverTime -= 1.0 / 60.0;
	if(simulationStep >= 1.0 / 60.0)
		simulationStep = 1.0 / 60.0;

	mActiveScene->Animate(simulationStep);

	getRenderer().queueRenderBuffer(mDebugLineRenderBuffer);
}

LRESULT SceneController::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(mActiveScene->HandleEvent(uMsg, wParam, lParam))
		return 1;

	if (uMsg == WM_KEYDOWN)
	{
		int iKeyPressed = static_cast<int>(wParam);

		if(iKeyPressed >= '1' && iKeyPressed <= '9')
		{
			changeScene(iKeyPressed - '1');
		}

		switch (iKeyPressed)
		{
		case 'P':
			mPaused = !mPaused;
			break;
		case 'O':
			mSingleStep++;
			break;
		default:
			break; 
		}
	}

	return 1;
}

void SceneController::drawUI()
{
	///////////////////////////////////////////////////////////////////////////////////////////
	// Cube
	///////////////////////////////////////////////////////////////////////////////////////////
	{
		ImGui::Text("Time Scale");
		ImGui::DragFloat("Scale", &mTimeScale, 0.1f, 0.0f, 100.0f);
	}

	bool pressed = false;

	pressed = pressed | ImGui::RadioButton("CPU", &mActivePlatform, (int)nv::cloth::Platform::CPU); ImGui::SameLine();
	pressed = pressed | ImGui::RadioButton("DX11", &mActivePlatform, (int)nv::cloth::Platform::DX11); ImGui::SameLine();
	pressed = pressed | ImGui::RadioButton("CUDA", &mActivePlatform, (int)nv::cloth::Platform::CUDA);

	if(!getFactory())
		mActivePlatform = (int)nv::cloth::Platform::CPU;

	for(int i = 0; i < Scene::GetSceneCount(); i++)
	{
		pressed = pressed | ImGui::RadioButton(Scene::GetSceneName(i), &mActiveSceneIndex, i);
	}
	if(pressed)
		changeScene(mActiveSceneIndex);

	mActiveScene->drawUI();
}

void SceneController::drawStatsUI()
{
	mActiveScene->drawStatsUI();
}

