/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "Scene.h"
#include <NvCloth/Cloth.h>
#include <NvCloth/Solver.h>
#include <NvCloth/Fabric.h>
#include "Renderer.h"

std::vector<SceneFactory> Scene::sSceneFactories;

Scene::~Scene()
{

}

namespace
{
template <typename T> void trackT(std::vector<T>& list, T object)
{
	list.push_back(object);
}

template <typename T> void untrackT(std::vector<T>& list, T object)
{
	for(auto it = list.begin(); it != list.end(); ++it)
	{
		if(*it == object)
		{
			list.erase(it);
			break;
		}
	}
}
}
void Scene::trackClothActor(ClothActor* clothActor)
{
	trackT(mClothList, clothActor);
}
void Scene::untrackClothActor(ClothActor* clothActor)
{
	untrackT(mClothList, clothActor);
}
void Scene::trackSolver(nv::cloth::Solver* solver)
{
	trackT(mSolverList, solver);
	mSolverHelpers[solver].Initialize(solver, mSceneController->getJobManager());
}
void Scene::untrackSolver(nv::cloth::Solver* solver)
{
	untrackT(mSolverList, solver);
	mSolverHelpers.erase(solver);
}
void Scene::trackFabric(nv::cloth::Fabric* fabric)
{
	trackT(mFabricList, fabric);
}
void Scene::untrackFabric(nv::cloth::Fabric* fabric)
{
	untrackT(mFabricList, fabric);
}

void Scene::addClothToSolver(ClothActor* clothActor, nv::cloth::Solver* solver)
{
	solver->addCloth(clothActor->mCloth);
	assert(mClothSolverMap.find(clothActor) == mClothSolverMap.end());
	mClothSolverMap[clothActor] = solver;
}

void Scene::trackRenderable(Renderable* renderable)
{
	trackT(mRenderableList, renderable);
}
void Scene::untrackRenderable(Renderable* renderable)
{
	untrackT(mRenderableList, renderable);
}

void Scene::autoDeinitialize()
{
	//Remove all cloths from solvers
	for (auto it : mClothSolverMap)
	{
		it.second->removeCloth(it.first->mCloth);
	}
	mClothSolverMap.clear();

	//Destroy all solvers
	for (auto it : mSolverList)
	{
		delete it;
	}
	mSolverList.clear();
	mSolverHelpers.clear();

	//Destroy all cloths
	for (auto it : mClothList)
	{
		delete it->mCloth;
		mSceneController->getRenderer().removeRenderable(it->mClothRenderable);
		delete it->mClothRenderMesh;
		delete it;
	}
	mClothList.clear();

	//Destroy all fabrics
	for (auto it : mFabricList)
	{
		it->decRefCount();
	}
	mFabricList.clear();

	//Destroy all renderables
	for(auto it : mRenderableList)
	{
		mSceneController->getRenderer().removeRenderable(it);
	}
	mRenderableList.clear();
}

void Scene::doSimulationStep(float dt)
{
	startSimulationStep(dt);
	waitForSimulationStep();
	updateSimulationGraphics();
}

void Scene::startSimulationStep(float dt)
{
	for(auto it = mSolverHelpers.begin(); it != mSolverHelpers.end(); ++it)
	{
		it->second.StartSimulation(dt);
	}
}
void Scene::waitForSimulationStep()
{
	for(auto it = mSolverHelpers.begin(); it != mSolverHelpers.end(); ++it)
	{
		it->second.WaitForSimulation();
	}
}

void Scene::updateSimulationGraphics()
{
	for each (auto actor in mClothList)
	{
		nv::cloth::MappedRange<physx::PxVec4> particles = actor->mCloth->getCurrentParticles();
		std::vector<PxVec3> particles3(particles.size());
		for(uint32_t i = 0; i < particles.size(); ++i)
			particles3[i] = particles[i].getXYZ();

		actor->mClothRenderMesh->update(particles3.data(), particles.size());
	}
}

