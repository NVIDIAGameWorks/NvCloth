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
#include "NvClothAuthoringLibrary/CollisionVisualization.h"
#include "NvCloth/Cloth.h"
#include "NvCloth/Fabric.h"
#include "NvCloth/Factory.h"
#include <assert.h>
#include <algorithm>
#include "NvCloth/Allocator.h"
#include "NvCloth/Callbacks.h"

namespace nv
{
namespace cloth
{
namespace collisionVisualization
{

SimpleMesh::SimpleMesh()
	:
mPositions(nullptr),
mNormals(nullptr),
mUvs(nullptr),
mIndices(nullptr),
mVertexCount(0),
mIndexCount(0)
{

}
SimpleMesh::~SimpleMesh()
{
	auto& allocator = *GetNvClothAllocator();
	allocator.deallocate(mPositions);
	allocator.deallocate(mNormals);
	allocator.deallocate(mUvs);
	allocator.deallocate(mIndices);	
}
void SimpleMesh::initialize(int vertexCount, int indexCount)
{
	mVertexCount = vertexCount;
	mIndexCount = indexCount;

	auto& allocator = *GetNvClothAllocator();
	
	mPositions = static_cast<physx::PxVec3*>(
		allocator.allocate(sizeof(physx::PxVec3)*vertexCount, "nv::cloth::collisionVisualization::SimpleMesh::mPositions", __FILE__, __LINE__)
		);
	mNormals = static_cast<physx::PxVec3*>(
		allocator.allocate(sizeof(physx::PxVec3)*vertexCount, "nv::cloth::collisionVisualization::SimpleMesh::mNormals", __FILE__, __LINE__)
		);
	mUvs = static_cast<physx::PxVec2*>(
		allocator.allocate(sizeof(physx::PxVec2)*vertexCount, "nv::cloth::collisionVisualization::SimpleMesh::mUvs", __FILE__, __LINE__)
		);
	mIndices = static_cast<unsigned int*>(
		allocator.allocate(sizeof(unsigned int)*indexCount, "nv::cloth::collisionVisualization::SimpleMesh::mIndices", __FILE__, __LINE__)
		);
}

void getSphereMemorySize(int segmentsX, int segmentsY, int* outVertexCount, int* outIndexCount)
{
	const int xSegments = segmentsX;
	const int ySegments = segmentsY;

	*outVertexCount = 1 + (xSegments * (ySegments - 1)) + 1;
	*outIndexCount = xSegments * 3 + 6 * (xSegments * (ySegments - 2)) + xSegments * 3;
}

void generateSphere(int segmentsX, int segmentY, physx::PxMat44 transform,
	physx::PxStrideIterator<unsigned int> outIndices, int indexOffset,
	physx::PxStrideIterator<physx::PxVec3> outPositions, physx::PxStrideIterator<physx::PxVec3> outNormals, physx::PxStrideIterator<physx::PxVec2> outUvs) 
{
	const int xSegments = segmentsX;
	const int ySegments = segmentY;

	physx::PxStrideIterator<physx::PxVec3> vertexIteratorPos = outPositions;
	physx::PxStrideIterator<physx::PxVec3> vertexIteratorNormal = outNormals;
	physx::PxStrideIterator<physx::PxVec2> vertexIteratorUv = outUvs;

	{
		//bottom
		physx::PxVec3 pos = physx::PxVec3(0.0f, -1.0f, 0.0f);
		*vertexIteratorPos++ = transform.transform(pos);
		if(outNormals.ptr()) *vertexIteratorNormal++ = transform.rotate(physx::PxVec4(pos, 0.0f)).getXYZ();
		if(outUvs.ptr()) *vertexIteratorUv++ = physx::PxVec2(0.0f, 0.0f);
	}

	//middle
	for(int y = 1; y < ySegments; y++)
	{
		for(int x = 0; x < xSegments; x++)
		{
			float xf = (float)x / (xSegments - 1.0f);
			float yaw = xf*physx::PxTwoPi;
			float yf = (float)y / (ySegments);
			float pitch = (yf - 0.5f)*physx::PxPi;

			physx::PxVec3 pos = physx::PxVec3(cos(yaw)*cos(pitch), sin(pitch), sin(yaw)*cos(pitch));;
			*vertexIteratorPos++ = transform.transform(pos);
			if(outNormals.ptr()) *vertexIteratorNormal++ = transform.rotate(physx::PxVec4(pos, 0.0f)).getXYZ();
			if(outUvs.ptr()) *vertexIteratorUv++ = physx::PxVec2(xf, yf);
		}
	}

	{
		//top
		physx::PxVec3 pos = physx::PxVec3(0.0f, 1.0f, 0.0f);
		*vertexIteratorPos++ = transform.transform(pos);
		if(outNormals.ptr()) *vertexIteratorNormal++ = transform.rotate(physx::PxVec4(pos, 0.0f)).getXYZ();
		if(outUvs.ptr()) *vertexIteratorUv++ = physx::PxVec2(0.0f, 0.0f);
	}

	physx::PxStrideIterator<unsigned int> indexIterator = outIndices;

	//bottom cap
	for(int x = 0; x < xSegments; x++)
	{
		*indexIterator++ = indexOffset;
		*indexIterator++ = 1 + x + indexOffset;
		*indexIterator++ = 1 + (x + 1) % xSegments + indexOffset;
	}

	const auto RingVertex = [xSegments, ySegments](int x, int y)
	{
		return 1 + y*xSegments + x%xSegments;
	};

	//middle
	for(int y = 0; y < ySegments - 2; y++)
	{
		for(int x = 0; x < xSegments; x++)
		{
			*indexIterator++ = RingVertex(x, y) + indexOffset;
			*indexIterator++ = RingVertex(x + 1, y) + indexOffset;
			*indexIterator++ = RingVertex(x, y + 1) + indexOffset;

			*indexIterator++ = RingVertex(x + 1, y) + indexOffset;
			*indexIterator++ = RingVertex(x + 1, y + 1) + indexOffset;
			*indexIterator++ = RingVertex(x, y + 1) + indexOffset;
		}
	}

	int numVertices = vertexIteratorPos - outPositions;

	//bottom cap
	for(int x = 0; x < xSegments; x++)
	{
		*indexIterator++ = numVertices - 1 + indexOffset;
		*indexIterator++ = RingVertex(x, ySegments - 2) + indexOffset;
		*indexIterator++ = RingVertex(x + 1, ySegments - 2) + indexOffset;
	}
}

void getCylinderMemorySize(int segmentsX, int segmentsY, int* outVertexCount, int* outIndexCount)
{
	const int xSegments = segmentsX;
	const int ySegments = segmentsY;

	*outVertexCount = xSegments * (ySegments + 1);
	*outIndexCount = xSegments * ySegments * 6;
}




void generateCylinder(int segmentsX, int segmentsY, physx::PxMat44 transform,
	physx::PxStrideIterator<unsigned int> outIndices, int indexOffset,
	physx::PxStrideIterator<physx::PxVec3> outPositions, physx::PxStrideIterator<physx::PxVec3> outNormals, physx::PxStrideIterator<physx::PxVec2> outUvs)
{
	const int xSegments = segmentsX;
	const int ySegments = segmentsY;

	physx::PxStrideIterator<physx::PxVec3> vertexIteratorPos = outPositions;
	physx::PxStrideIterator<physx::PxVec3> vertexIteratorNormal = outNormals;
	physx::PxStrideIterator<physx::PxVec2> vertexIteratorUv = outUvs;

	//middle
	for(int y = 0; y < ySegments + 1; y++)
	{
		for(int x = 0; x < xSegments; x++)
		{
			float xf = (float)x / (xSegments - 1.0f);
			float yaw = xf*physx::PxTwoPi;
			float yf = (float)y / (ySegments) * 2.0f - 1.0f;

			physx::PxVec3 pos = physx::PxVec3(cos(yaw), yf, sin(yaw));
			*vertexIteratorPos++ = transform.transform(pos);
			if(outNormals.ptr()) *vertexIteratorNormal++ = transform.rotate(physx::PxVec4(physx::PxVec3(cos(yaw), 0.0f, sin(yaw)), 0.0f)).getXYZ();
			if(outUvs.ptr()) *vertexIteratorUv++ = physx::PxVec2(xf, yf);
		}
	}

	physx::PxStrideIterator<unsigned int> indexIterator = outIndices;


	const auto RingVertex = [xSegments, ySegments](int x, int y)
	{
		return y*xSegments + x%xSegments;
	};

	//middle
	for(int y = 0; y < ySegments; y++)
	{
		for(int x = 0; x < xSegments; x++)
		{
			*indexIterator++ = RingVertex(x, y) + indexOffset;
			*indexIterator++ = RingVertex(x + 1, y) + indexOffset;
			*indexIterator++ = RingVertex(x, y + 1) + indexOffset;

			*indexIterator++ = RingVertex(x + 1, y) + indexOffset;
			*indexIterator++ = RingVertex(x + 1, y + 1) + indexOffset;
			*indexIterator++ = RingVertex(x, y + 1) + indexOffset;
		}
	}
}

void getCollisionCapsuleMemorySize(int sphereCount, int indexCount, SimpleMesh const& cachedSphere, SimpleMesh const& cachedCylinder, int* outVertexCount, int* outIndexCount)
{
	*outVertexCount = cachedSphere.mVertexCount*sphereCount + cachedCylinder.mVertexCount*(indexCount / 2);
	*outIndexCount = cachedSphere.mIndexCount*sphereCount + cachedCylinder.mIndexCount*(indexCount / 2);
}

namespace
{
	/** returns two vectors in b and c so that [a b c] form a basis.
	* a needs to be a unit vector.
	*/
	inline void computeBasis(const physx::PxVec3& a, physx::PxVec3* b, physx::PxVec3* c)
	{
		if(fabsf(a.x) >= 0.57735f)
			*b = physx::PxVec3(a.y, -a.x, 0.0f);
		else
			*b = physx::PxVec3(0.0f, a.z, -a.y);

		*b = b->getNormalized();
		*c = a.cross(*b);
	}

