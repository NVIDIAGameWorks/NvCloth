// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#pragma once
#include <foundation/PxVec2.h>
#include <foundation/PxVec3.h>
#include <foundation/PxVec4.h>
#include <foundation/PxMat44.h>
#include <foundation/PxStrideIterator.h>
#include <vector>
#include <algorithm>

namespace nv
{
namespace cloth
{

class Cloth;

namespace collisionVisualization
{

// struct used to store cashed meshes in
// the initialize and ~destructor methods use nv::cloth::(de)allocate()
struct SimpleMesh
{
	SimpleMesh();
	~SimpleMesh();
	void initialize(int vertexCount, int indexCount);
	physx::PxVec3* mPositions;
	physx::PxVec3* mNormals;
	physx::PxVec2* mUvs;
	unsigned int* mIndices;

	int mVertexCount;
	int mIndexCount;

	physx::PxStrideIterator<physx::PxVec3> GetPositionIterator() { return physx::PxStrideIterator<physx::PxVec3>(mPositions); }
	physx::PxStrideIterator<physx::PxVec3> GetNormalIterator() { return physx::PxStrideIterator<physx::PxVec3>(mNormals); }
	physx::PxStrideIterator<physx::PxVec2> GetUvIterator() { return physx::PxStrideIterator<physx::PxVec2>(mUvs); }
	physx::PxStrideIterator<unsigned int> GetIndexIterator() { return physx::PxStrideIterator<unsigned int>(mIndices); }

};

//Generates simple meshes with smooth shading
//Use the Get*MemorySize to calculate how much memory the generate* function needs
//outNormals and outUvs are optional, they can be left nullptr

//Spheres
void getSphereMemorySize(int segmentsX, int segmentsY, int* outVertexCount, int* outIndexCount);
void generateSphere(int segmentsX, int segmentY, physx::PxMat44 transform, 
	physx::PxStrideIterator<unsigned int> outIndices, int indexOffset,
	physx::PxStrideIterator<physx::PxVec3> outPositions, physx::PxStrideIterator<physx::PxVec3> outNormals = physx::PxStrideIterator<physx::PxVec3>(), physx::PxStrideIterator<physx::PxVec2> outUvs = physx::PxStrideIterator<physx::PxVec2>());

//Cylinders without caps
void getCylinderMemorySize(int segmentsX, int segmentsY, int* outVertexCount, int* outIndexCount);
void generateCylinder(int segmentsX, int segmentsY, physx::PxMat44 transform,
	physx::PxStrideIterator<unsigned int> outIndices, int indexOffset,
	physx::PxStrideIterator<physx::PxVec3> outPositions, physx::PxStrideIterator<physx::PxVec3> outNormals = physx::PxStrideIterator<physx::PxVec3>(), physx::PxStrideIterator<physx::PxVec2> outUvs = physx::PxStrideIterator<physx::PxVec2>());


//Combines cashed spheres and cylinders to generate the capsules
void getCollisionCapsuleMemorySize(int sphereCount, int indexCount, SimpleMesh const& cachedSphere, SimpleMesh const& cachedCylinder, int* outVertexCount, int* outIndexCount);
void generateCollisionCapsules(physx::PxVec4 const* spheres, int sphereCount, uint32_t const* indices, int indexCount, float grow,
	SimpleMesh const& cachedSphere, SimpleMesh const& cachedCylinder,
	physx::PxStrideIterator<unsigned int> outIndices, int indexOffset,
	physx::PxStrideIterator<physx::PxVec3> outPositions, physx::PxStrideIterator<physx::PxVec3> outNormals = physx::PxStrideIterator<physx::PxVec3>(), physx::PxStrideIterator<physx::PxVec2> outUvs = physx::PxStrideIterator<physx::PxVec2>());
void getCollisionCapsuleSubmeshOffsets(int sphereCount, int indexCount, SimpleMesh const& cachedSphere, SimpleMesh const& cachedCylinder, physx::PxStrideIterator<unsigned int> submehsOffsets);

}
}
}