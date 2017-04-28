/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "FrictionScene.h"
#include "Scene/SceneController.h"
#include <NvClothExt/ClothFabricCooker.h>
#include "ClothMeshGenerator.h"
#include <NvCloth/Fabric.h>
#include <NvCloth/Solver.h>
#include <NvCloth/Cloth.h>
#include <NvCloth/Factory.h>
#include "Renderer.h"
#include "renderer/RenderUtils.h"

DECLARE_SCENE_NAME(FrictionScene, "Friction Scene")

void FrictionScene::initializeCloth(int index, physx::PxVec3 offset, float frictionCoef)
{
	///////////////////////////////////////////////////////////////////////
	ClothMeshData clothMesh;

	physx::PxMat44 transform = PxTransform(PxVec3(0.f, 9.f, 0.f) + offset, PxQuat(PxPi / 6.f, PxVec3(1.f, 0.f, 0.f)));
	clothMesh.GeneratePlaneCloth(4.f, 5.f, 29, 34, false, transform);

	mClothActor[index] = new ClothActor;
	nv::cloth::ClothMeshDesc meshDesc = clothMesh.GetClothMeshDesc();
	{
		mClothActor[index]->mClothRenderMesh = new ClothRenderMesh(meshDesc);
		mClothActor[index]->mClothRenderable = getSceneController()->getRenderer().createRenderable(*(static_cast<IRenderMesh*>(mClothActor[index]->mClothRenderMesh)), *getSceneController()->getDefaultMaterial());
		mClothActor[index]->mClothRenderable->setColor(getRandomPastelColor());
	}

	nv::cloth::Vector<int32_t>::Type phaseTypeInfo;
	mFabric[index] = NvClothCookFabricFromMesh(getSceneController()->getFactory(), meshDesc, physx::PxVec3(0.0f, 0.0f, 1.0f), &phaseTypeInfo, false);
	trackFabric(mFabric[index]);

	// Initialize start positions and masses for the actual cloth instance
	// (note: the particle/vertex positions do not have to match the mesh description here. Set the positions to the initial shape of this cloth instance)
	std::vector<physx::PxVec4> particlesCopy;
	particlesCopy.resize(clothMesh.mVertices.size());

	physx::PxVec3 clothOffset = transform.getPosition();
	for (int i = 0; i < (int)clothMesh.mVertices.size(); i++)
	{
		// To put attachment point closer to each other
		if(clothMesh.mInvMasses[i] < 1e-6)
			clothMesh.mVertices[i] = (clothMesh.mVertices[i] - clothOffset)*0.9f + clothOffset;

		particlesCopy[i] = physx::PxVec4(clothMesh.mVertices[i], clothMesh.mInvMasses[i]); // w component is 1/mass, or 0.0f for anchored/fixed particles
	}

	// Create the cloth from the initial positions/masses and the fabric
	mClothActor[index]->mCloth = getSceneController()->getFactory()->createCloth(nv::cloth::Range<physx::PxVec4>(&particlesCopy[0], &particlesCopy[0] + particlesCopy.size()), *mFabric[index]);
	particlesCopy.clear(); particlesCopy.shrink_to_fit();

	std::vector<physx::PxVec4> planes;
	planes.push_back(physx::PxVec4(PxQuat(PxPiDivFour*0.5f, PxVec3(1.f, 0.f, 0.f)).rotate(physx::PxVec3(0.0f, 1.f, 0.0f)), -0.01f));

	nv::cloth::Range<const physx::PxVec4> planesR(&planes[0], &planes[0] + planes.size());
	mClothActor[index]->mCloth->setPlanes(planesR, 0, mClothActor[index]->mCloth->getNumPlanes());
	std::vector<uint32_t> indices;
	indices.resize(planes.size());
	for (int i = 0; i < (int)indices.size(); i++)
		indices[i] = 1 << i;
	nv::cloth::Range<uint32_t> cind(&indices[0], &indices[0] + indices.size());
	mClothActor[index]->mCloth->setConvexes(cind, 0, mClothActor[index]->mCloth->getNumConvexes());

	mClothActor[index]->mCloth->setGravity(physx::PxVec3(0.0f, -9.8f, 0.0f));

	// Setup phase configs
	std::vector<nv::cloth::PhaseConfig> phases(mFabric[index]->getNumPhases());
	for (int i = 0; i < (int)phases.size(); i++)
	{
		phases[i].mPhaseIndex = i;
		phases[i].mStiffness = 1.0f;
		phases[i].mStiffnessMultiplier = 1.0f;
		phases[i].mCompressionLimit = 1.0f;
		phases[i].mStretchLimit = 1.0f;
	}
	mClothActor[index]->mCloth->setPhaseConfig(nv::cloth::Range<nv::cloth::PhaseConfig>(&phases.front(), &phases.back()));
	mClothActor[index]->mCloth->setFriction(frictionCoef);

	trackClothActor(mClothActor[index]);

	// Add the cloth to the solver for simulation
	addClothToSolver(mClothActor[index], mSolver);
}

void FrictionScene::onInitialize()
{
	mSolver = getSceneController()->getFactory()->createSolver();
	trackSolver(mSolver);
	
	float spaceX = -5.f;
	float frictionDelta = 0.2f;

	for (int i = 0; i < 5; ++i)
	{
		float friction = i > 0 ? float(i) * frictionDelta : 0.f; // 0.0, 0.2, 0.4, 0.6, 0.8
		initializeCloth(i, physx::PxVec3(4.f + float(i) * spaceX, 4.f, -18.f), friction);
	}

	{
		IRenderMesh* mesh = getSceneController()->getRenderer().getPrimitiveRenderMesh(PrimitiveRenderMeshType::Plane);
		Renderable* plane = getSceneController()->getRenderer().createRenderable(*mesh, *getSceneController()->getDefaultPlaneMaterial());
		plane->setTransform(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxPiDivFour*0.5f, PxVec3(1.f, 0.f, 0.f))*PxQuat(PxPiDivTwo, PxVec3(0.f, 0.f, 1.f))));
		plane->setScale(PxVec3(1000.f));
		trackRenderable(plane);
	}
}
