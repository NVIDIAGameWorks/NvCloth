/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "RenderMaterial.h"
#include <DirectXMath.h>
#include "PxMat44.h"
#include "PxVec3.h"
#include "PxVec4.h"

using namespace physx;

class Renderer;

/**
RenderMesh interface, used by Renderable
*/
class IRenderMesh
{
public:
	IRenderMesh(){ m_submeshCount = 1; }
	virtual ~IRenderMesh() {}
	virtual const std::vector<D3D11_INPUT_ELEMENT_DESC>& getInputElementDesc() const = 0;
	virtual void render(ID3D11DeviceContext& context, int submesh) const = 0;
	virtual physx::PxMat44 getRenderSubmeshTransform(int submesh) { return physx::PxMat44(physx::PxIdentity); }
	virtual bool isRenderSubmeshHidden(int submesh) { return false; }

	virtual int getBoneCount() const { return 0; }
	int getSubMeshCount() const { return m_submeshCount; }
protected:
	int m_submeshCount;
};

/**
Renderable, represents single object renderer by Renderer.
Basically Renderable = RenderMaterial(s) + RenderMesh(es)
*/
class Renderable
{
public:
	//////// public API ////////

	void setMaterial(RenderMaterial& material);
	void clearMaterials() { m_materialInstances.clear(); }
	void addMaterial(RenderMaterial& material);
	RenderMaterial& getMaterial(int id) const { return m_materialInstances[id]->getMaterial(); }
	int getMaterialCount() const { return (int)m_materialInstances.size(); }

	PxMat44 getModelMatrix() const
	{
		return PxMat44(m_transform) * PxMat44(PxVec4(m_scale, 1));
	}

	void setTransform(PxTransform& transform)
	{
		m_transform = transform;
	}

	const PxTransform& getTransform() const
	{
		return m_transform;
	}

	void setScale(PxVec3 scale)
	{
		m_scale = scale;
	}

	const PxVec3& getScale() const
	{
		return m_scale;
	}

	void setColor(DirectX::XMFLOAT4 color)
	{
		m_colors.clear();
		m_colors.push_back(color);
	}
	void addColor(DirectX::XMFLOAT4 color)
	{
		m_colors.push_back(color);
	}
	DirectX::XMFLOAT4& getColor(int id) { return m_colors[id]; }
	DirectX::XMFLOAT4 const& getColor(int id) const { return m_colors[id]; }
	int getColorCount() const { return (int)m_colors.size(); }
	int getBoneCount() const { return m_mesh.getBoneCount(); }

	void setHidden(bool hidden)
	{
		m_hidden = hidden;
	}

	bool isHidden() const 
	{ 
		return m_hidden;
	}

	bool isTransparent() const
	{
		for(int i = 0; i < (int)m_materialInstances.size(); i++)
		{
			if(!(m_materialInstances[i]->getMaterial().getBlending() == RenderMaterial::BLEND_NONE))
				return true;
		}
		return false;
	}

private:
	//////// methods used by Renderer ////////

	friend class Renderer;

	void render(Renderer& renderer) const
	{
		render(renderer, false);
	}

	void renderDepthStencilOnly(Renderer& renderer) const
	{
		render(renderer, true);
	}

	Renderable(IRenderMesh& mesh, RenderMaterial& material);

	void render(Renderer& renderer, bool depthStencilOnly) const;


	//////// internal data ////////

	std::vector<DirectX::XMFLOAT4> m_colors;
	PxTransform                 m_transform;
	PxVec3                      m_scale;

	std::vector<RenderMaterial::InstancePtr> m_materialInstances;
	IRenderMesh&                m_mesh;
	bool                        m_hidden;
};

#endif //RENDERABLE_H