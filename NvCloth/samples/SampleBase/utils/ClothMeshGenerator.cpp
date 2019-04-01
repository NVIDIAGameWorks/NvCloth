/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "ClothMeshGenerator.h"

#include <NvCloth/ps/PsMathUtils.h>

#include <fstream>
#include <iterator>
#include <algorithm>
#include <assert.h>

namespace
{
	template<typename T>
	std::vector<T> readValuesFromFile(const std::string& path)
	{
		std::ifstream inputFile(path);
		std::vector<T> data{ std::istream_iterator<T>{inputFile}, { } };
		return std::move(data);
	}
} // end of anonymous namespace


void ClothMeshData::Clear()
{
	mVertices.clear();
	mTriangles.clear();
	mQuads.clear();
}

void ClothMeshData::GeneratePlaneCloth(float width, float height, int segmentsX, int segmentsY, bool createQuads, physx::PxMat44 transform, bool alternatingDiagonals, int zigzag)
{
/*
GeneratePlaneCloth(x,y,2,2) generates:

    v0______v1_____v2     v0______v1_____v2
     |      |      |       |\     |\     |
     |  Q0  |  Q1  |       |  \t0 |  \t2 |
     |      |      |       | t1 \ | t3 \ |
    v3------v4-----v5     v3-----\v4----\v5
     |      |      |       | \    | \    |
     |  Q2  |  Q3  |       |   \t4|   \t6|
     |______|______|       |_t5_\_|_t7__\|
    v6      v7     v8     v6      v7     v8
*/


	Clear();
	mVertices.resize((segmentsX + 1) * (segmentsY + 1));
	mInvMasses.resize((segmentsX + 1) * (segmentsY + 1));
	mTriangles.resize(segmentsX * segmentsY * 2);
	if (createQuads)
		mQuads.resize(segmentsX * segmentsY);
	
	mMesh.vertices.resize(mVertices.size());
	mMesh.indices.resize(3 * mTriangles.size());

	physx::PxVec3 topLeft(-width * 0.5f, 0.f, -height * 0.5f);
//	vec3 topLeftGLM(-width * 0.5f, translateUp, -height * 0.5f);

	//calculate uv scale and offset to keep texture aspect ratio 1:1
	float uvSx = width > height ? 1.0f : width / height;
	float uvSy = width > height ? height / width : 1.0f;
	float uvOx = 0.5f * (1.0f - uvSx);
	float uvOy = 0.5f * (1.0f - uvSy);

	// Vertices
	for (int y = 0; y < segmentsY + 1; y++)
	{
		for(int x = 0; x < segmentsX + 1; x++)
		{
			physx::PxVec3 pos;
			switch(zigzag)
			{
				case 1:
					pos =	physx::PxVec3(((float)x / (float)segmentsX) * width,
							sinf(y*0.5)/(float)segmentsY * height,
							((float)y / (float)segmentsY) * height);
					break;
				case 2:
					pos =	physx::PxVec3(((float)x / (float)segmentsX) * width,
							((float)(y&2) / (float)segmentsY) * height,
							((float)((y+1)&~1) / (float)segmentsY) * height);
					break;
				default:
					pos =	physx::PxVec3(((float)x / (float)segmentsX) * width,
							0.f,
							((float)y / (float)segmentsY) * height);
			}

			mVertices[x + y * (segmentsX + 1)] = transform.transform(topLeft + pos);

			mInvMasses[x + y * (segmentsX + 1)] = 1.0f;

			mMesh.vertices[x + y * (segmentsX + 1)].position = transform.transform(topLeft + pos);

			mMesh.vertices[x + y * (segmentsX + 1)].normal = transform.transform(physx::PxVec3(0.f, 1.f, 0.f));

			mMesh.vertices[x + y * (segmentsX + 1)].uv = physx::PxVec2(uvOx + uvSx*(float)x / (float)segmentsX, uvOy + uvSy*(1.0f - (float)y / (float)segmentsY));
		}
	}

	if (createQuads)
	{
		// Quads
		for (int y = 0; y < segmentsY; y++)
		{
			for (int x = 0; x < segmentsX; x++)
			{
				mQuads[(x + y * segmentsX)] = Quad((uint32_t)(x + 0) + (y + 0) * (segmentsX + 1),
					(uint32_t)(x + 1) + (y + 0) * (segmentsX + 1),
					(uint32_t)(x + 1) + (y + 1) * (segmentsX + 1),
					(uint32_t)(x + 0) + (y + 1) * (segmentsX + 1));
			}
		}
	}

	// Triangles
	for (int y = 0; y < segmentsY; y++)
	{
		for(int x = 0; x < segmentsX; x++)
		{
			if(alternatingDiagonals && (x^y)&1)
			{
				//Top right to bottom left
				mTriangles[(x + y * segmentsX) * 2 + 0] = Triangle( (uint32_t)(x + 0) + (y + 0) * (segmentsX + 1),
																	(uint32_t)(x + 1) + (y + 0) * (segmentsX + 1),
																	(uint32_t)(x + 0) + (y + 1) * (segmentsX + 1));
																		    	   	      
				mTriangles[(x + y * segmentsX) * 2 + 1] = Triangle( (uint32_t)(x + 1) + (y + 0) * (segmentsX + 1),
																	(uint32_t)(x + 1) + (y + 1) * (segmentsX + 1),
																	(uint32_t)(x + 0) + (y + 1) * (segmentsX + 1));
			}
			else
			{
				//Top left to bottom right
				mTriangles[(x + y * segmentsX) * 2 + 0] = Triangle( (uint32_t)(x + 0) + (y + 0) * (segmentsX + 1),
																	(uint32_t)(x + 1) + (y + 0) * (segmentsX + 1),
																	(uint32_t)(x + 1) + (y + 1) * (segmentsX + 1));
																		    	   	      
				mTriangles[(x + y * segmentsX) * 2 + 1] = Triangle( (uint32_t)(x + 0) + (y + 0) * (segmentsX + 1),
																	(uint32_t)(x + 1) + (y + 1) * (segmentsX + 1),
																	(uint32_t)(x + 0) + (y + 1) * (segmentsX + 1));
			}
		}																    		 
	}

	for (int i = 0; i < (int)mTriangles.size(); i++)
	{
		mMesh.indices[3 * i] = mTriangles[i].a;
		mMesh.indices[3 * i + 1] = mTriangles[i].b;
		mMesh.indices[3 * i + 2] = mTriangles[i].c;
	}
}

