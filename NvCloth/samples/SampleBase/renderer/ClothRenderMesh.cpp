/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/


#include "ClothRenderMesh.h"
#include "Renderer.h"

#include <NvClothExt/ClothFabricCooker.h>
#include "foundation/PxStrideIterator.h"

using namespace nv;
using namespace cloth;


template <typename T>
void gatherIndices(std::vector<uint16_t>& indices,
	const BoundedData& triangles, const BoundedData& quads)
{
	PxStrideIterator<const T> tIt, qIt;

	indices.reserve(triangles.count * 3 + quads.count * 6);

	tIt = PxMakeIterator(reinterpret_cast<const T*>(triangles.data), triangles.stride);
	for (PxU32 i = 0; i < triangles.count; ++i, ++tIt)
	{
		indices.push_back(static_cast<uint16_t>(tIt.ptr()[0]));
		indices.push_back(static_cast<uint16_t>(tIt.ptr()[1]));
		indices.push_back(static_cast<uint16_t>(tIt.ptr()[2]));
	}
	qIt = PxMakeIterator(reinterpret_cast<const T*>(quads.data), quads.stride);
	for (PxU32 i = 0; i < quads.count; ++i, ++qIt)
	{
		indices.push_back(static_cast<uint16_t>(qIt.ptr()[0]));
		indices.push_back(static_cast<uint16_t>(qIt.ptr()[1]));
		indices.push_back(static_cast<uint16_t>(qIt.ptr()[2]));
		indices.push_back(static_cast<uint16_t>(qIt.ptr()[0]));
		indices.push_back(static_cast<uint16_t>(qIt.ptr()[2]));
		indices.push_back(static_cast<uint16_t>(qIt.ptr()[3]));
	}
}

ClothRenderMesh::ClothRenderMesh(const ClothMeshDesc& desc)
{
	uint32_t numVertices = desc.points.count;
	mVertices.resize(numVertices);

	PxStrideIterator<const PxVec3> pIt(reinterpret_cast<const PxVec3*>(desc.points.data), desc.points.stride);
	for (PxU32 i = 0; i < numVertices; ++i)
	{
		mVertices[i].position = *pIt++;
		mVertices[i].normal = PxVec3(0.f);
	}

	// build triangle indices
	if (desc.flags & MeshFlag::e16_BIT_INDICES)
		gatherIndices<PxU16>(mIndices, desc.triangles, desc.quads);
	else
		gatherIndices<PxU32>(mIndices, desc.triangles, desc.quads);

	for (PxU32 i = 0; i < mIndices.size(); i += 3)
	{
		auto p0 = mVertices[mIndices[i]].position;
		auto p1 = mVertices[mIndices[i + 1]].position;
		auto p2 = mVertices[mIndices[i + 2]].position;

		auto normal = ((p2 - p0).cross(p1 - p0)).getNormalized();

		mVertices[mIndices[i]].normal += normal;
		mVertices[mIndices[i + 1]].normal += normal;
		mVertices[mIndices[i + 2]].normal += normal;
	}

	for (PxU32 i = 0; i < numVertices; ++i)
		mVertices[i].normal.normalize();
	
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
	layout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	layout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });

	initialize(mVertices.data(), (uint32_t)mVertices.size(), sizeof(Vertex), layout, mIndices.data(), (uint32_t)mIndices.size() / 3);
}

ClothRenderMesh::ClothRenderMesh()
	: mIndexBuffer(nullptr)
{
}

ClothRenderMesh::ClothRenderMesh(const void* vertices, uint32_t numVertices, uint32_t vertexSize,
	std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, const uint16_t* faces, uint32_t numFaces)
	: mIndexBuffer(nullptr)
{
	initialize(vertices, numVertices, vertexSize, inputDesc, faces, numFaces);
}

void ClothRenderMesh::initialize(const void* vertices, uint32_t numVertices, uint32_t vertexSize,
	std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, const uint16_t* faces, uint32_t numFaces)
{
	mDevice = GetDeviceManager()->GetDevice();

	mInputDesc = inputDesc;
	mNumVertices = numVertices;
	mVertexSize = vertexSize;
	mNumFaces = numFaces;

	// VB
	{
		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertices;

		D3D11_BUFFER_DESC bufferDesc;

		memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = vertexSize * numVertices;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

		V(mDevice->CreateBuffer(&bufferDesc, &vertexBufferData, &mVertexBuffer));
	}

	// IB
	if (faces != nullptr)
	{
		D3D11_SUBRESOURCE_DATA indexBufferData;

		ZeroMemory(&indexBufferData, sizeof(indexBufferData));
		indexBufferData.pSysMem = faces;

		D3D11_BUFFER_DESC bufferDesc;

		memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(uint16_t) * numFaces*3;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

		V(mDevice->CreateBuffer(&bufferDesc, &indexBufferData, &mIndexBuffer));
	}
}

ClothRenderMesh::~ClothRenderMesh()
{
	SAFE_RELEASE(mVertexBuffer);
	SAFE_RELEASE(mIndexBuffer);
}

void ClothRenderMesh::update(const PxVec3* positions, uint32_t numVertices)
{
	PxStrideIterator<const PxVec3> pIt(positions, sizeof(PxVec3));
	Vertex* vertices = mVertices.data();
	const uint16_t* indices = mIndices.data();
	for (PxU32 i = 0; i < numVertices; ++i)
	{
		vertices[i].position = *pIt++;
		vertices[i].normal = PxVec3(0.f);
	}

	const PxU32 numIndices = (PxU32)mIndices.size();
	for (PxU32 i = 0; i < numIndices; i += 3)
	{
		const auto p0 = vertices[indices[i]].position;
		const auto p1 = vertices[indices[i + 1]].position;
		const auto p2 = vertices[indices[i + 2]].position;

		const auto normal = ((p2 - p0).cross(p1 - p0)).getNormalized();

		vertices[indices[i]].normal += normal;
		vertices[indices[i + 1]].normal += normal;
		vertices[indices[i + 2]].normal += normal;
	}

	for (PxU32 i = 0; i < numVertices; ++i)
		vertices[i].normal.normalize();

	ID3D11DeviceContext* context;
	mDevice->GetImmediateContext(&context);

	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		V(context->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mappedResource));
		memcpy(mappedResource.pData, mVertices.data(), sizeof(Vertex) * mVertices.size());

		context->Unmap(mVertexBuffer, 0);
	}
}

void ClothRenderMesh::render(ID3D11DeviceContext& context) const
{
	context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT strides[1] = { mVertexSize };
	UINT offsets[1] = { 0 };
	context.IASetVertexBuffers(0, 1, &mVertexBuffer, strides, offsets);

	context.IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	if (mIndexBuffer)
		context.DrawIndexed(mNumFaces*3, 0, 0);
	else
		context.Draw(mNumVertices, 0);
}