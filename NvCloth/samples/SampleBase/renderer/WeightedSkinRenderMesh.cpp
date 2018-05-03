/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/


#include "WeightedSkinRenderMesh.h"
#include "Renderer.h"
#include "Model.h"

WeightedSkinRenderMesh::WeightedSkinRenderMesh(const Model* model)
{
	PX_ASSERT_WITH_MESSAGE(meshes.size() <= MeshesCountMax, "meshes.size() have to be <= WeightedSkinRenderMesh::MeshesCountMax");

	m_model = model;
	m_modelInstance.mAnimationIndex = 0;
	m_modelInstance.mAnimationTime = 0.0f;

	m_device = GetDeviceManager()->GetDevice();

	// input element desc setup
	m_inputDesc.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	m_inputDesc.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	m_inputDesc.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }); //uv
	m_inputDesc.push_back({ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 }); //bone Indices
	m_inputDesc.push_back({ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0});  //bone weights

	ID3D11DeviceContext* context;
	m_device->GetImmediateContext(&context);

	// reserve VB
	uint32_t verticesTotal = model->getTotalVertexCount();
	std::vector<SkinnedMesh::Vertex> vertexBuffer;
	vertexBuffer.reserve(verticesTotal);

	// reserve IB 
	uint32_t indicesTotal = model->getTotalIndexCount();
	m_indices.reserve(indicesTotal);

	// fill VB, IB, MeshInfo
	m_meshesInfo.resize(model->getSubMeshCount());
	m_submeshCount = model->getTotalSubmeshInstanceCount();
	for (int meshIndex = 0; meshIndex < m_submeshCount; ++meshIndex)
	{
		const SkinnedMesh* mesh = &model->getSubMesh(meshIndex);
		MeshInfo& meshInfo = m_meshesInfo[meshIndex];

		meshInfo.firstVertex = (uint32_t)vertexBuffer.size();
		vertexBuffer.insert(vertexBuffer.end(), mesh->vertices.begin(), mesh->vertices.end());
		meshInfo.verticesCount = (uint32_t)mesh->vertices.size();

		meshInfo.firstIndex = (uint32_t)m_indices.size();
		uint32_t indexOffset = meshInfo.firstVertex;
		for (uint32_t index : mesh->indices)
		{
			m_indices.push_back((uint32_t)index);
		}
		meshInfo.indicesCount = (uint32_t)mesh->indices.size();
	}

	// vertex buffer
	{
		D3D11_SUBRESOURCE_DATA vertexBufferData;

		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = vertexBuffer.data();

		D3D11_BUFFER_DESC bufferDesc;

		memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = (uint32_t)(sizeof(SkinnedMesh::Vertex) * vertexBuffer.size());
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

		V(m_device->CreateBuffer(&bufferDesc, &vertexBufferData, &m_vertexBuffer));
	}

	// index buffer
	{
		D3D11_BUFFER_DESC bufferDesc;

		memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.ByteWidth = (uint32_t)(sizeof(uint32_t) * m_indices.size());
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

		V(m_device->CreateBuffer(&bufferDesc, nullptr, &m_indexBuffer));
	}

	// bone texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = 4;
		desc.Height = (uint32_t)model->getNodeCount()*(1+model->getSubMeshCount()); //transforms and bind pose transforms
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		V(m_device->CreateTexture2D(&desc, nullptr, &m_boneTexture));
	}

	// bone texture SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = 1;
		desc.Texture2D.MostDetailedMip = 0;
		V(m_device->CreateShaderResourceView(m_boneTexture, &desc, &m_boneTextureSRV));
	}

	// update index buffer
	{
		D3D11_MAPPED_SUBRESOURCE mappedRead;
		V(context->Map(m_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mappedRead));

		uint32_t* indexBuffer = (uint32_t*)mappedRead.pData;
		uint32_t indexCount = 0;
		for(const MeshInfo& info : m_meshesInfo)
		{
			memcpy(indexBuffer + indexCount, &m_indices[info.firstIndex], info.indicesCount * sizeof(uint32_t));
			indexCount += info.indicesCount;
		}
		context->Unmap(m_indexBuffer, 0);
		m_indexCount = indexCount;
		PX_ASSERT(m_indexCount % 3 == 0);
	}
}

WeightedSkinRenderMesh::~WeightedSkinRenderMesh()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_boneTexture);
	SAFE_RELEASE(m_boneTextureSRV);
}

void WeightedSkinRenderMesh::updateVisibleMeshTransforms(float deltatime, physx::PxMat44 transform)
{
	m_modelInstance.mAnimationTime += deltatime;
	m_model->updateModelInstance(m_modelInstance, transform);
	

	ID3D11DeviceContext* context;
	m_device->GetImmediateContext(&context);

	// update bone transform texture
	{
		D3D11_MAPPED_SUBRESOURCE mappedRead;
		V(context->Map(m_boneTexture, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mappedRead));
		for (uint32_t i = 0; i < m_modelInstance.mNodes.size(); ++i)
		{
			std::memcpy((uint8_t*)mappedRead.pData + i * mappedRead.RowPitch, &m_modelInstance.mNodes[i].mTransform, sizeof(PxMat44));
		}

		//probably should separate this in to a different texture as it doesn't change
		for(int submeshId = 0; submeshId < m_model->getSubMeshCount(); submeshId++)
		{
			auto& submesh = m_model->getSubMesh(submeshId);
			int offset = (1 + submeshId) * (int)m_modelInstance.mNodes.size();
			for(int i = 0; i < m_model->getNodeCount(); ++i)
			{
				std::memcpy((uint8_t*)mappedRead.pData + (i+offset) * mappedRead.RowPitch, &submesh.mBoneOffsets[i], sizeof(PxMat44));
			}
		}
		context->Unmap(m_boneTexture, 0);
	}
}

int WeightedSkinRenderMesh::getBoneCount() const
{
	return (int)m_modelInstance.mNodes.size();
}

void WeightedSkinRenderMesh::render(ID3D11DeviceContext& context, int submesh) const
{
	Model::SubmeshInstance submeshInstance = m_model->getSubMeshInstance(submesh);

	context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT strides[1] = { sizeof(SkinnedMesh::Vertex)};
	UINT offsets[1] = { 0 };
	ID3D11Buffer* buffers[1] = { m_vertexBuffer };
	context.IASetVertexBuffers(0, 1, buffers, strides, offsets);

	context.IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context.VSSetShaderResources(1, 1, &m_boneTextureSRV);

	context.DrawIndexed(m_meshesInfo[submeshInstance.mSubmeshId].indicesCount, m_meshesInfo[submeshInstance.mSubmeshId].firstIndex, m_meshesInfo[submeshInstance.mSubmeshId].firstVertex);
}

physx::PxMat44 WeightedSkinRenderMesh::getRenderSubmeshTransform(int submesh)
{
	Model::SubmeshInstance submeshInstance = m_model->getSubMeshInstance(submesh);
	return m_modelInstance.mNodes[submeshInstance.mParrentNodeId].mTransform;
}

bool WeightedSkinRenderMesh::isRenderSubmeshHidden(int submesh)
{
	Model::SubmeshInstance submeshInstance = m_model->getSubMeshInstance(submesh);
	return m_modelInstance.mNodes[submeshInstance.mParrentNodeId].mHidden;
}