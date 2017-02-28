// This code contains NVIDIA Confidential Information and is disclosed
// under the Mutual Non-Disclosure Agreement.
//
// Notice
// ALL NVIDIA DESIGN SPECIFICATIONS AND CODE ("MATERIALS") ARE PROVIDED "AS IS" NVIDIA MAKES
// NO REPRESENTATIONS, WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ANY IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
//
// NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. No third party distribution is allowed unless
// expressly authorized by NVIDIA.  Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (C) 2012, NVIDIA Corporation. All rights reserved.
//
// NVIDIA Corporation and its licensors retain all intellectual property and proprietary
// rights in and to this software and related documentation and any modifications thereto.
// Any use, reproduction, disclosure or distribution of this software and related
// documentation without an express license agreement from NVIDIA Corporation is
// strictly prohibited.

#pragma once

/*===========================================================================
GFSDK_Common.h

GFSDK_Common defines common macros, types and structs used generally by
all NVIDIA TECH Libraries.  You won't find anything particularly interesting
here.

For any issues with this particular header please contact:

devsupport@nvidia.com

For any issues with the specific FX library you are using, see the header
file for that library for contact information.

===========================================================================*/

#pragma pack(push,8) // Make sure we have consistent structure packings

#ifdef __cplusplus
extern "C" {
#endif


/*===========================================================================
AUTO CONFIG
===========================================================================*/
#ifndef __GFSDK_COMMON_AUTOCONFIG
#define __GFSDK_COMMON_AUTOCONFIG

  #ifdef __GNUC__
    #define __GFSDK_CC_GNU__ 1
    #define __GFSDK_CC_MSVC__ 0
  #else
    #define __GFSDK_CC_GNU__ 0
    #define __GFSDK_CC_MSVC__ 1
  #endif

#endif


/*===========================================================================
FORWARD DECLARATIONS 
===========================================================================*/
#ifndef __GFSDK_COMMON_FORWARDDECLS
#define __GFSDK_COMMON_FORWARDDECLS

    #if defined ( __GFSDK_GL__ )
		// $ TODO Add common fwd decls for OGL
	#elif defined ( __GFSDK_DX11__ )
		// $ TODO Add common fwd decls for D3D11
    #endif

#endif // __GFSDK_COMMON_FORWARDDECLS


/*===========================================================================
TYPES
===========================================================================*/
#ifndef __GFSDK_COMMON_TYPES
#define __GFSDK_COMMON_TYPES

  typedef unsigned char gfsdk_U8; 
  typedef unsigned short gfsdk_U16;
  typedef signed int gfsdk_S32; 
  typedef signed long long gfsdk_S64; 
  typedef unsigned int gfsdk_U32; 
  typedef unsigned long long gfsdk_U64;   
  typedef float gfsdk_F32; 

  typedef struct {
    gfsdk_F32 x;
    gfsdk_F32 y;
  } gfsdk_float2;

  typedef struct {
    gfsdk_F32 x;
    gfsdk_F32 y;
    gfsdk_F32 z;
  } gfsdk_float3;

  typedef struct {
    gfsdk_F32 x;
    gfsdk_F32 y;
    gfsdk_F32 z;
    gfsdk_F32 w;
  } gfsdk_float4;

  // implicit row major
  typedef struct {
    gfsdk_F32 _11, _12, _13, _14;
    gfsdk_F32 _21, _22, _23, _24;
    gfsdk_F32 _31, _32, _33, _34;
    gfsdk_F32 _41, _42, _43, _44;
  } gfsdk_float4x4;

	typedef bool                 gfsdk_bool;
	typedef char                 gfsdk_char;
	typedef const gfsdk_char*    gfsdk_cstr;
	typedef double               gfsdk_F64;

#endif // __GFSDK_COMMON_TYPES


/*=========================================================================
Useful default matrices/vectors
===========================================================================*/

static gfsdk_float4x4 GFSDK_Identity_Matrix = {	1,0,0,0,
												0,1,0,0,
												0,0,1,0,
												0,0,0,1 };

static gfsdk_float2 GFSDK_Zero_Vector2 = { 0, 0 };
static gfsdk_float2 GFSDK_One_Vector2 = { 1, 1 };
static gfsdk_float3 GFSDK_Zero_Vector3 = { 0, 0, 0 };
static gfsdk_float3 GFSDK_One_Vector3 = { 1, 1, 1 };


/*===========================================================================
MACROS
===========================================================================*/
#ifndef __GFSDK_COMMON_MACROS
#define __GFSDK_COMMON_MACROS

  // GNU
  #if __GFSDK_CC_GNU__
    #define __GFSDK_ALIGN__(bytes) __attribute__((aligned (bytes)))
    #define __GFSDK_EXPECT__(exp,tf) __builtin_expect(exp, tf)
    #define __GFSDK_INLINE__ __attribute__((always_inline))
    #define __GFSDK_NOLINE__ __attribute__((noinline))
    #define __GFSDK_RESTRICT__ __restrict
  /*-------------------------------------------------------------------------*/
    #define __GFSDK_CDECL__
    #define __GFSDK_IMPORT__
    #define __GFSDK_EXPORT__
    #define __GFSDK_STDCALL__
  #endif


  // MSVC
  #if __GFSDK_CC_MSVC__
    #define __GFSDK_ALIGN__(bytes) __declspec(align(bytes))
    #define __GFSDK_EXPECT__(exp, tf) (exp)
    #define __GFSDK_INLINE__ __forceinline
    #define __GFSDK_NOINLINE__
    #define __GFSDK_RESTRICT__ __restrict
  /*-------------------------------------------------------------------------*/
    #define __GFSDK_CDECL__ __cdecl
    #define __GFSDK_IMPORT__ __declspec(dllimport)
    #define __GFSDK_EXPORT__ __declspec(dllexport)
    #define __GFSDK_STDCALL__ __stdcall
  #endif

#endif // __GFSDK_COMMON_MACROS


/*===========================================================================
RETURN CODES
===========================================================================*/
#ifndef __GFSDK_COMMON_RETURNCODES
#define __GFSDK_COMMON_RETURNCODES

  #define GFSDK_RETURN_OK   0
  #define GFSDK_RETURN_FAIL 1
  #define GFSDK_RETURN_EMULATION 2

#endif // __GFSDK_COMMON_RETURNCODES


/*===========================================================================
CUSTOM HEAP
===========================================================================*/
#ifndef __GFSDK_COMMON_CUSTOMHEAP
#define __GFSDK_COMMON_CUSTOMHEAP

  typedef struct {
    void* (*new_)(size_t);
    void (*delete_)(void*);
  } gfsdk_new_delete_t;

#endif // __GFSDK_COMMON_CUSTOMHEAP


/*===========================================================================
HANDLE DECLARATION
===========================================================================*/
#ifndef __GFSDK_COMMON_DECLHANDLE
#define __GFSDK_COMMON_DECLHANDLE
#define GFSDK_DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#endif // __GFSDK_COMMON_DECLHANDLE


/*===========================================================================
Forward Declarations
===========================================================================*/

// Shadow buffer handle
struct GFSDK_ShadowLib_Buffer;		 

// Shadow map handle
struct GFSDK_ShadowLib_Map;


/*===========================================================================
Resource Structure Declarations (different for each platform)
===========================================================================*/

struct GFSDK_ShadowLib_DeviceContext
{
	#if defined ( __GFSDK_DX11__ )
		
		ID3D11Device*				pD3DDevice;
		ID3D11DeviceContext*		pDeviceContext;

		GFSDK_ShadowLib_DeviceContext()
		{
			pD3DDevice = NULL;
			pDeviceContext = NULL;
		}

	#elif defined ( __GFSDK_GL__ )

		// $ TO implement

    #endif
};

struct GFSDK_ShadowLib_RenderTargetView
{
	#if defined ( __GFSDK_DX11__ )
		
		ID3D11RenderTargetView*		pRTV;

		GFSDK_ShadowLib_RenderTargetView()
		{
			pRTV = NULL;
		}

	#elif defined ( __GFSDK_GL__ )

		GLuint	FBO;

		GFSDK_ShadowLib_RenderTargetView()
		{
			FBO = 0;
		}

    #endif
};

struct GFSDK_ShadowLib_ShaderResourceView
{
	#if defined ( __GFSDK_DX11__ )
		
		ID3D11ShaderResourceView*	pSRV;

		GFSDK_ShadowLib_ShaderResourceView()
		{
			pSRV = NULL;
		}

	#elif defined ( __GFSDK_GL__ )

		GLuint	Texture;

		GFSDK_ShadowLib_ShaderResourceView()
		{
			Texture = 0;
		}

    #endif
};

struct GFSDK_ShadowLib_DepthStencilView
{
	#if defined ( __GFSDK_DX11__ )
		
		ID3D11DepthStencilView*	pDSV;

		GFSDK_ShadowLib_DepthStencilView()
		{
			pDSV = NULL;
		}

	#elif defined ( __GFSDK_GL__ )

		GLuint	Texture;

		GFSDK_ShadowLib_DepthStencilView()
		{
			Texture = 0;
		}

    #endif
};

struct GFSDK_ShadowLib_ScissorRect
{
	#if defined ( __GFSDK_DX11__ )
		D3D11_RECT*		pSR;

		GFSDK_ShadowLib_ScissorRect()
		{
			pSR = NULL;
		}

	#elif defined ( __GFSDK_GL__ )

		// $ TO implement

    #endif
};

struct GFSDK_ShadowLib_Texture2D
{
	gfsdk_U32					uWidth;
	gfsdk_U32					uHeight;
	gfsdk_U32					uSampleCount;

	#if defined ( __GFSDK_DX11__ )

		DXGI_FORMAT					Format;
		ID3D11Texture2D*			pTexture;
		ID3D11ShaderResourceView*	pSRV;
		ID3D11RenderTargetView*		pRTV;

	#elif defined ( __GFSDK_GL__ )

		GLenum						Type;
		GLenum						InternalFormat;
		GLenum						Format;
		GLuint						Texture;
		GLuint						FBO;

    #endif

	GFSDK_ShadowLib_Texture2D()
	{
		uWidth			= 0;
		uHeight			= 0;
		uSampleCount	= 1;
		
		#if defined ( __GFSDK_DX11__ )
			
			Format			= DXGI_FORMAT_UNKNOWN;
			pTexture		= NULL;
			pSRV			= NULL;
			pRTV			= NULL;

		#elif defined ( __GFSDK_GL__ )

			Type			= 0;
			InternalFormat	= 0;
			Format			= 0;
			Texture			= 0;
			FBO				= 0;

		#endif
	}
};


/*===========================================================================
Interface defines
===========================================================================*/

#define __GFSDK_SHADOWLIB_INTERFACE__ GFSDK_ShadowLib_Status __GFSDK_CDECL__

#ifdef __DLL_GFSDK_SHADOWLIB_EXPORTS__
	#define __GFSDK_SHADOWLIB_EXTERN_INTERFACE__ __GFSDK_EXPORT__ __GFSDK_SHADOWLIB_INTERFACE__
#else
	#define __GFSDK_SHADOWLIB_EXTERN_INTERFACE__ __GFSDK_IMPORT__ __GFSDK_SHADOWLIB_INTERFACE__
#endif

#define GFSDK_ShadowLib_FunctionPointer (void(*)(void*,gfsdk_float4x4*))

#ifdef __cplusplus
}; //extern "C" {

#endif

#pragma pack(pop)


/*===========================================================================
EOF
===========================================================================*/
