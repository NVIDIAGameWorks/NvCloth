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
#include "DebugRenderBuffer.h"
#include <foundation/PxVec3.h>
#include <foundation/PxMat44.h>

class DebugLineRenderBuffer : public DebugRenderBuffer
{
public:
	void clear() { m_lines.clear(); }
	void addLine(physx::PxVec3 a, physx::PxVec3 b, unsigned int color);
	void addVector(physx::PxVec3 start, physx::PxVec3 vec, unsigned int color) { addLine(start, start + vec, color); }

	void addLine(physx::PxMat44 t, physx::PxVec3 a, physx::PxVec3 b, unsigned int color);
	void addVector(physx::PxMat44 t, physx::PxVec3 start, physx::PxVec3 vec, unsigned int color) { addLine(t, start, start + vec, color); }
};