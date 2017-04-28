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
#include "imgui.h"
#include <sstream>
#include "utils/DebugLineRenderBuffer.h"
#include <foundation/PxVec2.h>

std::vector<SceneFactory> Scene::sSceneFactories;
unsigned int Scene::mDebugVisualizationFlags;
Scene::SceneDebugRenderParams Scene::sSceneDebugRenderParams;

Scene::~Scene()
{

}

void Scene::UpdateParticleDragging(float dt)
{
	physx::PxVec2 screenResolution(
		mSceneController->getRenderer().getScreenWidth(),
		mSceneController->getRenderer().getScreenHeight());

	physx::PxMat33 invScreenTransform;
	{
		float m[]{2.0f / screenResolution.x, 0.0f, 0.0f, 0.0f, -2.0f / screenResolution.y, 0.0f, -1.0f, 1.0f, 1.0f};
		invScreenTransform = physx::PxMat33(m);
	}

	physx::PxVec2 mousePoint;
	{
		POINT wpt;
		GetCursorPos(&wpt);
		ScreenToClient(GetDeviceManager()->GetHWND(), &wpt);
		int x, y;
		x = wpt.x;
		y = wpt.y;
		physx::PxVec3 pt(invScreenTransform*physx::PxVec3((float)x, (float)y, 1.0f));
		mousePoint.x = pt.x;
		mousePoint.y = pt.y;
	}

	physx::PxMat44 viewProjectionMatrix;
	physx::PxVec3 viewDirection;
	physx::PxVec3 viewPoint;
	{
		auto camera = mSceneController->getRenderer().getCamera();
		auto m1 = camera.GetViewMatrix();
		auto m2 = camera.GetProjMatrix();
		viewProjectionMatrix = physx::PxMat44((float*)&m2.r)*physx::PxMat44((float*)&m1.r);
		auto v1 = camera.GetWorldAhead();
		viewDirection = physx::PxVec3(*(physx::PxVec3*)&v1);
		auto v2 = camera.GetEyePt();
		viewPoint = physx::PxVec3(*(physx::PxVec3*)&v2);
	}

	if(mDraggingParticle.mTrackedCloth)
	{
		nv::cloth::Cloth* cloth = mDraggingParticle.mTrackedCloth->mCloth;
		physx::PxMat44 modelMatrix = GetDebugDrawTransform(*mDraggingParticle.mTrackedCloth);
		nv::cloth::Range<physx::PxVec4> particles = cloth->getCurrentParticles();
		nv::cloth::Range<physx::PxVec4> prevParticles = cloth->getPreviousParticles();

		physx::PxMat44 tmp = viewProjectionMatrix;
		auto tmp2 = DirectX::XMMatrixInverse(nullptr, DirectX::XMMATRIX(tmp.front()));
		tmp = physx::PxMat44((float*)&tmp2.r);
		physx::PxMat44 invMatrix = tmp;

		physx::PxVec3 particleWorld = modelMatrix.transform(particles[mDraggingParticle.mParticleIndex].getXYZ());
		physx::PxVec4 mousePointWorldT = invMatrix.transform(physx::PxVec4(mousePoint.x, mousePoint.y, 1.0, 1.0));
		physx::PxVec3 mousePointWorld = mousePointWorldT.getXYZ() / mousePointWorldT.w;
		physx::PxVec3 mouseRayStart = viewPoint;
		physx::PxVec3 mouseRayDir = (mousePointWorld - mouseRayStart).getNormalized();
		//float rayT = -viewDirection.dot(mouseRayStart - particleWorld) / viewDirection.dot(mouseRayDir); //intersect plane
		float rayT = mDraggingParticle.mDist;
		physx::PxVec3 mousePointPlane = mouseRayStart + mouseRayDir*rayT;

		physx::PxVec3 offset = mousePointPlane - particleWorld;
		if(offset.magnitudeSquared() > 2.5f*2.5f)
			offset = offset.getNormalized()*2.5f;

		for(int i = 0; i < (int)particles.size(); i++)
		{
			physx::PxVec4 p = particles[i];
			float dist = (p.getXYZ() - particleWorld).magnitude();

			if(p.w > 0.0f) //Only move dynamic points
			{
				float weight = max(0.0,min(1.0,0.4-dist));
				physx::PxVec3 point0(prevParticles[i].x, prevParticles[i].y, prevParticles[i].z);
				point0 = point0 - weight*offset;
				point0 = point0*0.99f + p.getXYZ()*0.01f;
				//move previous particle in the opposite direction to avoid invalid configurations in the next solver iteration.
				prevParticles[i] = physx::PxVec4(point0.x, point0.y, point0.z, prevParticles[i].w);
			}
		}
	}
}


