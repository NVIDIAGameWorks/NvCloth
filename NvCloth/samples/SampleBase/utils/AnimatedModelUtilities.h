/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#pragma once
#include <vector>
#include <string>
#include <PxVec4.h>
#include <PxVec3.h>
#include <PxMat44.h>
class Model;
class Scene;

bool saveBoneCapsuleData(std::string filepath, Model* model,
	std::vector<physx::PxVec4>& sphereOffsets, std::vector<uint32_t>& activeSpheres, std::vector<uint32_t>& capsuleNodes);

bool loadBoneCapsuleData(std::string filepath, Model* model,
	std::vector<physx::PxVec4>& sphereOffsets, std::vector<uint32_t>& activeSpheres, std::vector<uint32_t>& capsuleNodes);

void renderBoneLines(Scene* scene, Model* model, ModelInstance* instance, physx::PxMat44 transform = physx::PxMat44(physx::PxIdentity), float scale = 1.0f);