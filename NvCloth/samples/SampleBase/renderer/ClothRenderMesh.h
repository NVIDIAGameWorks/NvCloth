/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef CLOTH_RENDER_MESH_H
#define CLOTH_RENDER_MESH_H

#include "Renderable.h"

namespace nv
{
namespace cloth
{
	class ClothMeshDesc;
}
}

struct Vertex
{
	PxVec3 position;
	PxVec3 normal;
};

/**
Simple ñloth render mesh
*/
class ClothRenderMesh : public IRenderMesh
{
public:
	ClothRenderMesh(const void* vertices, uint32_t numVertices, uint32_t vertexSize,
					std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, const uint16_t* faces = nullptr, uint32_t numFaces = 0);
	ClothRenderMesh(const nv::cloth::ClothMeshDesc& desc);
	virtual ~ClothRenderMesh();

	void update(const PxVec3* positions, uint32_t numVertices);

	const std::vector<D3D11_INPUT_ELEMENT_DESC>& getInputElementDesc() const { return mInputDesc; }
	void render(ID3D11DeviceContext& context) const;

protected:
	ClothRenderMesh();
	void initialize(const void* vertices, uint32_t numVertices, uint32_t vertexSize, 
					std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, const uint16_t* faces, uint32_t numFaces);

private:
	ID3D11Device* mDevice;

	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mIndexBuffer;

	std::vector<Vertex> mVertices;
	std::vector<uint16_t> mIndices;

	uint32_t      mNumFaces;
	uint32_t      mNumVertices;
	uint32_t      mVertexSize;

	std::vector<D3D11_INPUT_ELEMENT_DESC> mInputDesc;
};


#endif //CLOTH_RENDER_MESH_H