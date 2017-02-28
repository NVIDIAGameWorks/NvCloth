/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef SIMPLE_SCENE_H
#define SIMPLE_SCENE_H

#include "scene/Scene.h"

class SimpleScene : public Scene
{
public:

	SimpleScene(SceneController* sceneController):Scene(sceneController) {}

	virtual void onInitialize() override;

private:
	nv::cloth::Fabric* mFabric;
	nv::cloth::Solver* mSolver;
	ClothActor* mClothActor;

};


#endif