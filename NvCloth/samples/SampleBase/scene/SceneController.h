/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef SCENE_CONTROLLER_H
#define SCENE_CONTROLLER_H


#include "SampleManager.h"
#include <map>
#include <vector>

#include <NvCloth/Factory.h>
#include "JobManager.h"

#include "ClothRenderMesh.h"
#include <cuda.h>
#include "CallbackImplementations.h"

namespace nv
{
	namespace cloth
	{
		class Factory;
		class Solver;
		class Cloth;
		class Fabric;
		class DxContextManagerCallback;
	}
}

class RenderMaterial;
class Renderable;
class DebugLineRenderBuffer;

struct ClothActor 
{
	Renderable*					mClothRenderable;
	ClothRenderMesh*			mClothRenderMesh;
	nv::cloth::Cloth*			mCloth;
};

class Scene;

class SceneController : public ISampleController
{
public:

	SceneController();
	virtual ~SceneController();

	virtual LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void Animate(double dt);
	void drawUI();
	void drawStatsUI();

	virtual void onInitialize();
	virtual void onSampleStart();
	virtual void onSampleStop();
	virtual void onTerminate();

	JobManager* getJobManager() { return &sJobManager; }
	nv::cloth::Factory* getFactory() { return mFactories[mActivePlatform]; }

	static JobManager sJobManager;

	//////// used controllers ////////

	Renderer& getRenderer() const
	{
		return getManager()->getRenderer();
	}

	CommonUIController& getCommonUIController() const
	{
		return getManager()->getCommonUIController();
	}

	RenderMaterial* getDefaultMaterial()
	{
		return mPhysXPrimitiveRenderMaterial;
	}

	RenderMaterial* getDefaultPlaneMaterial()
	{
		return mPhysXPlaneRenderMaterial;
	}

	DebugLineRenderBuffer* getDebugLineRenderBuffer()
	{
		return mDebugLineRenderBuffer;
	}

private:
	SceneController& operator= (SceneController&); // not implemented

	void changeScene(int index);

	//////// internal data ////////

	RenderMaterial*				mPhysXPrimitiveRenderMaterial;
	RenderMaterial*				mPhysXPlaneRenderMaterial;

	Renderable*					mPlane;
	std::vector<Renderable*>	mBoxes;
	DebugLineRenderBuffer*		mDebugLineRenderBuffer;

	float						mTimeScale;
	float						mStartDelay;

	// NvCloth
	CpuDispatcher				mDispatcher;
	physx::PxTaskManager*		mTaskManager;

	nv::cloth::Factory*			mFactories[3];
	int							mActivePlatform;
	bool						mCUDAInitialized;
	CUcontext					mCUDAContext;
	bool						mDXInitialized;
	ID3D11Device*				mDXDevice;
	ID3D11DeviceContext*		mDXDeviceContext;
	nv::cloth::DxContextManagerCallback* mGraphicsContextManager;

	Scene*						mActiveScene;
	int							mActiveSceneIndex;
	double						mLeftOverTime;

	bool						mPaused;
	int						mSingleStep;

};

#endif