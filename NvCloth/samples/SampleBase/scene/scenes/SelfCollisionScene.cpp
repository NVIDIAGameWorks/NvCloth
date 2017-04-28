/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "SelfCollisionScene.h"
#include "Scene/SceneController.h"
#include <NvClothExt/ClothFabricCooker.h>
#include "ClothMeshGenerator.h"
#include <NvCloth/Fabric.h>
#include <NvCloth/Solver.h>
#include <NvCloth/Cloth.h>
#include <NvCloth/Factory.h>
#include "Renderer.h"
#include "renderer/RenderUtils.h"

DECLARE_SCENE_NAME(SelfCollisionScene, "Self Collision Scene")

void SelfCollisionScene::initializeCloth(int index, physx::PxMat44 transform)
{
	///////////////////////////////////////////////////////////////////////
	
	float w = 5.f - index;
	float h = 6.f + index;
	transform *= PxTransform(PxVec3(0.f, 13.f, 0.f), PxQuat(0, PxVec3(1.f, 0.f, 0.f)));
	ClothMeshData clothMesh;
	clothMesh.GeneratePlaneCloth(w, h, 5 * w, 5 * h, false, transform);
	clothMesh.AttachClothPlaneByAngles(5 * w, 5 * h);
	clothMesh.SetInvMasses(1.0f / (1000.0f / (5.0f*w*5.0f*h)));

	float w2 = w - 1.0f;
	float h2 = h + 1.0f;
	transform *= PxTransform(PxVec3(0.f, 0.8f, -0.2f), PxQuat(0, PxVec3(1.f, 0.f, 0.f)));
	ClothMeshData clothMesh2;
	clothMesh2.GeneratePlaneCloth(w2, h2, 5 * w2, 5 * h2, false, transform);
	clothMesh2.AttachClothPlaneByAngles(5 * w2, 5 * h2);
	clothMesh2.SetInvMasses(1.0f / (1000.0f / (5.0f*w2*5.0f*h2)));
	int FirstParticleIndexCloth2 = (int)clothMesh.mVertices.size();
	clothMesh.Merge(clothMesh2);

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
	for(int i = 0; i < (int)clothMesh.mVertices.size(); i++)
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
	planes.push_back(physx::PxVec4(physx::PxVec3(0.0f, 1.f, 0.0f), -0.01f));

	nv::cloth::Range<const physx::PxVec4> planesR(&planes[0], &planes[0] + planes.size());
	mClothActor[index]->mCloth->setPlanes(planesR, 0, mClothActor[index]->mCloth->getNumPlanes());
	std::vector<uint32_t> indices;
	indices.resize(planes.size());
	for(int i = 0; i < (int)indices.size(); i++)
		indices[i] = 1 << i;
	nv::cloth::Range<uint32_t> cind(&indices[0], &indices[0] + indices.size());
	mClothActor[index]->mCloth->setConvexes(cind, 0, mClothActor[index]->mCloth->getNumConvexes());

	mClothActor[index]->mCloth->setGravity(physx::PxVec3(0.0f, -1.0f, 0.0f));
	mClothActor[index]->mCloth->setFriction(0.1);
	mClothActor[index]->mCloth->setDragCoefficient(0.1f);
	mClothActor[index]->mCloth->setLiftCoefficient(0.1f);
	mClothActor[index]->mCloth->setSolverFrequency(120.0f);
	mClothActor[index]->mCloth->setSelfCollisionDistance(0.26);
	mClothActor[index]->mCloth->setSelfCollisionStiffness(0.95);

	std::vector<uint32_t> selfCollisionIndices;

	//only enable every other particle for self collision
	for(int y = 0; y < 5*h + 1; y++)
		for(int x = 0; x < 5 * w + 1; x++)
			if((x & 1) ^ (y & 1)) selfCollisionIndices.push_back(x + y*(5 * w + 1));

	for(int y = 0; y < 5 * h2 + 1; y++)
		for(int x = 0; x < 5 * w2 + 1; x++)
			if((x & 1) ^ (y & 1)) selfCollisionIndices.push_back(FirstParticleIndexCloth2 + x + y*(5 * w2 + 1));

	nv::cloth::Range<uint32_t> selfCollisionIndicesRange (&selfCollisionIndices[0], &selfCollisionIndices[0] + selfCollisionIndices.size());
	mClothActor[index]->mCloth->setSelfCollisionIndices(selfCollisionIndicesRange);

	// Setup phase configs
	std::vector<nv::cloth::PhaseConfig> phases(mFabric[index]->getNumPhases());
	for(int i = 0; i < (int)phases.size(); i++)
	{
		phases[i].mPhaseIndex = i;
		phases[i].mStiffness = 1.0f;
		phases[i].mStiffnessMultiplier = 1.0f;
		phases[i].mCompressionLimit = 1.0f;
		phases[i].mStretchLimit = 1.0f;
	}
	mClothActor[index]->mCloth->setPhaseConfig(nv::cloth::Range<nv::cloth::PhaseConfig>(&phases.front(), &phases.back()));

	trackClothActor(mClothActor[index]);

	// Add the cloth to the solver for simulation
	addClothToSolver(mClothActor[index], mSolver);
}

void SelfCollisionScene::onInitialize()
{
	mSolver = getSceneController()->getFactory()->createSolver();
	/*mSolver->setInterCollisionNbIterations(8);
	mSolver->setInterCollisionDistance(0.4f);
	mSolver->setInterCollisionStiffness(0.95f);
	mSolver->setInterCollisionFilter(
		[](void* a, void* b) {
		return true;
	}
	);*/
	trackSolver(mSolver);

	physx::PxMat44 posTrans(physx::PxIdentity);
	posTrans.setPosition(physx::PxVec3(0.0f, 0.f, -1.0f));
	initializeCloth(0, posTrans);
	//posTrans.setPosition(physx::PxVec3(0.0f, 0.8f, -1.2f));
	//initializeCloth(1, posTrans);

	{
		IRenderMesh* mesh = getSceneController()->getRenderer().getPrimitiveRenderMesh(PrimitiveRenderMeshType::Plane);
		Renderable* plane = getSceneController()->getRenderer().createRenderable(*mesh, *getSceneController()->getDefaultPlaneMaterial());
		plane->setTransform(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxPiDivTwo, PxVec3(0.f, 0.f, 1.f))));
		plane->setScale(PxVec3(1000.f));
		trackRenderable(plane);
	}
}