	physx::PxVec3 IntersectSpheres(float* circleRadius, physx::PxVec3 aCenter, float aRadius, physx::PxVec3 bCenter, float bRadius)
	{
		//Intersect spheres in 2d (http://paulbourke.net/geometry/circlesphere/ Intersection of two circles)
		float d = (aCenter - bCenter).magnitude();
		float a = (aRadius*aRadius - bRadius*bRadius + d*d) / (2.0f*d);
		float h = sqrtf(aRadius*aRadius - a*a);
		physx::PxVec3 P3 = aCenter + a * (bCenter - aCenter) / d;
		if(circleRadius) *circleRadius = h;
		return P3;
	}
}

void generateCollisionCapsules(physx::PxVec4 const* spheres, int sphereCount, uint32_t const* indices, int indexCount, float grow,
	SimpleMesh const& cachedSphere, SimpleMesh const& cachedCylinder,
	physx::PxStrideIterator<unsigned int> outIndices, int indexOffset,
	physx::PxStrideIterator<physx::PxVec3> outPositions, physx::PxStrideIterator<physx::PxVec3> outNormals, physx::PxStrideIterator<physx::PxVec2> outUvs)
{
	physx::PxStrideIterator<physx::PxVec3> vertexIteratorPos = outPositions;
	physx::PxStrideIterator<physx::PxVec3> vertexIteratorNormal = outNormals;
	physx::PxStrideIterator<physx::PxVec2> vertexIteratorUv = outUvs;

	physx::PxStrideIterator<unsigned int> indexIterator = outIndices;

	int nextVertex = 0;
	int nextIndex = 0;
	for(int i = 0; i < sphereCount; i++)
	{
		int baseIndex = nextVertex;
		physx::PxMat44 transform =
			physx::PxMat44(physx::PxMat33(physx::PxIdentity), spheres[i].getXYZ())
			* physx::PxMat44(physx::PxVec4(spheres[i].w + grow, spheres[i].w + grow, spheres[i].w + grow, 1.0f));

		for(int vi = 0; vi<cachedSphere.mVertexCount; vi++)
		{
			physx::PxVec3 pos = cachedSphere.mPositions[vi];
			*vertexIteratorPos++ = transform.transform(pos);
			if(outNormals.ptr()) *vertexIteratorNormal++ = transform.rotate(physx::PxVec4(cachedSphere.mNormals[vi], 0.0f)).getXYZ();
			if(outUvs.ptr()) *vertexIteratorUv++ = cachedSphere.mUvs[vi];
		}

		for(int ii = 0; ii < cachedSphere.mIndexCount; ii++)
		{
			*indexIterator++ = cachedSphere.mIndices[ii] + baseIndex + indexOffset;
		}
		nextVertex += cachedSphere.mVertexCount;
	}

	for(int i = 0; i < indexCount; i += 2)
	{
		int baseIndex = nextVertex;

		physx::PxVec3 spherePosA = spheres[indices[i]].getXYZ();
		physx::PxVec3 spherePosB = spheres[indices[i + 1]].getXYZ();
		float sphereRadiusA = spheres[indices[i]].w + grow;
		float sphereRadiusB = spheres[indices[i + 1]].w + grow;

		if(sphereRadiusA < sphereRadiusB)
		{
			std::swap(sphereRadiusA, sphereRadiusB);
			std::swap(spherePosA, spherePosB);
		}

		{
			//http://jwilson.coe.uga.edu/emt669/Student.Folders/Kertscher.Jeff/Essay.3/Tangents.html

			//sphere a with smaller radius
			float cRadius = sphereRadiusA - sphereRadiusB;
			if(cRadius > 0.00001)
			{
				physx::PxVec3 basis[3];
				basis[2] = spherePosB - spherePosA;
				basis[2].normalize();
				computeBasis(basis[2], &basis[0], &basis[1]);

				physx::PxVec3 cCenter = spherePosA;

				//sphere in between the a and b
				physx::PxVec3 dCenter = (spherePosA + spherePosB)*0.5f;
				float dRadius = (spherePosA - spherePosB).magnitude()*0.5f;

				//intersection between c and d to get tangent point
				float iRadius;
				physx::PxVec3 iCenter = IntersectSpheres(&iRadius, dCenter, dRadius, cCenter, cRadius);
				physx::PxVec3 iPoint = iCenter + basis[0] * iRadius; //tangent point on c
				physx::PxVec3 offset = (iPoint - spherePosA).getNormalized(); //offset direction

				physx::PxVec3 aPoint = spherePosA + offset*sphereRadiusA;
				spherePosA = (aPoint - spherePosA).dot(basis[2])*basis[2] + spherePosA;
				sphereRadiusA = (aPoint - spherePosA).magnitude();
				physx::PxVec3 bPoint = spherePosB + offset*sphereRadiusB;
				spherePosB = (bPoint - spherePosA).dot(basis[2])*basis[2] + spherePosA;
				sphereRadiusB = (bPoint - spherePosB).magnitude();
			}
		}

		float length = (spherePosB - spherePosA).magnitude();


		physx::PxMat44 scaleA = physx::PxMat44(physx::PxVec4(sphereRadiusA, length / 2.0f, sphereRadiusA + grow, 1.0f));
		physx::PxMat44 scaleB = physx::PxMat44(physx::PxVec4(sphereRadiusB, length / 2.0f, sphereRadiusB, 1.0f));

		physx::PxQuat orientation;
		{
			physx::PxVec3 u = physx::PxVec3(0.0f, 1.0f, 0.0f);
			physx::PxVec3 v = spherePosB - spherePosA;
			v.normalize();

			if(u.dot(v) < -0.9999 || u.dot(v) > 0.9999)
			{
				physx::PxVec3 orth, tmp;
				computeBasis(u, &orth, &tmp);
				orientation = physx::PxQuat(orth.x, orth.y, orth.z, 0.0f);
			}
			else
			{
				physx::PxVec3 half = u + v;
				half.normalize();
				physx::PxVec3 imaginary = u.cross(half);
				orientation = physx::PxQuat(imaginary.x, imaginary.y, imaginary.z, u.dot(half));
			}
		}

		physx::PxMat44 transform = physx::PxMat44(physx::PxTransform(spherePosA, orientation))*scaleA;

		int firstRing = cachedCylinder.mVertexCount / 2;
		for(int vi = 0; vi<firstRing; vi++)
		{
			physx::PxVec3 pos = cachedCylinder.mPositions[vi];
			*vertexIteratorPos++ = transform.transform(pos);
			if(outNormals.ptr()) *vertexIteratorNormal++ = transform.rotate(physx::PxVec4(cachedCylinder.mNormals[vi], 0.0f)).getXYZ();
			if(outUvs.ptr()) *vertexIteratorUv++ = cachedCylinder.mUvs[vi];
		}
		transform = physx::PxMat44(physx::PxTransform(spherePosA, orientation))*scaleB;
		for(int vi = firstRing; vi<cachedCylinder.mVertexCount; vi++)
		{
			physx::PxVec3 pos = cachedCylinder.mPositions[vi];
			*vertexIteratorPos++ = transform.transform(pos);
			if(outNormals.ptr()) *vertexIteratorNormal++ = transform.rotate(physx::PxVec4(cachedCylinder.mNormals[vi], 0.0f)).getXYZ();
			if(outUvs.ptr()) *vertexIteratorUv++ = cachedCylinder.mUvs[vi];
		}

		nextVertex += cachedCylinder.mVertexCount;

		for(int ii = 0; ii < cachedCylinder.mIndexCount; ii++)
		{
			*indexIterator++ = cachedCylinder.mIndices[ii] + baseIndex + indexOffset;
		}
	}
}

void getCollisionCapsuleSubmeshOffsets(int sphereCount, int indexCount, SimpleMesh const& cachedSphere, SimpleMesh const& cachedCylinder, physx::PxStrideIterator<unsigned int> submehsOffsets)
{
	int nextOffset = 0;
	for(int i = 0; i < sphereCount; i++)
	{
		*submehsOffsets++ = nextOffset;
		nextOffset += cachedSphere.mIndexCount;
	}
	for(int i = 0; i < indexCount; i+=2)
	{
		*submehsOffsets++ = nextOffset;
		nextOffset += cachedCylinder.mIndexCount;
	}
}


}
}
}