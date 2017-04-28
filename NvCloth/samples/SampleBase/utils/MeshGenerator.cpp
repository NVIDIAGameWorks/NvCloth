

#include "./MeshGenerator.h"
#include <foundation/PxVec2.h>
#include <utility>
#include "utils/Utils.h"

namespace MeshGenerator
{

void Polygon::triangulate(std::vector<Polygon>& out) const
{
	for(int i = 2; i < (int)mPoints.size(); i++)
	{
		out.push_back(Polygon(mPoints[0], mPoints[i - 1], mPoints[i]));
	}
}

void Polygon::triangulate(std::vector<RenderVertex>& verts, std::vector<uint16_t>& indices) const
{
	physx::PxVec3 normal = calculateNormal();
	for(int i = 2; i < (int)mPoints.size(); i++)
	{
		indices.push_back((uint16_t)verts.size());
		verts.push_back(RenderVertex(mPoints[0].p,normal));
		indices.push_back((uint16_t)verts.size());
		verts.push_back(RenderVertex(mPoints[i - 1].p, normal));
		indices.push_back((uint16_t)verts.size());
		verts.push_back(RenderVertex(mPoints[i].p, normal));
	}
}

void Polygon::triangulateWeld(std::vector<RenderVertex>& verts, std::vector<uint16_t>& indices) const
{
	auto addVertex = [&verts, &indices](RenderVertex v)
	{
		for(int i = 0; i < (int)verts.size(); i++)
		{
			if((verts[i].p - v.p).magnitudeSquared() < 0.001f)
			{
				return i;
			}
		}
		verts.push_back(v);
		return (uint16_t)verts.size()-1;
	};

	physx::PxVec3 weightedNormal = calculateNormal()*calculateArea();

	for(int i = 2; i < (int)mPoints.size(); i++)
	{
		indices.push_back(addVertex(RenderVertex(mPoints[0].p, weightedNormal)));
		indices.push_back(addVertex(RenderVertex(mPoints[i - 1].p, weightedNormal)));
		indices.push_back(addVertex(RenderVertex(mPoints[i].p, weightedNormal)));
	}
}

void Polygon::triangulateForCollision(std::vector<physx::PxVec3>& verts) const
{
	for(int i = 2; i < (int)mPoints.size(); i++)
	{
		verts.push_back(mPoints[0].p);
		verts.push_back(mPoints[i - 1].p);
		verts.push_back(mPoints[i].p);
	}
}

physx::PxVec3 Polygon::calculateNormal() const
{
	physx::PxVec3 normal(0.0f, 0.0f, 0.0f);
	for(int i = 2; i < (int)mPoints.size(); i++)
	{
		physx::PxVec3 p[3];
		p[0] = mPoints[0].p;
		p[1] = mPoints[i - 1].p;
		p[2] = mPoints[i].p;
		normal += (p[1] - p[0]).cross(p[2] - p[0]);
	}
	normal.normalize();
	return normal;
}

float Polygon::calculateArea() const
{
	float doubleArea = 0.0f;
	for(int i = 2; i < (int)mPoints.size(); i++)
	{
		physx::PxVec3 p[3];
		p[0] = mPoints[0].p;
		p[1] = mPoints[i - 1].p;
		p[2] = mPoints[i].p;
		doubleArea += (p[1] - p[0]).cross(p[2] - p[0]).magnitude();
	}
	return doubleArea*0.5f;
}

void Polygon::subdivideTriangle(std::vector<Polygon>& out) const
{
	if(!isTriangle())
		return;

	for(int i = 0; i < 3; i++)
	{
		out.push_back(Polygon(Point(mPoints[i].p), Point(0.5f*(mPoints[(i+1)%3].p + mPoints[i].p)), Point(0.5f*(mPoints[(i + 2) % 3].p + mPoints[i].p))));
	}
	out.push_back(Polygon(Point(0.5f*(mPoints[0].p + mPoints[1].p)), Point(0.5f*(mPoints[1].p + mPoints[2].p)), Point(0.5f*(mPoints[2].p + mPoints[0].p))));

}

float intersetcLinePlane(physx::PxVec3 a, physx::PxVec3 b, physx::PxVec4 plane)
{
	physx::PxVec3 planeNormal(plane.x, plane.y, plane.z);
	float aprj = planeNormal.dot(a);
	float bprj = planeNormal.dot(b);

	return (-plane.w - aprj) / (bprj - aprj);
}

bool Polygon::pointPlaneSide(physx::PxVec3 p, physx::PxVec4 plane) const
{
	physx::PxVec3 planeNormal(plane.x, plane.y, plane.z);
	return p.dot(planeNormal) + plane.w < 0;
}

void Polygon::clip(physx::PxVec4 plane, bool flip)
{
	if(mPoints.size() < 3)
		return;

	std::vector<Point> input = mPoints;
	mPoints.clear();

	Point S = input.back();
	for(int pointIndex = 0; pointIndex < (int)input.size(); pointIndex++)
	{
		if(pointPlaneSide(input[pointIndex].p, plane) != flip)
		{
			if(pointPlaneSide(S.p, plane) == flip)
			{
				float w = intersetcLinePlane(S.p, input[pointIndex].p, plane);
				mPoints.push_back(S * (1.0f - w) + input[pointIndex] * w);
			}
			mPoints.push_back(input[pointIndex]);
		}
		else if(pointPlaneSide(S.p, plane) != flip)
		{
			float w = intersetcLinePlane(S.p, input[pointIndex].p, plane);
			mPoints.push_back(S * (1.0f - w) + input[pointIndex] * w);
		}
		S = input[pointIndex];
	}
}

void Mesh::addConvexPolygon(physx::PxVec4 plane, physx::PxVec4* planes, uint32_t mask, bool flip)
{
	physx::PxVec3 t1, t2, normal;
	normal.x = plane.x;
	normal.y = plane.y;
	normal.z = plane.z;
	computeBasis(normal, &t1, &t2);
	Polygon poly;
	for(int i = 0; i < 4; i++)
	{
		float xTable[4] = {-1.0f, 1.0f, 1.0f, -1.0f};
		float yTable[4] = {-1.0f, -1.0f, 1.0f, 1.0f};
		poly.mPoints.push_back(normal*-plane.w + 200.0f*t1 * xTable[i] + 200.0f*t2 * yTable[i]);
		//polyTexcoord.push_back(vec2(0.0f, 10.0f) * xTable[i] + vec2(10.0f, 0.0f) * yTable[i]);
	}

	for(int i = 0; i < 32; i++)
	{
		if((1 << i) & mask)
		{
			const physx::PxVec4 pl = planes[i];
			poly.clip(pl, flip);
		}
	}
	
	mPolygons.push_back(poly);
}

void Mesh::generateRenderBuffers(RenderVertex** vertices, uint16_t** indices, int* vertexCount, int* indexCount) const
{
	std::vector<RenderVertex> verts;
	std::vector<uint16_t> inds;
	verts.reserve(mPolygons.size()*3);
	verts.reserve(inds.size()*3);
	
	for(auto& p : mPolygons)
	{
		p.triangulate(verts, inds);
	}

	*vertices = new RenderVertex[verts.size()];
	*indices = new uint16_t[inds.size()];

	memcpy(*vertices, verts.data(), sizeof(RenderVertex)*verts.size());
	memcpy(*indices, inds.data(), sizeof(uint16_t)*inds.size());
	*vertexCount = (uint16_t)verts.size();
	*indexCount = (uint16_t)inds.size();
}

void Mesh::generateSmoothRenderBuffers(RenderVertex** vertices, uint16_t** indices, int* vertexCount, int* indexCount) const
{
	std::vector<RenderVertex> verts;
	std::vector<uint16_t> inds;
	verts.reserve(mPolygons.size() * 3);
	verts.reserve(inds.size() * 3);

	for(auto& p : mPolygons)
	{
		p.triangulateWeld(verts, inds);
	}

	for(auto& v : verts)
	{
		v.n.normalize();
	}

	*vertices = new RenderVertex[verts.size()];
	*indices = new uint16_t[inds.size()];

	memcpy(*vertices, verts.data(), sizeof(RenderVertex)*verts.size());
	memcpy(*indices, inds.data(), sizeof(uint16_t)*inds.size());
	*vertexCount = (uint16_t)verts.size();
	*indexCount = (uint16_t)inds.size();
	
}

int Mesh::generateTriangleList(physx::PxVec3** positions)
{
	std::vector<physx::PxVec3> verts;
	verts.reserve(mPolygons.size() * 3);

	for(auto& p : mPolygons)
	{
		p.triangulateForCollision(verts);
	}

	*positions = new physx::PxVec3[verts.size()];
	memcpy(*positions, verts.data(), sizeof(physx::PxVec3)*verts.size());
	return (int)verts.size();
}

void Mesh::applyTransfom(physx::PxMat44 transform)
{
	for(auto& trig : mPolygons)
		for(auto& point : trig.mPoints)
		{
			point.p = transform.transform(point.p);
		}
}

void Mesh::merge(const Mesh& mesh)
{
	mPolygons.insert(mPolygons.end(), mesh.mPolygons.begin(), mesh.mPolygons.end());
}

Mesh generateTetrahedron(float radius)
{
	Mesh mesh;
	Point p[4];
	for(int i = 0; i < 3; i++)
	{
		p[i] = Point(radius*physx::PxVec3(cosf((float)i / 3.0f*PxTwoPi), -sqrtf(2.0f / 3.0f)*0.5f*sqrtf(3), sinf((float)i / 3.0f*PxTwoPi)));
	}
	p[3] = Point(radius*physx::PxVec3(0, sqrtf(2.0f / 3.0f)*0.5f*sqrtf(3), 0));

	mesh.mPolygons.push_back(Polygon(p[0], p[1], p[2]));
	mesh.mPolygons.push_back(Polygon(p[3], p[1], p[0]));
	mesh.mPolygons.push_back(Polygon(p[3], p[2], p[1]));
	mesh.mPolygons.push_back(Polygon(p[3], p[0], p[2]));

	return mesh;
}

Mesh generateIcosahedron(float radius, int subdivisions)
{
	Mesh mesh;
	Point p[12];

	//generate positions 
	float goldenRatio = (1.0f + sqrtf(5.0f)) * 0.5f;
	float scale = radius / physx::PxVec2(goldenRatio, 1.0f).magnitude();
	for(int j = 0; j < 3; j++)
	for(int i = 0; i < 4; i++)
	{
		float signA = i & 1 ? 1.0f : -1.0f;
		float signB = i & 2 ? -1.0f : 1.0f;
		physx::PxVec3 point(signA, signB * goldenRatio, 0.0f);
		p[i + 4 * j] = physx::PxVec3(point[j % 3], point[(j + 1) % 3], point[(j + 2) % 3]) * scale;
	}

	//generate triangles
	uint16_t ti[20 * 3] =
	{
		0, 7, 9,
		0, 9, 1,
		0, 1, 11,
		0, 11, 6,
		0, 6, 7,

		1, 9, 5,
		9, 7, 8,
		7, 6, 2,
		6, 11, 10,
		11, 1, 4,

		3, 5, 8,
		3, 8, 2,
		3, 2, 10,
		3, 10, 4,
		3, 4, 5,

		8, 5, 9,
		2, 8, 7,
		10, 2, 6,
		4, 10, 11,
		5, 4, 1
	};

	for(int i = 0; i < 20*3; i += 3)
	{
		mesh.mPolygons.push_back(Polygon(p[ti[i]], p[ti[i+1]], p[ti[i+2]]));
	}

	bool projectToSphere = subdivisions > 0;
	while(subdivisions > 0)
	{
		subdivisions--;
		Mesh sub;
		for(auto& trig : mesh.mPolygons)
		{
			trig.subdivideTriangle(sub.mPolygons);
		}
		std::swap(sub.mPolygons, mesh.mPolygons);
	}

	if(projectToSphere)
	{
		for(auto& trig : mesh.mPolygons)
			for(auto& point : trig.mPoints)
			{
				point.p = point.p.getNormalized() * radius;
			}
	}

	return mesh;
}

Mesh generateCone(physx::PxVec4 a, physx::PxVec4 b, int segments, float grow, bool correctCone)
{
	Mesh mesh;

	if(a.w < b.w)
		std::swap(a, b);

	physx::PxVec3 aCenter = a.getXYZ();
	physx::PxVec3 bCenter = b.getXYZ();
	float aRadius = a.w + grow;
	float bRadius = b.w + grow;

	physx::PxVec3 basis[3];
	basis[2] = bCenter - aCenter;
	basis[2].normalize();
	computeBasis(basis[2], &basis[0], &basis[1]);

	physx::PxVec3 pa = aCenter + aRadius*basis[0];
	physx::PxVec3 pb = bCenter + bRadius*basis[0];
	physx::PxVec3 dir = pb - pa;

	physx::PxVec3 n = basis[2].cross(dir);
	physx::PxVec3 n2 = dir.cross(n);
	physx::PxVec3 focusPoint = aCenter + ((pa - aCenter).dot(n2)) / basis[2].dot(n2) * basis[2];

	if(correctCone)
	{
		{
			float focusDistance = (focusPoint - aCenter).magnitude();
			physx::PxVec3 cCenter = (focusPoint + aCenter)*0.5f;
			float cRadius = focusDistance*0.5f;
			float d = (aCenter - cCenter).magnitude();
			float a = (aRadius*aRadius - cRadius*cRadius + d*d) / (2.0f*d);
			float h = sqrtf(aRadius*aRadius - a*a);
			physx::PxVec3 P3 = aCenter + a * (cCenter - aCenter) / d;

			aCenter = P3;
			aRadius = h;
		}

		{
			float focusDistance = (focusPoint - bCenter).magnitude();
			physx::PxVec3 cCenter = (focusPoint + bCenter)*0.5f;
			float cRadius = focusDistance*0.5f;
			float d = (bCenter - cCenter).magnitude();
			float a = (bRadius*bRadius - cRadius*cRadius + d*d) / (2.0f*d);
			float h = sqrtf(bRadius*bRadius - a*a);
			physx::PxVec3 P3 = bCenter + a * (cCenter - bCenter) / d;

			bCenter = P3;
			bRadius = h;
		}
	}


	for(int i = 0; i < segments; i++)
	{
		float angle1 = (float)i / (float)segments*physx::PxTwoPi;
		float angle2 = (float)(i+1) / (float)segments*physx::PxTwoPi;

		Polygon p;
		p.addPoints(Point(aCenter + (cosf(angle1)*basis[0] + sinf(angle1)*basis[1])*aRadius));
		p.addPoints(Point(aCenter + (cosf(angle2)*basis[0] + sinf(angle2)*basis[1])*aRadius));
		p.addPoints(Point(bCenter + (cosf(angle2)*basis[0] + sinf(angle2)*basis[1])*bRadius));
		p.addPoints(Point(bCenter + (cosf(angle1)*basis[0] + sinf(angle1)*basis[1])*bRadius));

		mesh.mPolygons.push_back(p);
	}

	return mesh;
}

Mesh generateCollisionConvex(physx::PxVec4* planes, uint32_t mask, float grow, bool flip)
{
	Mesh mesh;
	if(grow != 0.0f)
	{
		physx::PxVec4* grownPlanes = new physx::PxVec4[32];
		for(int i = 0; i < 32; i++)
		{
			if((1 << i) & mask)
			{
				grownPlanes[i] = planes[i];
				grownPlanes[i].w -= grow;
			}
		}
		planes = grownPlanes;
	}

	for(int i = 0; i < 32; i++)
	{
		if((1 << i) & mask)
			mesh.addConvexPolygon(planes[i], planes, mask ^ (1 << i), flip);
	}

	if(grow != 0.0f)
		delete[] planes;

	return mesh;
}

Mesh generateCollisionCapsules(physx::PxVec4* spheres, int sphereCount, uint32_t* indices, int indexCount, float grow)
{
	Mesh finalMesh;
	for(int i = 0; i < sphereCount; i++)
	{
		Mesh sphere = generateIcosahedron(spheres[i].w+ grow, 4);
		sphere.applyTransfom(physx::PxTransform(spheres[i].getXYZ()));
		finalMesh.merge(sphere);
	}

	for(int i = 0; i < indexCount; i += 2)
	{
		finalMesh.merge(generateCone(spheres[indices[i]], spheres[indices[i + 1]], 32, grow, true));
	}

	return finalMesh;
}

uint32_t generateConvexPolyhedronPlanes(int segmentsX, int segmentsY, physx::PxVec3 center, float radius, std::vector<physx::PxVec4>* planes)
{
	int offset = 0;
	if(planes)
	{
		planes->reserve(planes->size() + segmentsX*segmentsY);
		offset = (int)planes->size();
	}

	segmentsY += 1;
	for(int i = 1; i < segmentsY; i++)
	{
		float angleY = (float)i / (float)segmentsY * physx::PxPi + physx::PxPiDivTwo;
		for(int j = 0; j < segmentsX; j++)
		{
			float angleX = (float)j / (float)segmentsX * physx::PxTwoPi;

			physx::PxVec3 nx(cosf(angleX), 0.0f, sinf(angleX));
			physx::PxVec3 n = cosf(angleY) * nx + sinf(angleY)*physx::PxVec3(0.0f, 1.0f, 0.0f);

			physx::PxVec3 p = n*radius + center;

			if(planes) planes->push_back(constructPlaneFromPointNormal(p, n));
		}
	}
	uint64_t shift = (segmentsX * (segmentsY - 1) + offset);
	uint64_t excludeMask = (((uint64_t)1 << offset) - 1);
	uint64_t mask = (((uint64_t)1 << shift) - 1) & ~excludeMask;
	return static_cast<uint32_t>(mask);
}

MeshGeneratorRenderMesh::MeshGeneratorRenderMesh(const Mesh mesh)
{
	RenderVertex* vertices;
	uint16_t* indices;
	int vertexCount, indexCount;
	mesh.generateRenderBuffers(&vertices, &indices, &vertexCount, &indexCount);

	std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
	layout.push_back({"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0});
	layout.push_back({"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0});

	initialize(vertices, (uint32_t)vertexCount, sizeof(RenderVertex), layout, indices, indexCount);

	delete vertices;
	delete indices;
}
MeshGeneratorRenderMesh::~MeshGeneratorRenderMesh()
{

}

MeshGeneratorRenderMeshSmooth::MeshGeneratorRenderMeshSmooth(const Mesh mesh)
{
	RenderVertex* vertices;
	uint16_t* indices;
	int vertexCount, indexCount;
	mesh.generateSmoothRenderBuffers(&vertices, &indices, &vertexCount, &indexCount);

	std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
	layout.push_back({"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0});
	layout.push_back({"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0});

	initialize(vertices, (uint32_t)vertexCount, sizeof(RenderVertex), layout, indices, indexCount);

	delete vertices;
	delete indices;
}
MeshGeneratorRenderMeshSmooth::~MeshGeneratorRenderMeshSmooth()
{

}



}