bool Scene::HandleEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto camera = mSceneController->getRenderer().getCamera();
	auto m1 = camera.GetViewMatrix();
	auto m2 = camera.GetProjMatrix();
	
	return HandlePickingEvent(uMsg, wParam, lParam, physx::PxMat44((float*)&m2.r)*physx::PxMat44((float*)&m1.r));
}

bool Scene::HandlePickingEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, physx::PxMat44 viewProjectionMatrix2)
{
	if(uMsg != WM_LBUTTONDOWN && uMsg != WM_LBUTTONUP)
		return false;

	physx::PxVec2 screenResolution(
		mSceneController->getRenderer().getScreenWidth(),
		mSceneController->getRenderer().getScreenHeight());

	physx::PxMat33 invScreenTransform;
	{
		float m[]{2.0f / screenResolution.x, 0.0f, 0.0f, 0.0f, -2.0f / screenResolution.y, 0.0f, -1.0f, 1.0f, 1.0f};
		invScreenTransform = physx::PxMat33(m);
	}

	physx::PxVec2 mousePoint;
	{
		int x, y;
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		physx::PxVec3 pt(invScreenTransform*physx::PxVec3((float)x, (float)y, 1.0f));
		mousePoint.x = pt.x;
		mousePoint.y = pt.y;
	}

	physx::PxMat44 viewProjectionMatrix;
	physx::PxVec3 viewDirection;
	physx::PxVec3 viewPoint;
	{
		auto camera = mSceneController->getRenderer().getCamera();
		auto m1 = camera.GetViewMatrix();
		auto m2 = camera.GetProjMatrix();
		viewProjectionMatrix = physx::PxMat44((float*)&m2.r)*physx::PxMat44((float*)&m1.r);
		auto v1 = camera.GetWorldAhead();
		viewDirection = physx::PxVec3(*(physx::PxVec3*)&v1);
		auto v2 = camera.GetEyePt();
		viewPoint = physx::PxVec3(*(physx::PxVec3*)&v2);
	}

	if(uMsg == WM_LBUTTONDOWN)
	{
		mDraggingParticle.mDist = 9999999.0f;
		mDraggingParticle.mOffset = 9999999.0f;
		mDraggingParticle.mTrackedCloth = nullptr;
		for(auto it : mClothList)
		{
			nv::cloth::Cloth* cloth = it->mCloth;
			physx::PxMat44 modelMatrix = GetDebugDrawTransform(*it);
			nv::cloth::Range<physx::PxVec4> particles = cloth->getCurrentParticles();

			physx::PxMat44 tmp = viewProjectionMatrix;
			auto tmp2 = DirectX::XMMatrixInverse(nullptr, DirectX::XMMATRIX(tmp.front()));
			tmp = physx::PxMat44((float*)&tmp2.r);
			physx::PxMat44 invMatrix = tmp;

			physx::PxVec4 mousePointWorldT = invMatrix.transform(physx::PxVec4(mousePoint.x, mousePoint.y, 1.0, 1.0));
			physx::PxVec3 mousePointWorld = mousePointWorldT.getXYZ() / mousePointWorldT.w;
			physx::PxVec3 mouseRayStart = viewPoint;
			physx::PxVec3 mouseRayDir = (mousePointWorld - mouseRayStart).getNormalized();

			for(int i = 0; i<(int)particles.size(); i++)
			{
				physx::PxVec4 p = particles[i];
				physx::PxVec4 point(p.x, p.y, p.z, 1.0f);

				float dist = mouseRayDir.dot(point.getXYZ() - mouseRayStart);
				float offset = (point.getXYZ() - (dist*mouseRayDir + mouseRayStart)).magnitude();

				if(offset<0.1f)
				{
					if(mDraggingParticle.mDist + 0.5f*mDraggingParticle.mOffset>dist + 0.5f*offset)
					{
						mDraggingParticle.mTrackedCloth = it;
						//mDraggingParticle.mOffset = offset;
						mDraggingParticle.mDist = dist;
						mDraggingParticle.mParticleIndex = i;
					}
				}
			}
		}
		return true;
	}
	if(uMsg == WM_LBUTTONUP)
	{
		mDraggingParticle.mTrackedCloth = nullptr;
		return true;
	}
	return false;
}

