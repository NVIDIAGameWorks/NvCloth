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
#include "NvClothAuthoringLibrary/Parameters.h"

namespace nv
{
namespace cloth
{
namespace authoring
{

Parameter InitParameter(ParameterType type, int catagoryId, const char* name, const char* description, void* set, void* get)
{
	Parameter p;
	static int nextId = 0;
	p.mCatagoryId = catagoryId;
	p.mParameterId = nextId++;
	p.mName = name;
	p.mDescription = description;
	p.mType = type;
	p.SetFunctionPointer = set;
	p.GetFunctionPointer = get;
	p.mHardMinLimit = false;
	p.mHardMaxLimit = false;
	return p;
}

Parameter InitParameter(int catagoryId, const char* name, const char* description, SetFloatParamType set, GetFloatParamType get, float minV, float maxV, bool hardMin = false, bool hardMax = false)
{
	Parameter p = InitParameter(ParameterType::FLOAT, catagoryId, name, description, set, get);
	p.mMinFloat = minV;
	p.mMaxFloat = maxV;
	p.mHardMinLimit = hardMin;
	p.mHardMaxLimit = hardMax;
	return p;
}

Parameter InitParameter(int catagoryId, const char* name, const char* description, SetVec3ParamType set, GetVec3ParamType get, physx::PxVec3 minV, physx::PxVec3 maxV, ParameterType parameterType = ParameterType::VEC3, bool hardMin = false, bool hardMax = false)
{
	Parameter p = InitParameter(parameterType, catagoryId, name, description, set, get);
	p.mMinVec3 = minV;
	p.mMaxVec3 = maxV;
	p.mHardMaxLimit = hardMax;
	p.mHardMinLimit = hardMin;
	return p;
}

static const char* sParameterCatagories[] =
{
	/* 0  */ "Basics",
	/* 1  */ "Air simulation",
	/* 2  */ "Local space simulation",
	/* 3  */ "Tethers",
	/* 4  */ "Collision",
	/* 5  */ "Misc",
	/* 6  */ "Airmeshes"
	/* 7  */
	/* 8  */
};

static Parameter sParameterList[] =
{
	InitParameter(0, "Damping",			"",
					[](Cloth* cloth, physx::PxVec3 value) {cloth->setDamping(value); },
					[](Cloth* cloth) {return cloth->getDamping(); },
					physx::PxVec3(0.0f), physx::PxVec3(1.0f),
					ParameterType::VEC3_SLIDER1MAX, true, true
					),
	InitParameter(0, "Gravity",			"",
					[](Cloth* cloth, physx::PxVec3 value) {cloth->setGravity(value); },
					[](Cloth* cloth) {return cloth->getGravity(); },
					physx::PxVec3(-50.0f,-50.0f,-50.0f), physx::PxVec3(50.0f,50.0f,50.0f)
					),
	InitParameter(0, "Solver frequency",			"Number of solver iterations per second",
					[](Cloth* cloth, float value) {cloth->setSolverFrequency(value); },
					[](Cloth* cloth) {return cloth->getSolverFrequency(); },
					0.0f, 600.0f, true, false
					),

	InitParameter(1, "Drag coefficient",			"For the advanced wind model (non zero enables this feature which is expensive)",
					[](Cloth* cloth, float value) {cloth->setDragCoefficient(value); },
					[](Cloth* cloth) {return cloth->getDragCoefficient(); },
					0.0f, 1.0f, true, true
					),
	InitParameter(1, "Lift coefficient",			"For the advanced wind model (non zero enables this feature which is expensive)",
					[](Cloth* cloth, float value) {cloth->setLiftCoefficient(value); },
					[](Cloth* cloth) {return cloth->getLiftCoefficient(); },
					0.0f, 1.0f, true, true
					),
	InitParameter(1, "Fluid density",			"For the advanced wind model, can be used to",
					[](Cloth* cloth, float value) {cloth->setLiftCoefficient(value); },
					[](Cloth* cloth) {return cloth->getLiftCoefficient(); },
					0.0f, 10.0f, true, false
					),

	InitParameter(2, "Linear inertia",			"Amount of linear force from moving the local space to be applied",
					[](Cloth* cloth, physx::PxVec3 value) {cloth->setLinearInertia(value); },
					[](Cloth* cloth) {return cloth->getLinearInertia(); },
					physx::PxVec3(0.0f), physx::PxVec3(1.0f),
					ParameterType::VEC3_SLIDER1MAX
					),
	InitParameter(2, "Angular inertia",			"Amount of angular force from moving the local space to be applied",
					[](Cloth* cloth, physx::PxVec3 value) {cloth->setAngularInertia(value); },
					[](Cloth* cloth) {return cloth->getAngularInertia(); },
					physx::PxVec3(0.0f), physx::PxVec3(1.0f),
					ParameterType::VEC3_SLIDER1MAX
					),
	InitParameter(2, "Centrifugal inertia",			"Amount of centrifugal force from moving the local space to be applied",
					[](Cloth* cloth, physx::PxVec3 value) {cloth->setCentrifugalInertia(value); },
					[](Cloth* cloth) {return cloth->getCentrifugalInertia(); },
					physx::PxVec3(0.0f), physx::PxVec3(1.0f),
					ParameterType::VEC3_SLIDER1MAX
					),
	InitParameter(2, "Linear drag",			"Amount of drag form moving the local space to be applied",
					[](Cloth* cloth, physx::PxVec3 value) {cloth->setLinearDrag(value); },
					[](Cloth* cloth) {return cloth->getLinearDrag(); },
					physx::PxVec3(0.0f), physx::PxVec3(1.0f),
					ParameterType::VEC3_SLIDER1MAX, true, true
					),
	InitParameter(2, "Angular drag",			"Amount of drag form moving the local space to be applied",
					[](Cloth* cloth, physx::PxVec3 value) {cloth->setAngularDrag(value); },
					[](Cloth* cloth) {return cloth->getAngularDrag(); },
					physx::PxVec3(0.0f), physx::PxVec3(1.0f),
					ParameterType::VEC3_SLIDER1MAX, true, true
					),

	InitParameter(3, "Scale",			"Scale factor for tether length",
					[](Cloth* cloth, float value) {cloth->setTetherConstraintScale(value); },
					[](Cloth* cloth) {return cloth->getTetherConstraintScale(); },
					0.0f, 4.0f, true, false
					),
	InitParameter(3, "Stiffness",			"",
					[](Cloth* cloth, float value) {cloth->setTetherConstraintStiffness(value); },
					[](Cloth* cloth) {return cloth->getTetherConstraintStiffness(); },
					0.0f, 1.0f, true, true
					),

	InitParameter(4, "Friction",			"Friction on collision contacts with collision shapes",
					[](Cloth* cloth, float value) {cloth->setFriction(value); },
					[](Cloth* cloth) {return cloth->getFriction(); },
					0.0f, 2.0f
					),

	InitParameter(4, "Motion constraint scale",			"Scale factor for the length of the motion/distance constraints",
					[](Cloth* cloth, float value) {cloth->setMotionConstraintScaleBias(value,cloth->getMotionConstraintBias()); },
					[](Cloth* cloth) {return cloth->getMotionConstraintScale(); },
					0.0f, 4.0f, true, false
					),
	InitParameter(4, "Motion constraint bias",			"Additional length for the length of the motion/distance constraints",
					[](Cloth* cloth, float value) {cloth->setMotionConstraintScaleBias(cloth->getMotionConstraintScale(), value); },
					[](Cloth* cloth) {return cloth->getMotionConstraintBias(); },
					0.0f, 8.0f, false, false
					),
	InitParameter(4, "Motion constraint stiffness",			"How hard the constraint is enforced",
					[](Cloth* cloth, float value) {cloth->setMotionConstraintStiffness(value); },
					[](Cloth* cloth) {return cloth->getMotionConstraintStiffness(); },
					0.0f, 1.0f, true, true
					),

	InitParameter(4, "Self collision radius",			"Min distance between two particles within this cloth (expensive when set > 0)",
					[](Cloth* cloth, float value) {cloth->setSelfCollisionDistance(value); },
					[](Cloth* cloth) {return cloth->getSelfCollisionDistance(); },
					0.0f, 1.0f, true, false
					),
	InitParameter(4, "Self collision stiffness",			"How hard the constraint is enforced",
					[](Cloth* cloth, float value) {cloth->setSelfCollisionStiffness(value); },
					[](Cloth* cloth) {return cloth->getSelfCollisionStiffness(); },
					0.0f, 1.0f, true, true
					),

	InitParameter(5, "Sleep threshold",			"Maximum movement allowed for cloth to go to sleep",
					[](Cloth* cloth, float value) {cloth->setSleepThreshold(value); },
					[](Cloth* cloth) {return cloth->getSleepThreshold(); },
					0.0f, 1.0f
					),
	InitParameter(5, "Stiffness frequency",			"",
					[](Cloth* cloth, float value) {cloth->setStiffnessFrequency(value); },
					[](Cloth* cloth) {return cloth->getStiffnessFrequency(); },
					0.0f, 100.0f, true, false
					),

/*
	InitParameter(6, "Rest volume scale",			"",
					[](Cloth* cloth, float value) {cloth->setRestVolumesScale(value); },
					[](Cloth* cloth) {return cloth->getRestVolumesScale(); },
					0.0f, 10.0f
					),
	InitParameter(6, "Compression stiffness",			"",
					[](Cloth* cloth, float value) {cloth->setCompressionStiffness(value); },
					[](Cloth* cloth) {return cloth->getCompressionStiffness(); },
					0.0f, 1.0f
					),
	InitParameter(6, "Expansion stiffness",			"",
					[](Cloth* cloth, float value) {cloth->setExpansionStiffness(value); },
					[](Cloth* cloth) {return cloth->getExpansionStiffness(); },
					0.0f, 1.0f
					),
*/
};

int GetClothNumParameters()
{
	return (sizeof(sParameterList) / sizeof(Parameter));
}
Parameter const& GetClothParameterInfo(int id)
{
	return sParameterList[id];
}

int GetClothNumParameterCatagories()
{
	return (sizeof(sParameterCatagories) / sizeof(const char*));
}
const char* GetClothParameterCatagorieInfo(int id)
{
	return sParameterCatagories[id];
}

}
}
}