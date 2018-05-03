/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef TELEPORT_SCENE_H
#define TELEPORT_SCENE_H

#include "scene/Scene.h"
#include <foundation/PxVec3.h>

class TeleportScene : public Scene
{
public:

	TeleportScene(SceneController* sceneController): Scene(sceneController) {}

	virtual void Animate(double dt) override;
	void initializeCloth(int index, physx::PxVec3 offset);
	virtual void onInitialize() override;
	void teleport();

private:
	nv::cloth::Fabric* mFabric[1];
	nv::cloth::Solver* mSolver[1];
	ClothActor* mClothActor[1];

	int mAttachmentVertices[1];
	physx::PxVec4 mAttachmentVertexOriginalPositions[1];

	float mTime;

};


#endif