void ClothMeshData::GenerateCylinderWave(float radiusTop, float radiusBottom, float height, float frequency, float ampitudeTop, float ampitudeBottom, int segmentsX, int segmentsY, physx::PxMat44 transform, bool attachTop, bool attachBottom, bool createQuads, int missingXsegments)
{
	Clear();
	int particleXsegments = segmentsX - std::max(0, missingXsegments - 1);
	int triangleXsegments = segmentsX - missingXsegments;
	assert(missingXsegments < segmentsX);
	mVertices.resize((particleXsegments + 0) * (segmentsY + 1));
	mInvMasses.resize((particleXsegments + 0) * (segmentsY + 1));
	mTriangles.resize(triangleXsegments * segmentsY * 2);
	if (createQuads)
		mQuads.resize(triangleXsegments * segmentsY);

	mMesh.vertices.resize(mVertices.size());
	mMesh.indices.resize(3 * mTriangles.size());

	float slopeX;
	float slopeY;
	{
		float y = height;
		float x = radiusBottom - radiusTop;
		float l = sqrtf(x*x + y*y);
		slopeY = x / l;
		slopeX = y / l;
	}

	// Vertices
	for (int y = 0; y < segmentsY + 1; y++)
	{
		float h = height - (float)y / (float)segmentsY * height - 0.5f*height;
		float ynorm = (float)y / (float)(segmentsY - 1);
		float w = ynorm;
		float r = radiusBottom * w + (1.0f - w) * radiusTop;
		for (int x = 0; x < particleXsegments; x++)
		{
			float theta = (float)x / (float)segmentsX * physx::PxTwoPi;
			float rw = r + cosf(frequency*theta)*(ampitudeBottom * w + (1.0f - w) * ampitudeTop);
			mVertices[x + y * particleXsegments] = transform.transform(physx::PxVec3(sinf(theta)*rw, h, cosf(theta)*rw));
			mInvMasses[x + y * particleXsegments] = (y == 0 && attachTop || y == segmentsY && attachBottom) ? 0.0f : 1.0f;

			mMesh.vertices[x + y * particleXsegments].position = mVertices[x + y * particleXsegments];
			mMesh.vertices[x + y * particleXsegments].uv = physx::PxVec2((float)x / (float)particleXsegments, (float)y / (float)segmentsY);
			// Not the correct normal, but we recalculate it anyway when updating the cloth mesh
			mMesh.vertices[x + y * particleXsegments].normal = physx::PxVec3(cosf(theta)*slopeX, slopeY, -sinf(theta)*slopeX);
		}
	}

	if (createQuads)
	{
		// Quads
		for (int y = 0; y < segmentsY; y++)
		{
			for (int x = 0; x < triangleXsegments; x++)
			{
				mQuads[(x + y * triangleXsegments)] = Quad((uint32_t)(x + 0) + (y + 0) * (particleXsegments),
					(uint32_t)((x + 1) % particleXsegments) + (y + 0) * (particleXsegments),
					(uint32_t)((x + 1) % particleXsegments) + (y + 1) * (particleXsegments),
					(uint32_t)((x + 0) % particleXsegments) + (y + 1) * (particleXsegments));
			}
		}
	}

	// Triangles
	for (int y = 0; y < segmentsY; y++)
	{
		for (int x = 0; x < triangleXsegments; x++)
		{
			mTriangles[(x + y * triangleXsegments) * 2 + 0] = Triangle((uint32_t)((x + 1) % particleXsegments) + (y + 1) * (particleXsegments),
															   (uint32_t)((x + 1) % particleXsegments) + (y + 0) * (particleXsegments),
								(uint32_t)(x + 0) + (y + 0) * (particleXsegments));

			mTriangles[(x + y * triangleXsegments) * 2 + 1] = Triangle((uint32_t)((x + 0) % particleXsegments) + (y + 1) * (particleXsegments),
															   (uint32_t)((x + 1) % particleXsegments) + (y + 1) * (particleXsegments),
								(uint32_t)(x + 0) + (y + 0) * (particleXsegments));
		}
	}

	for (int i = 0; i < (int)mTriangles.size(); i++)
	{
		mMesh.indices[3 * i + 0] = mTriangles[i].a;
		mMesh.indices[3 * i + 1] = mTriangles[i].b;
		mMesh.indices[3 * i + 2] = mTriangles[i].c;
	}
}

