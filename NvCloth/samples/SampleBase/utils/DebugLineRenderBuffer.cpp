/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "DebugLineRenderBuffer.h"

void DebugLineRenderBuffer::addLine(physx::PxVec3 a, physx::PxVec3 b, unsigned int color)
{
	m_lines.push_back(PxDebugLine(a, b, color));
}

void DebugLineRenderBuffer::addLine(physx::PxMat44 t, physx::PxVec3 a, physx::PxVec3 b, unsigned int color)
{
	m_lines.push_back(PxDebugLine(t.transform(a), t.transform(b), color));
}