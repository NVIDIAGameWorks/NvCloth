/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <map>
#include "scene/SceneController.h"
#include "foundation/PxMat44.h"
namespace nv
{
	namespace cloth
	{
		class Factory;
		class Solver;
		class Cloth;
		class Fabric;
	}
}

class Scene;
struct SceneFactory
{
	SceneFactory(Scene* (*create)(SceneController*), const char* name):Create(create), mName(name) {}
	Scene* (*Create)(SceneController*);
	const char* mName;
};

#define DECLARE_SCENE_NAME(classname, name) static int classname##id = Scene::AddSceneFactory([](SceneController* c) {return static_cast<Scene*>(new classname(c)); }, name);
#define DECLARE_SCENE(classname) DECLARE_SCENE_NAME(classname,#classname)

class Scene
{
public:

	Scene(SceneController* sceneController):mSceneController(sceneController) {}
	virtual ~Scene();

	virtual void Animate(double dt) { doSimulationStep(dt); drawDebugVisualization(); }
	void UpdateParticleDragging(float dt);
	virtual bool HandleEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool HandlePickingEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, physx::PxMat44 viewProjectionMatrix);
	virtual void drawUI();
	virtual void drawStatsUI() {}
	virtual void drawDebugVisualization();

	virtual void onInitialize() = 0;
	virtual void onTerminate() { autoDeinitialize(); }

	SceneController* getSceneController() { return mSceneController; }

	static int AddSceneFactory(Scene* (*create)(SceneController*), const char* name) { sSceneFactories.push_back(SceneFactory(create, name)); return (int)sSceneFactories.size(); }
	static Scene* CreateScene(int index, SceneController* controller) { return sSceneFactories[index].Create(controller); }
	static const char* GetSceneName(int index) { return sSceneFactories[index].mName; }
	static int GetSceneCount() { return (int)sSceneFactories.size(); }

protected:
	//Helper functions to enable automatic deinitialize
	//Tracking an object will delete it when autoDeinitialize is called
	//Untracking can be used if you delete it sooner than autoDeinitialize
	void trackClothActor(ClothActor* clothActor);
	void untrackClothActor(ClothActor* clothActor);

	void trackSolver(nv::cloth::Solver* solver);
	void untrackSolver(nv::cloth::Solver* solver);

	void trackFabric(nv::cloth::Fabric* fabric);
	void untrackFabric(nv::cloth::Fabric* fabric);

	void addClothToSolver(ClothActor* clothActor, nv::cloth::Solver* solver); //Helps to detach cloths from solver at AutoDeinit.

	void trackRenderable(Renderable* renderMesh);
	void untrackRenderable(Renderable* renderMesh);

	void autoDeinitialize();


	void doSimulationStep(float dt);
	void startSimulationStep(float dt);
	void waitForSimulationStep();
	void updateSimulationGraphics();

private:
	Scene& operator= (Scene&); // not implemented

	physx::PxMat44 GetDebugDrawTransform(const ClothActor& actor);
	void DebugRenderDistanceConstraints();
	void DebugRenderTethers();
	void DebugRenderConstraints();
	void DebugRenderConstraintStiffness();
	void DebugRenderConstraintError();
	void DebugRenderPositionDelta();
	void DebugRenderBoundingBox();

private:
	SceneController* mSceneController;

	std::vector<ClothActor*> mClothList;
	std::vector<nv::cloth::Solver*> mSolverList;
	std::map<nv::cloth::Solver*, MultithreadedSolverHelper> mSolverHelpers;
	std::vector<nv::cloth::Fabric*> mFabricList;
	std::map<ClothActor*, nv::cloth::Solver*> mClothSolverMap;
	std::vector<Renderable*> mRenderableList;

	static std::vector<SceneFactory> sSceneFactories;

	enum
	{
		DEBUG_VIS_DISTANCE_CONSTRAINTS = 1,
		DEBUG_VIS_TETHERS = 2,
		DEBUG_VIS_CONSTRAINTS = 4,
		DEBUG_VIS_CONSTRAINTS_STIFFNESS = 8,
		DEBUG_VIS_NORMALS = 16,
		DEBUG_VIS_TANGENTS = 32,
		DEBUG_VIS_BITANGENTS = 64,
		DEBUG_VIS_CONSTRAINT_ERROR = 128,
		DEBUG_VIS_POSITION_DELTA = 256,
		DEBUG_VIS_ACCELERATION = 512,
		DEBUG_VIS_BOUNDING_BOX = 1024,
		DEBUG_VIS_LAST
	};

	static unsigned int mDebugVisualizationFlags;

	struct SceneDebugRenderParams
	{
		//Constraint render params
		int mVisiblePhaseRangeBegin;
		int mVisiblePhaseRangeEnd;
	};

	static SceneDebugRenderParams sSceneDebugRenderParams;

	//Particle dragging
	struct DraggingParticle
	{
		DraggingParticle() { mTrackedCloth = nullptr; }
		ClothActor* mTrackedCloth;
		float mDist;
		float mOffset;
		int mParticleIndex;
	};
	DraggingParticle mDraggingParticle;
};


#endif