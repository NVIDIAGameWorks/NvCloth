/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "DistanceConstraintScene.h"
#include "Scene/SceneController.h"
#include <NvClothExt/ClothFabricCooker.h>
#include "ClothMeshGenerator.h"
#include <NvCloth/Fabric.h>
#include <NvCloth/Solver.h>
#include <NvCloth/Cloth.h>
#include <NvCloth/Factory.h>
#include "Renderer.h"
#include "renderer/RenderUtils.h"
 
DECLARE_SCENE_NAME(DistanceConstraintScene,"Distance Constraint Scene")

void DistanceConstraintScene::onInitialize()
{
	///////////////////////////////////////////////////////////////////////
	ClothMeshData clothMesh;

	physx::PxMat44 transform = PxTransform(PxVec3(0.f, 13.f, 0.f), physx::PxQuat(-0.9f, physx::PxVec3(0.0f, 1.0f, 0.0f)) * physx::PxQuat(PxPiDivFour, physx::PxVec3(1.0f, 0.0f, 0.0f)));
	clothMesh.GeneratePlaneCloth(6.f, 7.f, 39, 39, false, transform);
	clothMesh.AttachClothPlaneByAngles(39, 39);

	mClothActor = new ClothActor;
	nv::cloth::ClothMeshDesc meshDesc = clothMesh.GetClothMeshDesc();
	{
		mClothActor->mClothRenderMesh = new ClothRenderMesh(meshDesc);
		mClothActor->mClothRenderable = getSceneController()->getRenderer().createRenderable(*(static_cast<IRenderMesh*>(mClothActor->mClothRenderMesh)), *getSceneController()->getDefaultMaterial());
		mClothActor->mClothRenderable->setColor(getRandomPastelColor());
	}

	nv::cloth::Vector<int32_t>::Type phaseTypeInfo;
	mFabric = NvClothCookFabricFromMesh(getSceneController()->getFactory(), meshDesc, physx::PxVec3(0.0f, -1.0f, 0.0f), &phaseTypeInfo, false);
	trackFabric(mFabric);

	// Initialize start positions and masses for the actual cloth instance
	// (note: the particle/vertex positions do not have to match the mesh description here. Set the positions to the initial shape of this cloth instance)
	std::vector<physx::PxVec4> particlesCopy;
	particlesCopy.resize(clothMesh.mVertices.size());

	for(int i = 0; i < (int)clothMesh.mVertices.size(); i++)
	{
		particlesCopy[i] = physx::PxVec4(clothMesh.mVertices[i], clothMesh.mInvMasses[i]); // w component is 1/mass, or 0.0f for anchored/fixed particles
	}

	// Create the cloth from the initial positions/masses and the fabric
	mClothActor->mCloth = getSceneController()->getFactory()->createCloth(nv::cloth::Range<physx::PxVec4>(&particlesCopy[0], &particlesCopy[0] + particlesCopy.size()), *mFabric);


	////////////////////////////////////
	////  Set distance constraints  ////
	////////////////////////////////////
	nv::cloth::Range<physx::PxVec4> distanceConstraints = mClothActor->mCloth->getMotionConstraints();
	for(int i = 0; i < (int)distanceConstraints.size(); i++)
	{
		distanceConstraints[i] = physx::PxVec4(particlesCopy[i].getXYZ(), 0.002f*(i % 800) * 0.002f*(i % 800));
	}

	particlesCopy.clear(); particlesCopy.shrink_to_fit();

	mClothActor->mCloth->setGravity(physx::PxVec3(0.0f, -9.8f, 0.0f));

	// Setup phase configs
	std::vector<nv::cloth::PhaseConfig> phases(mFabric->getNumPhases());
	for(int i = 0; i < (int)phases.size(); i++)
	{
		phases[i].mPhaseIndex = i;
		phases[i].mStiffness = 1.0f;
		phases[i].mStiffnessMultiplier = 1.0f;
		phases[i].mCompressionLimit = 1.0f;
		phases[i].mStretchLimit = 1.0f;
	}
	mClothActor->mCloth->setPhaseConfig(nv::cloth::Range<nv::cloth::PhaseConfig>(&phases.front(), &phases.back()));
//	mClothActor->mCloth->setDragCoefficient(0.1f);
//	mClothActor->mCloth->setDragCoefficient(0.1f);

	mSolver = getSceneController()->getFactory()->createSolver();
	trackSolver(mSolver);
	trackClothActor(mClothActor);

	// Add the cloth to the solver for simulation
	addClothToSolver(mClothActor, mSolver);
	
	{
		IRenderMesh* mesh = getSceneController()->getRenderer().getPrimitiveRenderMesh(PrimitiveRenderMeshType::Plane);
		Renderable* plane = getSceneController()->getRenderer().createRenderable(*mesh, *getSceneController()->getDefaultPlaneMaterial());
		plane->setTransform(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxPiDivTwo, PxVec3(0.f, 0.f, 1.f))));
		plane->setScale(PxVec3(1000.f));
		trackRenderable(plane);
	}
}
