/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef MESH_GENERATOR_H
#define MESH_GENERATOR_H

#include <vector>
#include "renderer/CustomRenderMesh.h"
#include <foundation/PxVec3.h>

namespace MeshGenerator
{

struct Point
{
	Point(){}
	Point(physx::PxVec3 _p):p(_p){}
	physx::PxVec3 p;

	Point operator*(float f) const { return Point(p*f); }
	Point operator+(Point pt) const { return Point(p+pt.p); }
};

struct RenderVertex
{
	RenderVertex() {}
	RenderVertex(physx::PxVec3 _p, physx::PxVec3 _n):p(_p),n(_n) {}
	physx::PxVec3 p;
	physx::PxVec3 n;
};

struct Polygon
{
	Polygon() {}
	template<typename P, typename... Args> Polygon(P p1, P p2, P p3, Args... args)
	{
		addPoints(p1, p2, p3, args...);
	}

	std::vector<Point> mPoints;

	bool isTriangle()const { return mPoints.size() == 3; }

	template<typename P, typename... Args> void addPoints(P p, Args... args)
	{
		mPoints.push_back(p);
		addPoints(args...);
	}
	template<typename P> void addPoints(P p)
	{
		mPoints.push_back(p);
	}

	void triangulate(std::vector<Polygon>& out) const;
	void triangulate(std::vector<RenderVertex>& verts, std::vector<uint16_t>& indices) const;
	void triangulateWeld(std::vector<RenderVertex>& verts, std::vector<uint16_t>& indices) const; //normalize normals afterwards
	void triangulateForCollision(std::vector<physx::PxVec3>& verts) const;
	physx::PxVec3 calculateNormal() const;
	float calculateArea() const;
	void subdivideTriangle(std::vector<Polygon>& out) const;
	bool Polygon::pointPlaneSide(physx::PxVec3 p, physx::PxVec4 plane) const;
	void clip(physx::PxVec4 plane, bool flip = false);
};

struct Mesh
{
	std::vector<Polygon> mPolygons;

	bool isTriangleMesh()const { bool b = true;  for(const auto& p : mPolygons) { b = b && p.isTriangle(); } return b; }

	void addConvexPolygon(physx::PxVec4 plane, physx::PxVec4* planes, uint32_t mask, bool flip);

	void generateRenderBuffers(RenderVertex** vertices, uint16_t** indices, int* vertexCount, int* indexCount) const;
	void generateSmoothRenderBuffers(RenderVertex** vertices, uint16_t** indices, int* vertexCount, int* indexCount) const;
	int generateTriangleList(physx::PxVec3** positions);

	void applyTransfom(physx::PxMat44 transform);

	void merge(const Mesh& mesh);
};

Mesh generateTetrahedron(float radius);
Mesh generateIcosahedron(float radius, int subdivisions);
Mesh generateCone(physx::PxVec4 a, physx::PxVec4 b, int segments, float grow, bool correctCone);
Mesh generateCollisionConvex(physx::PxVec4* planes, uint32_t mask, float grow, bool flip);
Mesh generateCollisionCapsules(physx::PxVec4* spheres, int sphereCount, uint32_t* indices, int indexCount, float grow);

uint32_t generateConvexPolyhedronPlanes(int segmentsX, int segmentsY, physx::PxVec3 center, float radius, std::vector<physx::PxVec4>* planes);

class MeshGeneratorRenderMesh : public CustomRenderMesh
{
public:
	MeshGeneratorRenderMesh(const Mesh mesh);
	virtual ~MeshGeneratorRenderMesh();
};

class MeshGeneratorRenderMeshSmooth : public CustomRenderMesh
{
public:
	MeshGeneratorRenderMeshSmooth(const Mesh mesh);
	virtual ~MeshGeneratorRenderMeshSmooth();
};


};

#endif