void Scene::drawUI()
{
	static int activeCloth = 0;

	if(ImGui::TreeNode("Cloth Properties"))
	{
		activeCloth = min(activeCloth, (int)mClothList.size() - 1);
		for(int i = 0; i < (int)mClothList.size(); i++)
		{
			if(i)
				ImGui::SameLine();
			nv::cloth::Cloth* cloth = mClothList[i]->mCloth;
			std::stringstream clothName;
			clothName << "Cloth " << i;
			ImGui::RadioButton(clothName.str().c_str(), &activeCloth, i);
		}

		nv::cloth::Cloth* cloth = mClothList[activeCloth]->mCloth;
		
		
		{
			bool b = cloth->isContinuousCollisionEnabled();
			if(ImGui::Checkbox("Continuous Collision Detection (CCD)", &b))
				cloth->enableContinuousCollision(b);
		}
		{
			physx::PxVec3 f3 = cloth->getDamping();
			if(ImGui::DragFloat3("Damping", &f3.x, 0.02f, 0.0f, 1.0f, "%.2f"))
				cloth->setDamping(f3);
			float f = f3.maxElement();
			if(ImGui::DragFloat("Damping xyz", &f, 0.02f, 0.0f, 1.0f, "%.2f"))
				cloth->setDamping(physx::PxVec3(f,f,f));
		}
		{
			float f = cloth->getDragCoefficient();
			if(ImGui::DragFloat("Drag Coefficient", &f, 0.02f, 0.0f, 0.99f, "%.2f"))
				cloth->setDragCoefficient(f);
		}
		{
			float f = cloth->getFriction();
			if(ImGui::DragFloat("Friction", &f, 0.04f, 0.0f, 2.0f, "%.2f"))
				cloth->setFriction(f);
		}
		{
			physx::PxVec3 f3 = cloth->getGravity();
			if(ImGui::DragFloat3("Gravity", &f3.x, 0.5f, -50.0f, 50.0f, "%.1f"))
				cloth->setGravity(f3);
		}
		{
			float f = cloth->getLiftCoefficient();
			if(ImGui::DragFloat("Lift Coefficient", &f, 0.02f, 0.0f, 1.0f, "%.2f"))
				cloth->setLiftCoefficient(f);
		}
		{
			physx::PxVec3 f3 = cloth->getLinearInertia();
			if(ImGui::DragFloat3("Linear Inertia", &f3.x, 0.02f, 0.0f, 1.0f, "%.2f"))
				cloth->setLinearInertia(f3);
			float f = f3.maxElement();
			if(ImGui::DragFloat("Linear Inertia xyz", &f, 0.02f, 0.0f, 1.0f, "%.2f"))
				cloth->setLinearInertia(physx::PxVec3(f, f, f));
		}
		{
			float f = cloth->getMotionConstraintScale();
			if(ImGui::DragFloat("Motion Constraint Scale", &f, 0.08f, 0.0f, 4.0f, "%.2f"))
				cloth->setMotionConstraintScaleBias(f, cloth->getMotionConstraintBias());
		}
		{
			float f = cloth->getMotionConstraintBias();
			if(ImGui::DragFloat("Motion Constraint Bias", &f, 0.16f, 0.0f, 8.0f, "%.2f"))
				cloth->setMotionConstraintScaleBias(cloth->getMotionConstraintScale(),f);
		}
		{
			float f = cloth->getSelfCollisionDistance();
			if(ImGui::DragFloat("Self Collision Distance", &f, 0.005f, 0.0f, 0.3f, "%.3f"))
				cloth->setSelfCollisionDistance(f);
		}
		{
			float f = cloth->getSelfCollisionStiffness();
			if(ImGui::DragFloat("Self Collision Stiffness", &f, 0.02f, 0.0f, 1.0f, "%.2f"))
				cloth->setSelfCollisionStiffness(f);
		}
		{
			float f = cloth->getSleepThreshold();
			if(ImGui::DragFloat("Sleep Threshold", &f, 0.02f, 0.0f, 1.0f, "%.2f"))
				cloth->setSleepThreshold(f);
		}
		{
			float f = cloth->getStiffnessFrequency();
			if(ImGui::DragFloat("Stiffness Frequency", &f, 1.0f, 0.0f, 600.0f, "%.0f",1.5f))
				cloth->setStiffnessFrequency(f);
		}
		{
			float f = cloth->getSolverFrequency();
			if(ImGui::DragFloat("Solver Frequency", &f, 1.0f, 0.0f, 600.0f, "%.0f", 1.5f))
				cloth->setSolverFrequency(f);
		}
		{
			float f = cloth->getTetherConstraintScale();
			if(ImGui::DragFloat("Tether Constraint Scale", &f, 0.08f, 0.0f, 4.0f, "%.2f"))
				cloth->setTetherConstraintScale(f);
		}
		{
			float f = cloth->getTetherConstraintStiffness();
			if(ImGui::DragFloat("Tether Constraint Stiffness", &f, 0.02f, 0.0f, 1.0f, "%.2f"))
				cloth->setTetherConstraintStiffness(f);
		}
		{
			physx::PxVec3 f3 = cloth->getWindVelocity();
			if(ImGui::DragFloat3("Wind Velocity", &f3.x, 0.5f, -50.0f, 50.0f, "%.1f"))
				cloth->setWindVelocity(f3);
		}
		ImGui::TreePop();
	}

	if(ImGui::TreeNode("Debug Visualization"))
	{
		ImGui::CheckboxFlags("Tethers", &mDebugVisualizationFlags, DEBUG_VIS_TETHERS);
		ImGui::CheckboxFlags("Constraints", &mDebugVisualizationFlags, DEBUG_VIS_CONSTRAINTS);
		if(mDebugVisualizationFlags&DEBUG_VIS_CONSTRAINTS)
		{
			ImGui::DragInt("Start Constraint Phase Range", &sSceneDebugRenderParams.mVisiblePhaseRangeBegin, 0.05, 0, 30);
			ImGui::DragInt("End", &sSceneDebugRenderParams.mVisiblePhaseRangeEnd, 0.05, 0, 30);
		}
		ImGui::CheckboxFlags("Constraint Stiffness", &mDebugVisualizationFlags, DEBUG_VIS_CONSTRAINTS_STIFFNESS);

		ImGui::CheckboxFlags("Constraint Error", &mDebugVisualizationFlags, DEBUG_VIS_CONSTRAINT_ERROR);
		ImGui::CheckboxFlags("Position Delta", &mDebugVisualizationFlags, DEBUG_VIS_POSITION_DELTA);
		ImGui::CheckboxFlags("Bounding Box", &mDebugVisualizationFlags, DEBUG_VIS_BOUNDING_BOX);
		ImGui::CheckboxFlags("Distance Constraints", &mDebugVisualizationFlags, DEBUG_VIS_DISTANCE_CONSTRAINTS);

		ImGui::TreePop();
	}

	static int activeSolver = 0;

	if(ImGui::TreeNode("Solver Properties"))
	{
		activeSolver = min(activeSolver, (int)mSolverList.size() - 1);
		for(int i = 0; i < (int)mSolverList.size(); i++)
		{
			if(i)
				ImGui::SameLine();
			nv::cloth::Solver* solver = mSolverList[i];
			std::stringstream clothName;
			clothName << "Solver " << i;
			ImGui::RadioButton(clothName.str().c_str(), &activeSolver, i);
		}

		nv::cloth::Solver* solver = mSolverList[activeSolver];

		{
			float f = solver->getInterCollisionDistance();
			if(ImGui::DragFloat("Inter Collision Distance", &f, 0.005f, 0.0f, 2.0f, "%.2f"))
				solver->setInterCollisionDistance(f);
		}
		{
			uint32_t i = solver->getInterCollisionNbIterations();
			if(ImGui::DragInt("Inter Collision Iterations", (int*)&i, 0.25, 0, 16))
				solver->setInterCollisionNbIterations(i);
		}
		{
			float f = solver->getInterCollisionStiffness();
			if(ImGui::DragFloat("Inter Collision Stiffness", &f, 0.005f, 0.0f, 1.0f, "%.2f"))
				solver->setInterCollisionStiffness(f);
		}
		ImGui::TreePop();
	}
}

