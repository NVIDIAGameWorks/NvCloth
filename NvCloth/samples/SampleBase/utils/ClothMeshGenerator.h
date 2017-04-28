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
#include <foundation/PxVec3.h>
#include <foundation/PxVec2.h>
#include "NvClothExt/ClothFabricCooker.h"
#include <foundation/PxMat44.h>
#include "Mesh.h"

struct ClothMeshData
{
	struct Triangle
	{
		Triangle(){}
		Triangle(uint32_t _a, uint32_t _b, uint32_t _c) :
			a(_a), b(_b), c(_c){}
		uint32_t a, b, c;

		Triangle operator+(uint32_t offset)const { return Triangle(a + offset, b + offset, c + offset); };
	};
	struct Quad
	{
		Quad(){}
		Quad(uint32_t _a, uint32_t _b, uint32_t _c, uint32_t _d) :
			a(_a), b(_b), c(_c), d(_d){}
		uint32_t a, b, c, d;

		Quad operator+(uint32_t offset)const { return Quad(a + offset, b + offset, c + offset, d + offset); };
	};
	std::vector<physx::PxVec3>	mVertices;
	std::vector<physx::PxVec2>	mUvs;
	std::vector<Triangle>		mTriangles;
	std::vector<Quad>			mQuads;
	std::vector<physx::PxReal>	mInvMasses;

	SimpleMesh					mMesh;

	void Clear();
	void GeneratePlaneCloth(float width, float height, int segmentsX, int segmentsY, bool createQuads = false, physx::PxMat44 transform = physx::PxIdentity, bool alternatingDiagonals = true, int zigzag = 0);

	void AttachClothPlaneByAngles(int segmentsX, int segmentsY, bool  attachByWidth = true);
	void AttachClothPlaneBySide(int segmentsX, int segmentsY, bool  attachByWidth = true);

	void SetInvMasses(float invMass);
	void SetInvMassesFromDensity(float density); 	// Todo

	nv::cloth::ClothMeshDesc	GetClothMeshDesc();
	SimpleMesh					GetRenderMesh();

	void Merge(const ClothMeshData& other);
};
