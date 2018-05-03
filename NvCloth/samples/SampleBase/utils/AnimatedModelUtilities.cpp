/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/


#include "DataStream.h"
#include "renderer/Model.h"
#include "scene/Scene.h"
#include "utils/DebugLineRenderBuffer.h"
#include <map>

bool saveBoneCapsuleData(std::string filepath, Model* model, 
	std::vector<physx::PxVec4>& sphereOffsets, std::vector<uint32_t>& activeSpheres, std::vector<uint32_t>& capsuleNodes)
{
	DataStream stream;
	uint32_t sphereOffsetCount = (uint32_t)sphereOffsets.size();
	stream.write(sphereOffsetCount);
	for(int i = 0; i < (int)sphereOffsets.size(); i++)
	{
		stream.write(std::string(model->getNodeName(i)));
		stream.write(sphereOffsets[i]);
	}
	uint32_t activeSphereCount = (uint32_t)activeSpheres.size();
	stream.write(activeSphereCount);
	for(int i = 0; i < (int)activeSpheres.size(); i++)
	{
		stream.write(activeSpheres[i]);
	}
	uint32_t capsuleNodeCount = (uint32_t)capsuleNodes.size();
	stream.write(capsuleNodeCount);
	for(int i = 0; i < (int)capsuleNodes.size(); i++)
	{
		stream.write(capsuleNodes[i]);
	}

	stream.saveToFile(filepath.c_str());
	return true;
}

bool loadBoneCapsuleData(std::string filepath, Model* model,
	std::vector<physx::PxVec4>& sphereOffsets, std::vector<uint32_t>& activeSpheres, std::vector<uint32_t>& capsuleNodes)
{
	if(!DataStream::fileExists(filepath.c_str()))
		return false;

	assert(sphereOffsets.size() == model->getNodeCount());

	DataStream stream(filepath.c_str());
	std::map<int, int> fileNodeIdToModelNodeId;
	uint32_t sphereOffsetCount = stream.read<uint32_t>();
	for(int i = 0; i < (int)sphereOffsetCount; i++)
	{
		auto nodeName = stream.read<std::string>();
		int modelNode = model->getNodeIdByNameWithErrorCode(nodeName);
		fileNodeIdToModelNodeId[i] = modelNode;
		if(modelNode == -1)
		{
			stream.read<physx::PxVec4>();
			printf("Warning: node [%d] \"%s\" not found in model (%s)\n", i, nodeName.c_str(), filepath.c_str());
			continue;
		}
		sphereOffsets[modelNode] = stream.read<physx::PxVec4>();
	}
	activeSpheres.resize(stream.read<uint32_t>());
	for(int i = 0; i < (int)activeSpheres.size(); i++)
	{
		int fileNode = stream.read<uint32_t>();
		if(fileNodeIdToModelNodeId[fileNode] == -1)
			continue;
		activeSpheres[i] = fileNodeIdToModelNodeId[fileNode];
	}
	capsuleNodes.resize(stream.read<uint32_t>());
	for(int i = 0; i < (int)capsuleNodes.size(); i++)
	{
		int fileNode = stream.read<uint32_t>();
		if(fileNodeIdToModelNodeId[fileNode] == -1)
			continue;
		capsuleNodes[i] = fileNodeIdToModelNodeId[fileNode];
	}
	return true;
}

void renderBoneLines(Scene* scene, Model* model, ModelInstance* instance, physx::PxMat44 transform, float scale)
{
	DebugLineRenderBuffer* dbl = scene->getSceneController()->getDebugLineRenderBuffer();

	//Render bone lines
	model->traverseNodes([&transform, scale, instance, dbl](int nodeId, int parrentId)
	{
		physx::PxVec3 a = instance->mNodes[parrentId].mTransform.transform(physx::PxVec3(0.0f, 0.0f, 0.0f));
		physx::PxVec3 b = instance->mNodes[nodeId].mTransform.transform(physx::PxVec3(0.0f, 0.0f, 0.0f));
		a = transform.transform(a * scale);
		b = transform.transform(b * scale);
		dbl->addLine(a, b, 0xFFFFFFFF);
	});
}