void Scene::drawDebugVisualization()
{
	if(mDebugVisualizationFlags & DEBUG_VIS_TETHERS)
		DebugRenderTethers();
	if(mDebugVisualizationFlags & DEBUG_VIS_CONSTRAINTS)
		DebugRenderConstraints();
	if(mDebugVisualizationFlags & DEBUG_VIS_CONSTRAINTS_STIFFNESS)
		DebugRenderConstraintStiffness();

	if(mDebugVisualizationFlags & DEBUG_VIS_CONSTRAINT_ERROR)
		DebugRenderConstraintError();
	if(mDebugVisualizationFlags & DEBUG_VIS_POSITION_DELTA)
		DebugRenderPositionDelta();
	if(mDebugVisualizationFlags & DEBUG_VIS_BOUNDING_BOX)
		DebugRenderBoundingBox();
	if(mDebugVisualizationFlags & DEBUG_VIS_DISTANCE_CONSTRAINTS)
		DebugRenderDistanceConstraints();
	
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
	if(dt < FLT_EPSILON)
		return;
	startSimulationStep(dt);
	waitForSimulationStep();
	updateSimulationGraphics();

	UpdateParticleDragging(dt);
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

physx::PxMat44 Scene::GetDebugDrawTransform(const ClothActor& actor)
{
	physx::PxMat44 transform;
	if(actor.mClothRenderable != nullptr)
	{
		transform = actor.mClothRenderable->getModelMatrix();
	}
	else
	{
		physx::PxVec3 translation = actor.mCloth->getTranslation();
		physx::PxQuat rotation = actor.mCloth->getRotation();
		transform = physx::PxMat44(rotation);
		transform[3] = physx::PxVec4(translation.x, translation.y, translation.z, 1.0f);
	}
	return transform;
}

void Scene::DebugRenderDistanceConstraints()
{
	DebugLineRenderBuffer* dbl = mSceneController->getDebugLineRenderBuffer();

	for(auto it : mClothList)
	{
		nv::cloth::Cloth& cloth = *it->mCloth;
		if(cloth.getNumMotionConstraints() == 0)
			continue;

		nv::cloth::Factory& factory = cloth.getFactory();

		physx::PxMat44 transform = GetDebugDrawTransform(*it);

		nv::cloth::MappedRange<physx::PxVec4> particles = cloth.getCurrentParticles();
		std::vector<physx::PxVec4> motionConstraints;
		motionConstraints.reserve(cloth.getNumMotionConstraints() * 2);
		motionConstraints.resize(cloth.getNumMotionConstraints());
		factory.extractMotionConstraints(cloth, nv::cloth::Range<physx::PxVec4>(&motionConstraints[0], &motionConstraints[0] + motionConstraints.size()));
		motionConstraints.resize(cloth.getNumMotionConstraints() * 2);

		nv::cloth::MappedRange<physx::PxVec4> positions = cloth.getCurrentParticles();

		assert(positions.size() == cloth.getNumMotionConstraints());


		//Set to 1 to color code lines based on distance constraint length (as % of max constraint distance in cloth)
		//Set to 0 to color code lines based on how close the particle is to the distance constraint (as % of max distance per constraint)
#define SHOW_DISTANCE_COLOR 0
#if SHOW_DISTANCE_COLOR
		float maxDist = 0.0f;
		for(int i = (int)(motionConstraints.size() >> 1) - 1; i >= 0; i--)
		{
			maxDist = max(maxDist, motionConstraints[i].w);
		}
#endif

		for(int i = ((int)motionConstraints.size() >> 1) - 1; i >= 0; i--)
		{
			float l = motionConstraints[i].w;
			physx::PxVec3 a = motionConstraints[i].getXYZ();
			physx::PxVec3 b = positions[i].getXYZ();
			physx::PxVec3 d = b - a;
			float currentDist = d.magnitude();
			if(d.magnitudeSquared() < 0.00001f)
			{
				d = physx::PxVec3(0.0f, 0.0f, 1.0f);
			}
			else
			{
				d.normalize();
			}
			unsigned char clerp;
#if SHOW_DISTANCE_COLOR
			clerp = (unsigned char)(max(0.0f,min(1.0f,(l / maxDist)))*255.0f);
#else
			clerp = (unsigned char)(max(0.0f, min(1.0f, (currentDist / l)))*255.0f);
#endif
			unsigned int c = ((255-clerp) << 8) + clerp;
			dbl->addLine(transform, a, a+d*l, c);
		}
	}
}

void Scene::DebugRenderTethers()
{
	DebugLineRenderBuffer* dbl = mSceneController->getDebugLineRenderBuffer();

	for(auto it : mClothList)
	{
		nv::cloth::Cloth& cloth = *it->mCloth;
		nv::cloth::Fabric& fabric = cloth.getFabric();
		if(fabric.getNumTethers() == 0)
			continue;

		nv::cloth::Factory& factory = cloth.getFactory();

		physx::PxMat44 transform = GetDebugDrawTransform(*it);

		nv::cloth::MappedRange<physx::PxVec4> particles = cloth.getCurrentParticles();

		std::vector<float> tetherLengths;
		tetherLengths.resize(fabric.getNumTethers());
		std::vector<uint32_t> anchors;
		anchors.resize(fabric.getNumTethers());

		factory.extractFabricData(fabric, nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(0, 0),
			nv::cloth::Range<uint32_t>(&anchors[0], &anchors[0] + anchors.size()), nv::cloth::Range<float>(&tetherLengths[0], &tetherLengths[0] + tetherLengths.size()), nv::cloth::Range<uint32_t>(0, 0));

		int particleCount = fabric.getNumParticles();

		for(int i = 0; i < (int)anchors.size(); i++)
		{
			float lengthDiff = (particles[anchors[i]].getXYZ() - particles[i].getXYZ()).magnitude() - tetherLengths[i];
			dbl->addLine(transform, particles[anchors[i]].getXYZ(), particles[i%particleCount].getXYZ(), lengthDiff>0.0f?0x0000FF:0x00FFFF);
		}
	}
}

void Scene::DebugRenderConstraints()
{
	DebugLineRenderBuffer* dbl = mSceneController->getDebugLineRenderBuffer();

	for(auto it : mClothList)
	{
		nv::cloth::Cloth& cloth = *it->mCloth;
		nv::cloth::Fabric& fabric = cloth.getFabric();
		if(fabric.getNumIndices() == 0)
			continue;

		nv::cloth::Factory& factory = cloth.getFactory();

		physx::PxMat44 transform = GetDebugDrawTransform(*it);

		nv::cloth::MappedRange<physx::PxVec4> particles = cloth.getCurrentParticles();

		if(sSceneDebugRenderParams.mVisiblePhaseRangeBegin >= sSceneDebugRenderParams.mVisiblePhaseRangeEnd)
		{
			//then simply render all constraints in one go
			std::vector<uint32_t> indices;
			indices.resize(fabric.getNumIndices());

			factory.extractFabricData(fabric, nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(&indices[0], &indices[0] + indices.size()),
				nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(0, 0));

			for(int i = 1; i < (int)indices.size(); i+=2)
			{
				dbl->addLine(transform, particles[indices[i]].getXYZ(), particles[indices[i-1]].getXYZ(), 0x991919);
			}
		}
		else
		{
			//otherwise we render individual phases
			std::vector<uint32_t> indices;
			indices.resize(fabric.getNumIndices());
			std::vector<uint32_t> phases;
			phases.resize(fabric.getNumPhases());
			std::vector<uint32_t> sets;
			sets.resize(fabric.getNumSets());

			factory.extractFabricData(fabric, nv::cloth::Range<uint32_t>(&phases[0], &phases[0] + phases.size()), nv::cloth::Range<uint32_t>(&sets[0], &sets[0] + sets.size()), nv::cloth::Range<float>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(&indices[0], &indices[0] + indices.size()),
				nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(0, 0));

			uint32_t* iIt = &indices[0];
			for(int phaseIndex = 0; phaseIndex < (int)fabric.getNumPhases(); phaseIndex++)
			{
				uint32_t* sIt = &sets[phases[phaseIndex]];
				uint32_t* iEnd = &indices[0] + (sIt[0] * 2);
				uint32_t* iStart = iIt;

				if(!(phaseIndex >= sSceneDebugRenderParams.mVisiblePhaseRangeBegin && phaseIndex < sSceneDebugRenderParams.mVisiblePhaseRangeEnd))
				{
					iIt = iEnd;
					continue;
				}

				for(iIt; iIt < iEnd; iIt+=2)
				{
					float c = (float)(iIt - iStart) / (float)(iEnd - iStart);
					unsigned char c255 = (unsigned char)(c*255.0f);

					unsigned int colorTable[3]
					{
						0xFF0000,
						0x00FF00,
						0x0000FF
					};
					unsigned int shiftTable[3]
					{
						8,
						0,
						16
					};

					dbl->addLine(transform, particles[*iIt].getXYZ(), particles[*(iIt+1)].getXYZ(), colorTable[phaseIndex%3] + (c255<< shiftTable[phaseIndex%3]));
				}
			}
		}
	}
}

void Scene::DebugRenderConstraintStiffness()
{
	DebugLineRenderBuffer* dbl = mSceneController->getDebugLineRenderBuffer();

	for(auto it : mClothList)
	{
		nv::cloth::Cloth& cloth = *it->mCloth;
		nv::cloth::Fabric& fabric = cloth.getFabric();
		if(fabric.getNumIndices() == 0)
			continue;

		if(!fabric.getNumStiffnessValues())
			continue;

		nv::cloth::Factory& factory = cloth.getFactory();

		physx::PxMat44 transform = GetDebugDrawTransform(*it);

		nv::cloth::MappedRange<physx::PxVec4> particles = cloth.getCurrentParticles();

		std::vector<uint32_t> indices;
		indices.resize(fabric.getNumIndices());
		std::vector<float> stiffness;
		stiffness.resize(fabric.getNumRestvalues());

		factory.extractFabricData(fabric, nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<float>(&stiffness[0], &stiffness[0] + stiffness.size()), nv::cloth::Range<uint32_t>(&indices[0], &indices[0] + indices.size()),
			nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(0, 0));

		for(int i = 1; i < (int)indices.size(); i+=2)
		{
			float c = 1.0f - exp2f(stiffness[i >> 1]);

			unsigned char ca255 = (unsigned char)(c*255.0f*0.8f);
			unsigned char cb255 = (unsigned char)((1.0f-c)*255.0f*0.8f);
			dbl->addLine(transform, particles[indices[i-1]].getXYZ(), particles[indices[i]].getXYZ(), (ca255<<8) + (cb255<<0));
		}
	}
}

void Scene::DebugRenderConstraintError()
{
	DebugLineRenderBuffer* dbl = mSceneController->getDebugLineRenderBuffer();

	for(auto it : mClothList)
	{
		nv::cloth::Cloth& cloth = *it->mCloth;
		nv::cloth::Fabric& fabric = cloth.getFabric();
		if(fabric.getNumRestvalues() == 0) { continue; }
		nv::cloth::Factory& factory = cloth.getFactory();

		physx::PxMat44 transform = GetDebugDrawTransform(*it);

		nv::cloth::MappedRange<physx::PxVec4> particles = cloth.getCurrentParticles();

		std::vector<uint32_t> indices;
		indices.resize(fabric.getNumIndices());
		std::vector<float> restLengths;
		restLengths.resize(fabric.getNumRestvalues());

		factory.extractFabricData(fabric, nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(&restLengths[0], &restLengths[0] + restLengths.size()), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(&indices[0], &indices[0] + indices.size()),
			nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(0, 0));

		for(int i = 0; i < (int)indices.size(); i += 2)
		{
			physx::PxVec4 p0 = particles[indices[i]];
			physx::PxVec4 p1 = particles[indices[i + 1]];
			float restLength = restLengths[i >> 1];
			float length = (p0 - p1).magnitude();
			const float scale = 2.0f;
			float error = (length / restLength * 0.5f - 0.5f) * scale + 0.5f;
			error = max(0.0f, min(1.0f, error));
			unsigned char c255 = (unsigned char)(error*255.0f*0.8f);
			dbl->addLine(transform, p0.getXYZ(), p1.getXYZ(), ((255-c255) << 8) + (c255 << 0));
		}
	}
}

void Scene::DebugRenderPositionDelta()
{
	DebugLineRenderBuffer* dbl = mSceneController->getDebugLineRenderBuffer();

	for(auto it : mClothList)
	{
		nv::cloth::Cloth& cloth = *it->mCloth;

		physx::PxMat44 transform = GetDebugDrawTransform(*it);

		nv::cloth::MappedRange<physx::PxVec4> particles1 = cloth.getCurrentParticles();
		nv::cloth::MappedRange<physx::PxVec4> particles0 = cloth.getPreviousParticles();

		std::vector<physx::PxVec4> lines;

		//scale so that the solver frequency doesn't affect the position delta length assuming 60fps
		float iterationsPerFrame = max(1, int(1.0f / 60.0f * cloth.getSolverFrequency() + 0.5f));

		for(int i = 0; i < (int)particles1.size(); i++)
		{
			dbl->addVector(transform, particles1[i].getXYZ(), (particles1[i] - particles0[i]).getXYZ()*iterationsPerFrame * 2.0f,0xFFFF00);
		}
	}
}

void Scene::DebugRenderBoundingBox()
{
	DebugLineRenderBuffer* dbl = mSceneController->getDebugLineRenderBuffer();

	for(auto it : mClothList)
	{
		nv::cloth::Cloth& cloth = *it->mCloth;

		physx::PxMat44 transform = GetDebugDrawTransform(*it);

		physx::PxVec3 c = cloth.getBoundingBoxCenter();
		physx::PxVec3 d = cloth.getBoundingBoxScale();
		physx::PxVec3 dx = physx::PxVec3(d.x, 0.0f, 0.0f);
		physx::PxVec3 dy = physx::PxVec3(0.0f, d.y, 0.0f);
		physx::PxVec3 dz = physx::PxVec3(0.0f, 0.0f, d.z);

		dbl->addLine(transform,c + dy + dz - dx,c + dy + dz + dx, 0x00FFFF);
		dbl->addLine(transform,c + dy - dz - dx,c + dy - dz + dx, 0x00FFFF);
		dbl->addLine(transform,c - dy + dz - dx,c - dy + dz + dx, 0x00FFFF);
		dbl->addLine(transform,c - dy - dz - dx,c - dy - dz + dx, 0x00FFFF);
		dbl->addLine(transform,c + dy + dx - dz,c + dy + dx + dz, 0x00FFFF);
		dbl->addLine(transform,c + dy - dx - dz,c + dy - dx + dz, 0x00FFFF);
		dbl->addLine(transform,c - dy + dx - dz,c - dy + dx + dz, 0x00FFFF);
		dbl->addLine(transform,c - dy - dx - dz,c - dy - dx + dz, 0x00FFFF);
		dbl->addLine(transform,c + dz + dx - dy,c + dz + dx + dy, 0x00FFFF);
		dbl->addLine(transform,c + dz - dx - dy,c + dz - dx + dy, 0x00FFFF);
		dbl->addLine(transform,c - dz + dx - dy,c - dz + dx + dy, 0x00FFFF);
		dbl->addLine(transform,c - dz - dx - dy,c - dz - dx + dy, 0x00FFFF);
		dbl->addLine(transform,c + dy + dz + dx,c - dy - dz - dx, 0x007777);
		dbl->addLine(transform,c + dy + dz - dx,c - dy - dz + dx, 0x007777);
		dbl->addLine(transform,c - dy + dz + dx,c + dy - dz - dx, 0x007777);
		dbl->addLine(transform,c - dy + dz - dx,c + dy - dz + dx, 0x007777);
	}
}