void ClothMeshData::AttachClothPlaneByAngles(int segmentsX, int segmentsY, bool attachByWidth)
{
	for (int y = 0; y < segmentsY + 1; y++)
		for (int x = 0; x < segmentsX + 1; x++)
			if ((attachByWidth && y == 0) || (!attachByWidth && x == 0))
				if (x == 0 || x == segmentsX)
					mInvMasses[x + y * (segmentsX + 1)] = 0.0f;
}

void ClothMeshData::AttachClothPlaneBySide(int segmentsX, int segmentsY, bool attachByWidth)
{
	for (int y = 0; y < segmentsY + 1; y++)
		for (int x = 0; x < segmentsX + 1; x++)
			if ((attachByWidth && y == 0) || (!attachByWidth && x == 0))
				mInvMasses[x + y * (segmentsX + 1)] = 0.0f;
}

void ClothMeshData::AttachClothUsingTopVertices(float thresholdY)
{
	int topVertexIndex = -1;
	physx::PxVec3 topVertex(-1e30f, -1e30f, -1e30f);

	for (int i = 0; i < (int)mVertices.size(); ++i)
	{
		if (mVertices[i].y > topVertex.y)
		{
			topVertex = mVertices[i];
			topVertexIndex = i;
		}
	}
	NV_CLOTH_ASSERT(topVertexIndex >= 0);

	for (int i = 0; i < (int)mVertices.size(); ++i)
	{
		if (topVertex.y - mVertices[i].y < thresholdY)
		{
			mInvMasses[i] = 0.0f;
		}
	}
}

bool ClothMeshData::ReadClothFromFile(const std::string& verticesPath, const std::string& indicesPath, physx::PxMat44 transform)
{
	std::vector<float> verticesXYZ = readValuesFromFile<float>(verticesPath);
	std::vector<uint32_t> indices = readValuesFromFile<uint32_t>(indicesPath);

	if(verticesXYZ.size() < 3*3 || indices.size() < 3)
		return false;

	return InitializeFromData<float,uint32_t>(ToBoundedData(verticesXYZ), ToBoundedData(indices), transform);
}

