#ifndef UTILS_H
#define UTILS_H

#include <DeviceManager.h>
#include <assert.h>
#include <foundation/PxVec3.h>
#include <foundation/PxVec4.h>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//														MACROS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef V_RETURN
#define V_RETURN(x)                                                                                                    \
	{                                                                                                                  \
		hr = (x);                                                                                                      \
		if(FAILED(hr))                                                                                                 \
		{                                                                                                              \
			return hr;                                                                                                 \
		}                                                                                                              \
	}
#endif

#ifndef V
#define V(x)                                                                                                           \
	{                                                                                                                  \
		HRESULT hr = (x);                                                                                              \
		_ASSERT(SUCCEEDED(hr));                                                                                        \
	}
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)                                                                                                \
	{                                                                                                                  \
		if(p)                                                                                                          \
		{                                                                                                              \
			(p)->Release();                                                                                            \
			(p) = NULL;                                                                                                \
		}                                                                                                              \
	}
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)                                                                                                 \
	{                                                                                                                  \
		if(p)                                                                                                          \
		{                                                                                                              \
			delete (p);                                                                                                \
			(p) = NULL;                                                                                                \
		}                                                                                                              \
	}
#endif

#define ASSERT_PRINT(cond, format, ...)                                                                                \
	if(!(cond))                                                                                                        \
	{                                                                                                                  \
		messagebox_printf("Assertion Failed!", MB_OK | MB_ICONERROR, #cond "\n" format, __VA_ARGS__);                  \
		assert(cond);                                                                                                  \
	}

HRESULT messagebox_printf(const char* caption, UINT mb_type, const char* format, ...);



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* strext(const char* str)
{
	const char* ext = NULL; // by default no extension found!
	while (str)
	{
		str = strchr(str, '.');
		if (str)
		{
			str++;
			ext = str;
		}
	}
	return ext;
}

//Math utilities
static inline float  lerp(float a, float b, float t)                          { return a + (b - a) * t; }

/** returns a PxVec4 containing [x,y,z,d] for plane equation ax + by + cz + d = 0.
* Where plane contains p and has normal n.
*/
inline physx::PxVec4 constructPlaneFromPointNormal(physx::PxVec3 p, physx::PxVec3 n)
{
	n.normalize();
	return physx::PxVec4(n, -p.dot(n));
}

/** returns two vectors in b and c so that [a b c] form a basis.
* a needs to be a unit vector.
*/
inline void computeBasis(const physx::PxVec3& a, physx::PxVec3* b, physx::PxVec3* c)
{
	if(fabsf(a.x) >= 0.57735f)
		*b = physx::PxVec3(a.y, -a.x, 0.0f);
	else
		*b = physx::PxVec3(0.0f, a.z, -a.y);

	*b = b->getNormalized();
	*c = a.cross(*b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif