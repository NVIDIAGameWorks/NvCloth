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
#include "NvClothAuthoringLibrary/DebugVisualization.h"
#include <foundation/PxMat44.h>
#include "NvCloth/Cloth.h"
#include "NvCloth/Fabric.h"
#include "NvCloth/Factory.h"
#include <assert.h>
#include <algorithm>

namespace nv
{
namespace cloth
{
namespace debugVisualization
{

void VisualizeDistanceConstraints(Cloth* cloth_, AddLineCallback addLineCallback)
{
	nv::cloth::Cloth& cloth = *cloth_;
	if(cloth.getNumMotionConstraints() == 0)
		return;

	nv::cloth::Factory& factory = cloth.getFactory();

	nv::cloth::MappedRange<physx::PxVec4> particles = cloth.getCurrentParticles();
	std::vector<physx::PxVec4> motionConstraints;
	motionConstraints.reserve(cloth.getNumMotionConstraints() * 2);
	motionConstraints.resize(cloth.getNumMotionConstraints());
	factory.extractMotionConstraints(cloth, nv::cloth::Range<physx::PxVec4>(&motionConstraints[0], &motionConstraints[0] + motionConstraints.size()));
	motionConstraints.resize(cloth.getNumMotionConstraints() * 2);

	nv::cloth::MappedRange<physx::PxVec4> positions = cloth.getCurrentParticles();

	assert(positions.size() == cloth.getNumMotionConstraints());


	//Set to 1 to color code lines based on distance constraint length (as % of max constraint distance in cloth)
	//Set to 0 to color code lines based on how close the particle is to the distance constraint (as % of max distance per constraint)
#define SHOW_DISTANCE_COLOR 0
#if SHOW_DISTANCE_COLOR
	float maxDist = 0.0f;
	for(int i = (int)(motionConstraints.size() >> 1) - 1; i >= 0; i--)
	{
		maxDist = max(maxDist, motionConstraints[i].w);
	}
#endif

	for(int i = ((int)motionConstraints.size() >> 1) - 1; i >= 0; i--)
	{
		float l = motionConstraints[i].w;
		physx::PxVec3 a = motionConstraints[i].getXYZ();
		physx::PxVec3 b = positions[i].getXYZ();
		physx::PxVec3 d = b - a;
		float currentDist = d.magnitude();
		if(d.magnitudeSquared() < 0.00001f)
		{
			d = physx::PxVec3(0.0f, 0.0f, 1.0f);
		}
		else
		{
			d.normalize();
		}

#if SHOW_DISTANCE_COLOR
		physx::PxVec4 color(0.0f, 1.0f-std::max(0.0f, std::min(1.0f, (l / maxDist))), 0.0f, 1.0f);
#else
		physx::PxVec4 color(0.0f, 1.0f-std::max(0.0f,std::min(1.0f, (currentDist / l))), 0.0f, 1.0f);
#endif

		addLineCallback(a, color, b, color);
	}
}

void VisualizeTethers(Cloth* cloth_, AddLineCallback addLineCallback)
{
	nv::cloth::Cloth& cloth = *cloth_;
	nv::cloth::Fabric& fabric = cloth.getFabric();
	if(fabric.getNumTethers() == 0)
		return;

	nv::cloth::Factory& factory = cloth.getFactory();
	nv::cloth::MappedRange<physx::PxVec4> particles = cloth.getCurrentParticles();

	std::vector<float> tetherLengths;
	tetherLengths.resize(fabric.getNumTethers());
	std::vector<uint32_t> anchors;
	anchors.resize(fabric.getNumTethers());

	factory.extractFabricData(fabric, nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(0, 0),
		nv::cloth::Range<uint32_t>(&anchors[0], &anchors[0] + anchors.size()), nv::cloth::Range<float>(&tetherLengths[0], &tetherLengths[0] + tetherLengths.size()), nv::cloth::Range<uint32_t>(0, 0));

	int particleCount = fabric.getNumParticles();

	for(int i = 0; i < (int)anchors.size(); i++)
	{
		float lengthDiff = (particles[anchors[i]].getXYZ() - particles[i%particleCount].getXYZ()).magnitude() - tetherLengths[i];
		physx::PxVec4 color = lengthDiff > 0.0f ? physx::PxVec4(1.0f, 0.0, 0.0, 1.0f) : physx::PxVec4(1.0f, 0.0, 1.0, 1.0f);
		addLineCallback(particles[anchors[i]].getXYZ(), color, particles[i%particleCount].getXYZ(), color);
	}
}

void VisualizeConstraints(Cloth* cloth_, AddLineCallback addLineCallback, int visiblePhaseRangeBegin, int visiblePhaseRangeEnd)
{
	nv::cloth::Cloth& cloth = *cloth_;
	nv::cloth::Fabric& fabric = cloth.getFabric();
	if(fabric.getNumIndices() == 0)
		return;

	nv::cloth::Factory& factory = cloth.getFactory();

	nv::cloth::MappedRange<physx::PxVec4> particles = cloth.getCurrentParticles();

	if(visiblePhaseRangeBegin >= visiblePhaseRangeEnd)
	{
		//then simply render all constraints in one go
		std::vector<uint32_t> indices;
		indices.resize(fabric.getNumIndices());

		factory.extractFabricData(fabric, nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(&indices[0], &indices[0] + indices.size()),
			nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(0, 0));

		physx::PxVec4 color(0.098, 0.098, 0.6, 1.0f);

		for(int i = 1; i < (int)indices.size(); i += 2)
		{
			addLineCallback(particles[indices[i]].getXYZ(), color, particles[indices[i - 1]].getXYZ(), color);
		}
	}
	else
	{
		//otherwise we render individual phases
		std::vector<uint32_t> indices;
		indices.resize(fabric.getNumIndices());
		std::vector<uint32_t> phases;
		phases.resize(fabric.getNumPhases());
		std::vector<uint32_t> sets;
		sets.resize(fabric.getNumSets());

		factory.extractFabricData(fabric, nv::cloth::Range<uint32_t>(&phases[0], &phases[0] + phases.size()), nv::cloth::Range<uint32_t>(&sets[0], &sets[0] + sets.size()), nv::cloth::Range<float>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(&indices[0], &indices[0] + indices.size()),
			nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(0, 0));

		uint32_t* iIt = &indices[0];
		for(int phaseIndex = 0; phaseIndex < (int)fabric.getNumPhases(); phaseIndex++)
		{
			uint32_t* sIt = &sets[phases[phaseIndex]];
			uint32_t* iEnd = &indices[0] + (sIt[0] * 2);
			uint32_t* iStart = iIt;

			if(!(phaseIndex >= visiblePhaseRangeBegin && phaseIndex < visiblePhaseRangeEnd))
			{
				iIt = iEnd;
				continue;
			}

			for(iIt; iIt < iEnd; iIt += 2)
			{
				float c = (float)(iIt - iStart) / (float)(iEnd - iStart);

				physx::PxVec4 colorTable[3]
				{
					physx::PxVec4(1.0f,  0.0f,  0.0f, 1.0f),
					physx::PxVec4(0.0f,  1.0f,  0.0f, 1.0f),
					physx::PxVec4(0.0f,  0.0f,  1.0f, 1.0f)
				};
				
				physx::PxVec4 shiftTable[3]
				{
					physx::PxVec4(0.0f,  1.0f,  0.0f, 0.0f),
					physx::PxVec4(1.0f,  0.0f,  0.0f, 0.0f),
					physx::PxVec4(0.0f,  0.0f,  1.0f, 0.0f)
				};

				physx::PxVec4 color = colorTable[phaseIndex % 3] + shiftTable[phaseIndex % 3] * c;

				addLineCallback(particles[*iIt].getXYZ(), color, particles[*(iIt + 1)].getXYZ(), color);
			}
		}
	}
}

void VisualizeConstraintStiffness(Cloth* cloth_, AddLineCallback addLineCallback)
{
	nv::cloth::Cloth& cloth = *cloth_;
	nv::cloth::Fabric& fabric = cloth.getFabric();
	if(fabric.getNumIndices() == 0)
		return;

	if(!fabric.getNumStiffnessValues())
		return;

	nv::cloth::Factory& factory = cloth.getFactory();

	nv::cloth::MappedRange<physx::PxVec4> particles = cloth.getCurrentParticles();

	std::vector<uint32_t> indices;
	indices.resize(fabric.getNumIndices());
	std::vector<float> stiffness;
	stiffness.resize(fabric.getNumRestvalues());

	factory.extractFabricData(fabric, nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<float>(&stiffness[0], &stiffness[0] + stiffness.size()), nv::cloth::Range<uint32_t>(&indices[0], &indices[0] + indices.size()),
		nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(0, 0));

	for(int i = 1; i < (int)indices.size(); i += 2)
	{
		float c = 1.0f - exp2f(stiffness[i >> 1]);

		physx::PxVec4 color(0.8f * (1.0f - c), c * 0.8f, 0.0f, 1.0f);
		addLineCallback(particles[indices[i - 1]].getXYZ(), color, particles[indices[i]].getXYZ(), color);
	}
}

void VisualizeConstraintError(Cloth* cloth_, AddLineCallback addLineCallback)
{
	nv::cloth::Cloth& cloth = *cloth_;
	nv::cloth::Fabric& fabric = cloth.getFabric();
	if(fabric.getNumRestvalues() == 0) { return; }
	nv::cloth::Factory& factory = cloth.getFactory();

	nv::cloth::MappedRange<physx::PxVec4> particles = cloth.getCurrentParticles();

	std::vector<uint32_t> indices;
	indices.resize(fabric.getNumIndices());
	std::vector<float> restLengths;
	restLengths.resize(fabric.getNumRestvalues());

	factory.extractFabricData(fabric, nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(&restLengths[0], &restLengths[0] + restLengths.size()), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(&indices[0], &indices[0] + indices.size()),
		nv::cloth::Range<uint32_t>(0, 0), nv::cloth::Range<float>(0, 0), nv::cloth::Range<uint32_t>(0, 0));

	for(int i = 0; i < (int)indices.size(); i += 2)
	{
		physx::PxVec4 p0 = particles[indices[i]];
		physx::PxVec4 p1 = particles[indices[i + 1]];
		float restLength = restLengths[i >> 1];
		float length = (p0 - p1).magnitude();
		const float scale = 2.0f;
		float error = (length / restLength * 0.5f - 0.5f) * scale + 0.5f;
		error = std::max(0.0f, std::min(1.0f, error));
		physx::PxVec4 color(error * 0.8f, 1.0f - error*0.8f, 0.0f, 1.0f);
		addLineCallback(p0.getXYZ(), color, p1.getXYZ(), color);
	}
}

void VisualizePositionDelta(Cloth* cloth_, AddLineCallback addLineCallback, float lengthScale)
{
	nv::cloth::Cloth& cloth = *cloth_;

	nv::cloth::MappedRange<physx::PxVec4> particles1 = cloth.getCurrentParticles();
	nv::cloth::MappedRange<physx::PxVec4> particles0 = cloth.getPreviousParticles();

	//scale so that the solver frequency doesn't affect the position delta length assuming 60fps
	float iterationsPerFrame = std::max(1, int(1.0f / 60.0f * cloth.getSolverFrequency() + 0.5f));

	physx::PxVec4 color(0.0f, 1.0f, 1.0f, 1.0f);

	for(int i = 0; i < (int)particles1.size(); i++)
	{
		addLineCallback(particles1[i].getXYZ(), color, particles1[i].getXYZ() + (particles1[i] - particles0[i]).getXYZ()*iterationsPerFrame * lengthScale, color);
	}
}

void VisualizeBoundingBox(Cloth* cloth_, AddLineCallback addLineCallback)
{
	nv::cloth::Cloth& cloth = *cloth_;

	physx::PxVec3 c = cloth.getBoundingBoxCenter();
	physx::PxVec3 d = cloth.getBoundingBoxScale();
	physx::PxVec3 dx = physx::PxVec3(d.x, 0.0f, 0.0f);
	physx::PxVec3 dy = physx::PxVec3(0.0f, d.y, 0.0f);
	physx::PxVec3 dz = physx::PxVec3(0.0f, 0.0f, d.z);

	physx::PxVec4 color(1.0f, 1.0f, 0.0f, 1.0f);

	addLineCallback(c + dy + dz - dx, color, c + dy + dz + dx, color);
	addLineCallback(c + dy - dz - dx, color, c + dy - dz + dx, color);
	addLineCallback(c - dy + dz - dx, color, c - dy + dz + dx, color);
	addLineCallback(c - dy - dz - dx, color, c - dy - dz + dx, color);
	addLineCallback(c + dy + dx - dz, color, c + dy + dx + dz, color);
	addLineCallback(c + dy - dx - dz, color, c + dy - dx + dz, color);
	addLineCallback(c - dy + dx - dz, color, c - dy + dx + dz, color);
	addLineCallback(c - dy - dx - dz, color, c - dy - dx + dz, color);
	addLineCallback(c + dz + dx - dy, color, c + dz + dx + dy, color);
	addLineCallback(c + dz - dx - dy, color, c + dz - dx + dy, color);
	addLineCallback(c - dz + dx - dy, color, c - dz + dx + dy, color);
	addLineCallback(c - dz - dx - dy, color, c - dz - dx + dy, color);

	color = physx::PxVec4(0.467f, 0.467f, 0.0f, 1.0f);

	addLineCallback(c + dy + dz + dx, color, c - dy - dz - dx, color);
	addLineCallback(c + dy + dz - dx, color, c - dy - dz + dx, color);
	addLineCallback(c - dy + dz + dx, color, c + dy - dz - dx, color);
	addLineCallback(c - dy + dz - dx, color, c + dy - dz + dx, color);
}

}
}
}