template<typename PositionType, typename IndexType>
bool ClothMeshData::InitializeFromData(nv::cloth::BoundedData positions, nv::cloth::BoundedData indices, physx::PxMat44 transform)
{
	if(positions.count < 3 || indices.count < 3)
		return false;

	NV_CLOTH_ASSERT(sizeof(PositionType) != physx::PxVec3 || positions.count % 3 == 0);
	NV_CLOTH_ASSERT(indices.count % 3 == 0);

	auto numVertices = (sizeof(PositionType) == sizeof(physx::PxVec3)) ? positions.count : positions.count / 3;
	const auto numTriangles = indices.count / 3;

	Clear();
	mVertices.resize(numVertices);
	mInvMasses.resize(numVertices);
	mTriangles.resize(numTriangles);

	// Quads not supported yet
	//mQuads.resize(numTriangles / 2);

	mMesh.vertices.resize(mVertices.size());
	mMesh.indices.resize(3 * mTriangles.size());

	for(int i = 0; i < (int)numVertices; ++i)
	{
		physx::PxVec3 pos;
		if(sizeof(PositionType) == sizeof(physx::PxVec3))
			pos = positions.at<physx::PxVec3>(i);
		else
			pos = physx::PxVec3(positions.at<float>(i * 3), positions.at<float>(i * 3 + 1), positions.at<float>(i * 3 + 2));

		pos = transform.transform(pos);

		mVertices[i] = pos;
		mInvMasses[i] = 1.0f;

		mMesh.vertices[i].position = pos;
		mMesh.vertices[i].normal = transform.transform(physx::PxVec3(0.f, 1.f, 0.f)); // TODO
		mMesh.vertices[i].uv = physx::PxVec2(0.0f, 0.0f); // TODO
	}

	for(int i = 0; i < (int)numTriangles; ++i)
	{
		mTriangles[i] = Triangle(
			indices.at<IndexType>(i * 3),
			indices.at<IndexType>(i * 3 + 1),
			indices.at<IndexType>(i * 3 + 2)
		);
	}

	for(int i = 0; i < (int)numTriangles; i++)
	{
		mMesh.indices[3 * i + 0] = mTriangles[i].a;
		mMesh.indices[3 * i + 1] = mTriangles[i].b;
		mMesh.indices[3 * i + 2] = mTriangles[i].c;
	}

	return true;
}
template bool ClothMeshData::InitializeFromData<float,uint16_t>(nv::cloth::BoundedData positions, nv::cloth::BoundedData indices, physx::PxMat44 transform);
template bool ClothMeshData::InitializeFromData<float,uint32_t>(nv::cloth::BoundedData positions, nv::cloth::BoundedData indices, physx::PxMat44 transform);
template bool ClothMeshData::InitializeFromData<physx::PxVec3,uint16_t>(nv::cloth::BoundedData positions, nv::cloth::BoundedData indices, physx::PxMat44 transform);
template bool ClothMeshData::InitializeFromData<physx::PxVec3,uint32_t>(nv::cloth::BoundedData positions, nv::cloth::BoundedData indices, physx::PxMat44 transform);

void ClothMeshData::SetInvMasses(float invMass)
{
	// Doesn't modify attached vertices
	for (int i = 0; i < (int)mInvMasses.size(); ++i)
		if (mInvMasses[i] > 1e-6f)
			mInvMasses[i] = invMass;
}

void ClothMeshData::SetInvMassesFromDensity(float density)
{
	// Tempt code, should take into account triangle's areas
	// Doesn't modify attached vertices
	for (int i = 0; i < (int)mInvMasses.size(); ++i)
		if (mInvMasses[i] > 1e-6f)
			mInvMasses[i] = 1.f / density;
}

nv::cloth::ClothMeshDesc ClothMeshData::GetClothMeshDesc()
{
	nv::cloth::ClothMeshDesc d;
	d.setToDefault();
	d.points = ToBoundedData(mVertices);
	if (mQuads.size() != 0)
		d.quads = ToBoundedData(mQuads);
	if (mTriangles.size() != 0)
		d.triangles = ToBoundedData(mTriangles);
	d.invMasses = ToBoundedData(mInvMasses);

	return d;
}

SimpleMesh ClothMeshData::GetRenderMesh()
{
	return mMesh;
}

void ClothMeshData::Merge(const ClothMeshData& other)
{
	uint32_t firstVertex = (uint32_t)mVertices.size();
	uint32_t firstTriangle = (uint32_t)mTriangles.size();
	uint32_t firstQuad = (uint32_t)mQuads.size();

	mVertices.insert(mVertices.end(), other.mVertices.begin(), other.mVertices.end());
	mUvs.insert(mUvs.end(), other.mUvs.begin(), other.mUvs.end());
	mInvMasses.insert(mInvMasses.end(), other.mInvMasses.begin(), other.mInvMasses.end());

	mMesh.vertices.insert(mMesh.vertices.end(), other.mMesh.vertices.begin(), other.mMesh.vertices.end());

	for(const auto& t : other.mTriangles)
	{
		mTriangles.push_back(t + firstVertex);
	}
	for(const auto& q : other.mQuads)
	{
		mQuads.push_back(q + firstVertex);
		mMesh.indices.push_back(mQuads.back().a);
		mMesh.indices.push_back(mQuads.back().b);
		mMesh.indices.push_back(mQuads.back().c);
	}
}