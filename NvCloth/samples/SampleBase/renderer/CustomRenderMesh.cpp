/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "CustomRenderMesh.h"
#include <assert.h>

CustomRenderMesh::CustomRenderMesh()
	: m_indexBuffer(nullptr)
{
}

CustomRenderMesh::CustomRenderMesh(const void* vertices, uint32_t numVertices, uint32_t vertexSize, 
							std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, const uint16_t* faces, uint32_t numFaces, int flags)
	: m_indexBuffer(nullptr)
{
	initialize(vertices, numVertices, vertexSize, inputDesc, faces, numFaces, flags);
}

void CustomRenderMesh::initialize(const void* vertices, uint32_t numVertices, uint32_t vertexSize, 
							std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, const uint16_t* faces, uint32_t numFaces, int flags)
{
	ID3D11Device* device = GetDeviceManager()->GetDevice();

	m_inputDesc = inputDesc;
	m_numVertices = numVertices;
	m_vertexSize = vertexSize;
	m_numFaces = numFaces;
	m_vertexCapacity = max(1,numVertices);
	m_indexCapacity = max(1,numFaces);

	// VB
	{
		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertices;

		void* backupBuffer = nullptr;
		if(vertices == nullptr)
		{
			void* backupBuffer = malloc(vertexSize * m_vertexCapacity);
			vertexBufferData.pSysMem = backupBuffer;
		}

		D3D11_BUFFER_DESC bufferDesc;

		memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = vertexSize * m_vertexCapacity;
		bufferDesc.CPUAccessFlags = (flags&DYNAMIC_VERTEX_BUFFER) ? D3D11_CPU_ACCESS_WRITE : 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = (flags&DYNAMIC_VERTEX_BUFFER) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;

		V(device->CreateBuffer(&bufferDesc, &vertexBufferData, &m_vertexBuffer));

		if(vertices = nullptr)
			free(backupBuffer);
	}

	// IB
	if (faces != nullptr)
	{
		D3D11_SUBRESOURCE_DATA indexBufferData;

		ZeroMemory(&indexBufferData, sizeof(indexBufferData));
		indexBufferData.pSysMem = faces;

		void* backupBuffer = nullptr;
		if(faces == nullptr)
		{
			void* backupBuffer = malloc(sizeof(uint16_t) * m_indexCapacity);
			indexBufferData.pSysMem = backupBuffer;
		}

		D3D11_BUFFER_DESC bufferDesc;

		memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(uint16_t) * m_indexCapacity;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

		V(device->CreateBuffer(&bufferDesc, &indexBufferData, &m_indexBuffer));

		if(faces = nullptr)
			free(backupBuffer);
	}
}

CustomRenderMesh::~CustomRenderMesh()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
}

void CustomRenderMesh::updateVertices(const void* vertices, uint32_t numVertices, uint32_t vertexSize)
{
	assert(numVertices <= m_vertexCapacity);
	assert(vertexSize == m_vertexSize);
	ID3D11Device* device = GetDeviceManager()->GetDevice();
	ID3D11DeviceContext* context;
	device->GetImmediateContext(&context);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	context->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//	Update the vertex buffer here.
	memcpy(mappedResource.pData, vertices, vertexSize*numVertices);
	//	Reenable GPU access to the vertex buffer data.
	context->Unmap(m_vertexBuffer, 0);
}


void CustomRenderMesh::render(ID3D11DeviceContext& context, int submesh) const
{
	context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT strides[1] = { m_vertexSize };
	UINT offsets[1] = { 0 };
	context.IASetVertexBuffers(0, 1, &m_vertexBuffer, strides, offsets);

	context.IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	if (m_indexBuffer)
		context.DrawIndexed(m_numFaces, 0, 0);
	else
		context.Draw(m_numVertices, 0);
}

