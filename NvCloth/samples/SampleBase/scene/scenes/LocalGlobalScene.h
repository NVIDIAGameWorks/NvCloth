/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef LOCAL_GLOBAL_SCENE_H
#define LOCAL_GLOBAL_SCENE_H

#include "scene/Scene.h"
#include <foundation/PxVec3.h>

class LocalGlobalScene : public Scene
{
public:

	LocalGlobalScene(SceneController* sceneController): Scene(sceneController) {}

	virtual void Animate(double dt) override;
	void initializeCloth(int index, physx::PxVec3 offset);
	virtual void onInitialize() override;

private:
	nv::cloth::Fabric* mFabric[2];
	nv::cloth::Solver* mSolver[2];
	ClothActor* mClothActor[2];

	int mAttachmentVertices[2];
	physx::PxVec4 mAttachmentVertexOriginalPositions[2];

	float mTime;

};


#endif