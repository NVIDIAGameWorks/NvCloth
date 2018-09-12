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
#include "NvCloth/Cloth.h"

namespace nv
{
namespace cloth
{
namespace authoring
{

enum class ParameterType
{
	NONE,
	FLOAT,
	VEC3,
	VEC3_NORMALIZED,
	VEC3_SLIDER1MAX,
};

//Parameter set callback types
typedef void(*SetFloatParamType)(Cloth* cloth, float value);
typedef void(*SetVec3ParamType)(Cloth* cloth, physx::PxVec3 value);

//Parameter get callback types
typedef float(*GetFloatParamType)(Cloth* cloth);
typedef physx::PxVec3(*GetVec3ParamType)(Cloth* cloth);

struct Parameter
{
	Parameter() {}
	Parameter(Parameter const & other)
	{
		memcpy(this, &other, sizeof(Parameter));
	}
	Parameter& operator=(Parameter const & other)
	{
		memcpy(this, &other, sizeof(Parameter));
	}

	int mParameterId;
	int mCatagoryId;
	const char* mName;
	const char* mDescription;
	ParameterType mType;

	//Setters
	union
	{
		void* SetFunctionPointer;
		SetFloatParamType SetFloat;
		SetVec3ParamType SetVec3;
	};

	//Getters
	union
	{
		void* GetFunctionPointer;
		GetFloatParamType GetFloat;
		GetVec3ParamType GetVec3;
	};

	union
	{
		float mMinFloat;
		physx::PxVec3 mMinVec3;
	};

	union
	{
		float mMaxFloat;
		physx::PxVec3 mMaxVec3;
	};
	bool mHardMinLimit; //true if the mMax* limit should be respected, false if they are just guide lines
	bool mHardMaxLimit; //true if the mMax* limit should be respected, false if they are just guide lines
};

int GetClothNumParameters();
Parameter const& GetClothParameterInfo(int id);
int GetClothNumParameterCatagories();
const char* GetClothParameterCatagorieInfo(int id);


}
}
}