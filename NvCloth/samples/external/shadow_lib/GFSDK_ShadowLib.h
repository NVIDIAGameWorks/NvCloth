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


/*===========================================================================
                                  GFSDK_ShadowLib.h
=============================================================================

GFSDK_ShadowLib written by Jon Story
-----------------------------------------
This library provides an API for rendering various kinds of shadow maps, 
and then rendering a shadow buffer using the desired filtering technique. 
The programmer is expected to provide either:
1) a pointer to a function containing all the necessary render calls for the shadow map(s).
2) pre-rendered shadow maps
In order to render the shadow buffer the programmer must supply a depth buffer. The library 
returns a SRV of the shadow buffer for modulation with the scene color buffer.

NOTE:
1) The library tracks and restores all graphics API state it uses.
2) All matrices are expected to be row major.

The typical expected use scenario would look like this:

GameEngine::OnCreateDevice()
{
	GFSDK_ShadowLib_GetVersion()
	GFSDK_ShadowLib_Create()
	GFSDK_ShadowLib_Context::AddBuffer()

	// Optional - if not providing your own shadow maps
	GFSDK_ShadowLib_Context::AddMap()
}

GameEngine::OnResizeWindow()
{
	GFSDK_ShadowLib_Context::RemoveBuffer()
	GFSDK_ShadowLib_Context::AddBuffer()
}

GameEngine::OnRender()
{
	// Optional - depending on the render settings some temp resources may be required
	// by the lib, see the GFSDK_ShadowLib_TempResources structure for details
	GFSDK_ShadowLib_Context::SetTempResources()

	// Optional - if not providing your own shadow maps
	GFSDK_ShadowLib_Context::RenderMap()			
	
	GFSDK_ShadowLib_Context::ClearBuffer()
	GFSDK_ShadowLib_Context::RenderBuffer()
	GFSDK_ShadowLib_Context::FinalizeBuffer()
	
	// Optional - if not wanting to perform a custom modulate with scene color
	GFSDK_ShadowLib_Context::ModulateBuffer()	
}

GameEngine::OnDestroyDevice()
{
	// Optional - if not providing your own shadow maps
	GFSDK_ShadowLib_Context::RemoveMap()			

	GFSDK_ShadowLib_Context::RemoveBuffer()
	GFSDK_ShadowLib_Context::Destroy()
}

ENGINEERING CONTACT
Jon Story (NVIDIA Senior Developer Technology Engineer)
jons@nvidia.com

===========================================================================*/

#pragma once

// defines general GFSDK includes and structs
#include "GFSDK_ShadowLib_Common.h"


/*===========================================================================
Version
===========================================================================*/

// The major, minor and CL version of this library header file
#define GFSDK_SHADOWLIB_MAJOR_VERSION	2
#define GFSDK_SHADOWLIB_MINOR_VERSION	0
#define GFSDK_SHADOWLIB_CHANGE_LIST "$Change: 20145232 $"


/*===========================================================================
Version structure
===========================================================================*/
struct GFSDK_ShadowLib_Version
{
	unsigned int uMajor;
	unsigned int uMinor;
};


/*===========================================================================
Return codes for the lib
===========================================================================*/
enum GFSDK_ShadowLib_Status
{
	// Success
    GFSDK_ShadowLib_Status_Ok											=  0,	
	// Fail
    GFSDK_ShadowLib_Status_Fail											= -1,   
	// Mismatch between header and dll
	GFSDK_ShadowLib_Status_Invalid_Version								= -2,	
	// One or more invalid parameters
	GFSDK_ShadowLib_Status_Invalid_Parameter							= -3,	
	// Failed to allocate a resource
	GFSDK_ShadowLib_Status_Out_Of_Memory								= -4,	
	// The library is being used by another thread
	GFSDK_ShadowLib_Busy												= -5,
	// glewInit failed
	GFSDK_ShadowLib_GLEW_INIT_FAILED									= -6,
	// glewIsSupported("GL_VERSION_3_2") returned false
    GFSDK_ShadowLib_GL_CORE_VERSION_NOT_SUPPORTED						= -7,               
};


/*===========================================================================
Shadow map types
===========================================================================*/
enum GFSDK_ShadowLib_MapType
{
	GFSDK_ShadowLib_MapType_Texture,
	GFSDK_ShadowLib_MapType_TextureArray,
	GFSDK_ShadowLib_MapType_Max,
};


/*===========================================================================
Channel from which to sample depth in the supplied depth SRV
===========================================================================*/
enum GFSDK_ShadowLib_EyeDepthChannel
{
	GFSDK_ShadowLib_EyeDepthChannel_R,
	GFSDK_ShadowLib_EyeDepthChannel_G,
	GFSDK_ShadowLib_EyeDepthChannel_B,
	GFSDK_ShadowLib_EyeDepthChannel_A,
	GFSDK_ShadowLib_EyeDepthChannel_Max,
};


/*===========================================================================
Filtering technique types
===========================================================================*/
enum GFSDK_ShadowLib_TechniqueType
{
	// A pure hard shadow
	GFSDK_ShadowLib_TechniqueType_Hard,
	// Regular box PCF shadow
	GFSDK_ShadowLib_TechniqueType_PCF,
	// PCSS+
	GFSDK_ShadowLib_TechniqueType_PCSS,
	GFSDK_ShadowLib_TechniqueType_Max,
};


/*===========================================================================
Quality type of the filtering technique to be used
===========================================================================*/
enum GFSDK_ShadowLib_QualityType
{
	// Quality settings only apply to the PCSS+ technique
	// Cascaded maps (C0=Low,C1=Low,C2=Low,C3=Low)
	GFSDK_ShadowLib_QualityType_Low,
	// Cascaded maps (C0=Medium,C1=Low,C2=Low,C3=Low)
	GFSDK_ShadowLib_QualityType_Medium,
	// Cascaded maps (C0=High,C1=Medium,C2=Low,C3=Low)
	GFSDK_ShadowLib_QualityType_High,
	GFSDK_ShadowLib_QualityType_Max,
};


/*===========================================================================
Accepted shadow map view types
===========================================================================*/
enum GFSDK_ShadowLib_ViewType
{
	// Must use GFSDK_ShadowLib_MapType_Texture with GFSDK_ShadowLib_ViewType_Single
	GFSDK_ShadowLib_ViewType_Single		= 1,	
	GFSDK_ShadowLib_ViewType_Cascades_2	= 2,
	GFSDK_ShadowLib_ViewType_Cascades_3	= 3,
	GFSDK_ShadowLib_ViewType_Cascades_4	= 4,
};


/*===========================================================================
Light source types
===========================================================================*/
enum GFSDK_ShadowLib_LightType
{
	GFSDK_ShadowLib_LightType_Directional,
	GFSDK_ShadowLib_LightType_Spot,
	GFSDK_ShadowLib_LightType_Max,
};


/*===========================================================================
Per-sample mode to use
===========================================================================*/
enum GFSDK_ShadowLib_MSAARenderMode
{
	// Every pixel in the shadow buffer is considered to be complex
	// and will be run at sample frequency
	GFSDK_ShadowLib_MSAARenderMode_BruteForce,
	// The application provides a stencil mask and ref values which
	// indicate which pixels are considered complex / simple
	GFSDK_ShadowLib_MSAARenderMode_ComplexPixelMask,
	GFSDK_ShadowLib_MSAARenderMode_Max,
};


/*===========================================================================
Modulate buffer mask
===========================================================================*/
enum GFSDK_ShadowLib_ModulateBufferMask
{
	GFSDK_ShadowLib_ModulateBufferMask_RGB,
	GFSDK_ShadowLib_ModulateBufferMask_R,
	GFSDK_ShadowLib_ModulateBufferMask_G,
	GFSDK_ShadowLib_ModulateBufferMask_B,
	GFSDK_ShadowLib_ModulateBufferMask_A,
	GFSDK_ShadowLib_ModulateBufferMask_Max,
};


/*===========================================================================
Light descriptor
===========================================================================*/
struct GFSDK_ShadowLib_LightDesc
{
	GFSDK_ShadowLib_LightType	eLightType;
	gfsdk_float3				v3LightPos;
	gfsdk_float3				v3LightLookAt;
	gfsdk_F32					fLightSize;
	// Params for fading off shadows by distance from source (ignored if bLightFalloff == false):
	// float3 v3LightVec = ( v3LightPos - v3PositionWS ) / fLightRadius;
	// float fDistanceScale = square( max( 0, length( v3LightVec ) * fLightFalloffDistance + fLightFalloffBias ) );
	// float fLerp = pow( saturate( 1.0f - fDistanceScale ), fLightFalloffExponent );
	// Shadow = lerp( 1.0f, Shadow, fLerp );
	gfsdk_bool					bLightFalloff;
	gfsdk_F32					fLightFalloffRadius;
	gfsdk_F32					fLightFalloffDistance;
	gfsdk_F32					fLightFalloffBias;
	gfsdk_F32					fLightFalloffExponent;

	// Defaults
	GFSDK_ShadowLib_LightDesc()
	{
		eLightType				= GFSDK_ShadowLib_LightType_Directional;
		v3LightPos				= GFSDK_One_Vector3;
		v3LightLookAt			= GFSDK_Zero_Vector3;
		fLightSize				= 1.0f;
		bLightFalloff			= false;
		fLightFalloffRadius		= 0.0f;
		fLightFalloffDistance	= 0.0f;
		fLightFalloffBias		= 0.0f;
		fLightFalloffExponent	= 0.0f;
	}
};


/*===========================================================================
Shadow buffer descriptor
===========================================================================*/
struct GFSDK_ShadowLib_BufferDesc
{
	gfsdk_U32							uResolutionWidth;
	gfsdk_U32							uResolutionHeight;
	gfsdk_U32							uSampleCount;
	// A read only depth stencil view, which is required for the optimized per sample
	// render path and the stencil channel is expected to contain a mask for complex
	// pixels. It is also a requirment for running in stereo mode (can be NULL otherwise) 
	GFSDK_ShadowLib_DepthStencilView	ReadOnlyDSV;

	// Defaults
	GFSDK_ShadowLib_BufferDesc()
	{
		uResolutionWidth	= 0;
		uResolutionHeight	= 0;
		uSampleCount		= 1;
	}
};


/*===========================================================================
Depth buffer descriptor
===========================================================================*/
struct GFSDK_ShadowLib_DepthBufferDesc
{
	// The depth SRV from which depth values will be read
	GFSDK_ShadowLib_ShaderResourceView	DepthStencilSRV;
	// A single sample version of depth, which is required for the optimized per sample
	// render path (can be NULL otherwise)
	GFSDK_ShadowLib_ShaderResourceView	ResolvedDepthStencilSRV;
	// The 2 ref values used during stencil testing to signal which pixels are considered
	// complex and which simple
	gfsdk_U32							uComplexRefValue;
	gfsdk_U32							uSimpleRefValue;
	// Determines from which channel to read depth values from in the input depth SRV
	GFSDK_ShadowLib_EyeDepthChannel		eEyeDepthChannel;
	// Set to true to invert depth buffer values
	gfsdk_bool							bInvertEyeDepth;
	
	// Defaults
	GFSDK_ShadowLib_DepthBufferDesc()
	{
		uComplexRefValue				= 0x01;
		uSimpleRefValue					= 0x00;
		eEyeDepthChannel				= GFSDK_ShadowLib_EyeDepthChannel_R;
		bInvertEyeDepth					= false;
	}
};


/*===========================================================================
Descriptor used to place each view with support for sub-regions of a shadow map
===========================================================================*/
struct GFSDK_ShadowLib_ViewLocationDesc
{
	// Array index of the shadow map (in the GFSDK_ShadowLib_MapType_TextureArray) containing this view
	gfsdk_U32		uMapID;	
	// The origin is considered to be: DX = Top Left, GL = Bottom Left 
	gfsdk_float2	v2Origin;
	gfsdk_float2	v2Dimension;

	// Defaults
	GFSDK_ShadowLib_ViewLocationDesc()
	{
		uMapID		= 0;
		v2Origin	= GFSDK_Zero_Vector2;
		v2Dimension = GFSDK_Zero_Vector2;
	}
};


/*===========================================================================
Map descriptor
===========================================================================*/
struct GFSDK_ShadowLib_MapDesc
{
	gfsdk_U32							uResolutionWidth;
	gfsdk_U32							uResolutionHeight;
	GFSDK_ShadowLib_MapType				eMapType;
	// Use this to optionally downsample a shadow map to increase performance of the
	// blocker search for the PCSS technique (intended use case is for high resolution shadow maps). 
	// This pathway is only supported for: GFSDK_ShadowLib_ViewType_Single, GFSDK_ShadowLib_MapType_Texture & GFSDK_ShadowLib_TechniqueType_PCSS
	// Setting this to true, will require you to provide the temp resource for the downsample, the resolution of the resource will determine 
	// the downsample size.
	gfsdk_bool							bDownsample;
	// Size of array (in the case of GFSDK_ShadowLib_MapType_TextureArray)
	gfsdk_U32							uArraySize;	
	GFSDK_ShadowLib_ViewType			eViewType;
	GFSDK_ShadowLib_ViewLocationDesc	ViewLocation[GFSDK_ShadowLib_ViewType_Cascades_4];	

	// Defaults
	GFSDK_ShadowLib_MapDesc()
	{
		uResolutionWidth			= 0;
		uResolutionHeight			= 0;
		eMapType					= GFSDK_ShadowLib_MapType_Texture;
		bDownsample					= false;
		uArraySize					= 1;
		eViewType					= GFSDK_ShadowLib_ViewType_Single;
	}
};


/*===========================================================================
External map descriptor
===========================================================================*/
struct GFSDK_ShadowLib_ExternalMapDesc
{
	GFSDK_ShadowLib_MapDesc		Desc;
	gfsdk_float4x4				m4x4EyeViewMatrix;
	gfsdk_float4x4				m4x4EyeProjectionMatrix;
	gfsdk_float4x4				m4x4LightViewMatrix;
	gfsdk_float4x4				m4x4LightProjMatrix[GFSDK_ShadowLib_ViewType_Cascades_4];
	// Pure offset added to eye Z values (in shadow map space) (on a per cascade basis)
	gfsdk_F32					fBiasZ[GFSDK_ShadowLib_ViewType_Cascades_4]; 
	// Global scale of shadow intensity: shadow buffer = lerp( Shadow,  1.0f, fShadowIntensity );
	gfsdk_F32					fShadowIntensity;	
	GFSDK_ShadowLib_LightDesc	LightDesc;

	// Defaults
	GFSDK_ShadowLib_ExternalMapDesc()
	{
		m4x4EyeViewMatrix								= GFSDK_Identity_Matrix;
		m4x4EyeProjectionMatrix							= GFSDK_Identity_Matrix;
		m4x4LightViewMatrix								= GFSDK_Identity_Matrix;
		m4x4LightProjMatrix[0]							= GFSDK_Identity_Matrix;
		m4x4LightProjMatrix[1]							= GFSDK_Identity_Matrix;
		m4x4LightProjMatrix[2]							= GFSDK_Identity_Matrix;
		m4x4LightProjMatrix[3]							= GFSDK_Identity_Matrix;
		fBiasZ[0] = fBiasZ[1] = fBiasZ[2] = fBiasZ[3]	= 0.0f;
		fShadowIntensity								= 0.0f;
	}
};


/*===========================================================================
Shadow map rendering params
===========================================================================*/
struct GFSDK_ShadowLib_MapRenderParams
{
	// Function pointer to user defined function that renders the shadow map
	void						(*fnpDrawFunction)( void*, gfsdk_float4x4* );	
	// User defined data passed to the user supplied function pointer
	void*						pDrawFunctionParams;	
	gfsdk_float4x4				m4x4EyeViewMatrix;
	gfsdk_float4x4				m4x4EyeProjectionMatrix;
	// World space axis aligned bounding box that encapsulates the shadow map scene geometry
	gfsdk_float3				v3WorldSpaceBBox[2];	
	GFSDK_ShadowLib_LightDesc	LightDesc;	
	// Defines the eye space Z far value for each cascade
	gfsdk_F32					fCascadeZFarPercent[GFSDK_ShadowLib_ViewType_Cascades_4];	
	// passed to the hw through D3D11_RASTERIZER_DESC.DepthBias  
	gfsdk_S32					iDepthBias;	
	// passed to the hw through D3D11_RASTERIZER_DESC.SlopeScaledDepthBias
	gfsdk_F32					fSlopeScaledDepthBias;	

	// Defaults
	GFSDK_ShadowLib_MapRenderParams()
	{
		fnpDrawFunction								= NULL;
		pDrawFunctionParams							= NULL;
		m4x4EyeViewMatrix							= GFSDK_Identity_Matrix;
		m4x4EyeProjectionMatrix						= GFSDK_Identity_Matrix;
		v3WorldSpaceBBox[0] = v3WorldSpaceBBox[1]	= GFSDK_Zero_Vector3;
		fCascadeZFarPercent[0]						= 20.0f;
		fCascadeZFarPercent[1]						= 40.0f;
		fCascadeZFarPercent[2]						= 70.0f;
		fCascadeZFarPercent[3]						= 100.0f;
		iDepthBias									= 1000;
		fSlopeScaledDepthBias						= 8.0f;
	}
};


/*===========================================================================
PCSS penumbra params
===========================================================================*/
struct GFSDK_ShadowLib_PCSSPenumbraParams
{
	// The World space blocker depth value at which maximum penumbra will occur
	gfsdk_F32	fMaxThreshold;
	// The World space blocker depth value at which penumbra size will no longer grow
	gfsdk_F32	fMaxClamp;
	// The minimum penumbra size, as a percentage of light size - so that you do not end up with zero
	// filtering at blocker depth zero
	gfsdk_F32	fMinSizePercent;
	// The slope applied to weights of possion disc samples based upon 1-length	as blocker depth approaches zero,
	// this basically allows one to increase the fMinSizePercent, but shift sample weights towards the center
	// of the disc. 
	gfsdk_F32	fMinWeightExponent;
	// The percentage of penumbra size below which the fMinWeightExponent function is applied. This stops 
	// the entire shadow from being affected.
	gfsdk_F32	fMinWeightThresholdPercent;
	// The percentage of the blocker search and filter radius to dither by
	gfsdk_F32	fBlockerSearchDitherPercent;
	gfsdk_F32	fFilterDitherPercent;
	
	// Defaults
	GFSDK_ShadowLib_PCSSPenumbraParams()
	{
		fMaxThreshold				= 100.0f;
		fMaxClamp					= 1.0f;
		fMinSizePercent				= 1.0f;
		fMinWeightExponent			= 3.0f;
		fMinWeightThresholdPercent	= 10.0f;
		fBlockerSearchDitherPercent = 25.0f;
		fFilterDitherPercent		= 15.0f;
	}
};


/*===========================================================================
Shadow buffer rendering params
===========================================================================*/
struct GFSDK_ShadowLib_BufferRenderParams
{
	GFSDK_ShadowLib_TechniqueType		eTechniqueType;
	GFSDK_ShadowLib_QualityType			eQualityType;
	GFSDK_ShadowLib_MSAARenderMode		eMSAARenderMode;
	// Soft shadow test: Shadow = ( ( SceneZ - ShadowMapZ ) > ( SceneZ * fSoftShadowTestScale ) ) ? ( 0.0f ) : ( 1.0f );
	gfsdk_F32							fSoftShadowTestScale;	
	GFSDK_ShadowLib_DepthBufferDesc		DepthBufferDesc;
	GFSDK_ShadowLib_PCSSPenumbraParams	PCSSPenumbraParams;
	// Cascade border and blending controls
	gfsdk_F32							fCascadeBorderPercent;
	gfsdk_F32							fCascadeBlendPercent;
	// When non-zero this value is used to test the convergence of the shadow every 32 taps. If the shadow
	// value has converged to this tolerance, the shader will early out. Shadow areas of low frequency can 
	// therefore take advantage of this.
	gfsdk_F32							fConvergenceTestTolerance;

	// Defaults
	GFSDK_ShadowLib_BufferRenderParams()
	{
		eTechniqueType					= GFSDK_ShadowLib_TechniqueType_PCSS;
		eQualityType					= GFSDK_ShadowLib_QualityType_High;
		eMSAARenderMode					= GFSDK_ShadowLib_MSAARenderMode_BruteForce;
		fSoftShadowTestScale			= 0.001f;
		fCascadeBorderPercent			= 1.0f;
		fCascadeBlendPercent			= 3.0f;
		fConvergenceTestTolerance		= 0.000001f;
	}
};


/*===========================================================================
All of the temp resources required by this lib. If you wish for the lib to create
a resource for you then call the utility/dev mode function:
GFSDK_ShadowLib_DevModeCreateTexture2D(). Otherwise you may provide the resource 
directly yourself. In either case you are responsible for releasing the resource.
===========================================================================*/
struct GFSDK_ShadowLib_TempResources
{
	// Downsampled shadow map resources (REQUIRED IF: GFSDK_ShadowLib_MapDesc::bDownsample = true)
	// See comments in GFSDK_ShadowLib_MapDesc for usage model.
	GFSDK_ShadowLib_Texture2D*	pDownsampledShadowMap;
	
	// Defaults
	GFSDK_ShadowLib_TempResources()
	{
		pDownsampledShadowMap	= NULL;
	}
};


/*===========================================================================
GFSDK_ShadowLib interface
===========================================================================*/
class GFSDK_ShadowLib_Context
{
public:


/*===========================================================================
Call once on device destruction to release the lib.
All resources held by the context will be released/deleted including the context itself
===========================================================================*/
virtual GFSDK_ShadowLib_Status Destroy() = 0;


/*===========================================================================
Call this function to set the temp resources required by this lib, please see 
the GFSDK_ShadowLib_TempResources structure for details of which resources
are required and what API usage triggers their requirement.
===========================================================================*/
virtual GFSDK_ShadowLib_Status SetTempResources(	
	// IN: Structure providing access to the required temp resources 
	GFSDK_ShadowLib_TempResources* __GFSDK_RESTRICT__ const	pTempResources ) = 0;


/*===========================================================================
Creates a shadow map, based upon the descriptor, and 
adds it to an internal list of shadow maps. Returns a shadow map
handle to the caller.
===========================================================================*/
virtual GFSDK_ShadowLib_Status AddMap(	
	// IN: Describes the type of shadow map desired (see GFSDK_ShadowLib_MapDesc) 
  	const GFSDK_ShadowLib_MapDesc* __GFSDK_RESTRICT__ const	pShadowMapDesc,
	// OUT: A handle to the created shadow map
	GFSDK_ShadowLib_Map** __GFSDK_RESTRICT__ const			ppShadowMapHandle ) = 0;


/*===========================================================================
Removes the shadow map (defined by the provided handle) from the lib's 
internal list of shadow maps
===========================================================================*/
virtual GFSDK_ShadowLib_Status RemoveMap(	
	// IN/OUT: All resources held by the handle will be released/deleted including the handle itself
	GFSDK_ShadowLib_Map** __GFSDK_RESTRICT__ const		ppShadowMapHandle ) = 0;   


/*===========================================================================
Creates a shadow buffer, based upon the descriptor, and 
adds it to an internal list of shadow buffers. Returns a shadow buffer
handle to the caller.
===========================================================================*/
virtual GFSDK_ShadowLib_Status AddBuffer(	
	// IN: Describes the type of shadow buffer desired (see GFSDK_ShadowLib_BufferDesc) 
  	const GFSDK_ShadowLib_BufferDesc* __GFSDK_RESTRICT__ const	pShadowBufferDesc,
	// IN/OUT: A handle to the created shadow buffer
	GFSDK_ShadowLib_Buffer** __GFSDK_RESTRICT__ const			ppShadowBufferHandle ) = 0;


/*===========================================================================
Removes the shadow buffer (defined by the provided handle) from the lib's 
internal list of shadow buffers
===========================================================================*/
virtual GFSDK_ShadowLib_Status RemoveBuffer(	
	// IN/OUT: All resources held by the handle will be released/deleted including the handle itself
	GFSDK_ShadowLib_Buffer** __GFSDK_RESTRICT__ const	ppShadowBufferHandle ) = 0;   


/*===========================================================================
Renders the shadow map (defined by the provided handle), based upon the 
provided render params
===========================================================================*/
virtual GFSDK_ShadowLib_Status RenderMap(	
	// IN: Handle of the shadow map you wish to render to
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const					pShadowMapHandle,
	// IN: The render params instructing the lib how to render the map (see GFSDK_ShadowLib_MapRenderParams)
	const GFSDK_ShadowLib_MapRenderParams* __GFSDK_RESTRICT__ const	pShadowMapRenderParams ) = 0;


/*===========================================================================
Clears the specified shadow buffer
===========================================================================*/
virtual GFSDK_ShadowLib_Status ClearBuffer(	
	// IN: Handle of the shadow buffer you wish to clear
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const	pShadowBufferHandle ) = 0;


/*===========================================================================
Accumulates shadows in the specified buffer (with min blending), 
using the given technique on the given shadow map. This function may be 
called multiple times with different shadow maps, and techniques to 
accumulate shadowed regions of the screen.
===========================================================================*/
virtual GFSDK_ShadowLib_Status RenderBuffer(	
	// IN: Handle of the shadow map you wish to use
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const						pShadowMapHandle,
	// IN: Handle of the shadow buffer you wish to use
	GFSDK_ShadowLib_Buffer* __GFSDK_RESTRICT__ const					pShadowBufferHandle,
	// IN: The render params instructing the lib how to render the buffer (see GFSDK_ShadowLib_BufferRenderParams)
	const GFSDK_ShadowLib_BufferRenderParams* __GFSDK_RESTRICT__ const	pShadowBufferRenderParams ) = 0;


/*===========================================================================
Accumulates shadows in the specified buffer (with min blending), 
using the given technique on the given _external_ shadow map. This function may be 
called multiple times with different _external_ shadow maps, and techniques to 
accumulate shadowed regions of the screen.
===========================================================================*/
virtual GFSDK_ShadowLib_Status RenderBufferUsingExternalMap(	
	// IN: Details of the external shadow map required by the library (see GFSDK_ShadowLib_ExternalMapDesc)
	const GFSDK_ShadowLib_ExternalMapDesc* __GFSDK_RESTRICT__ const		pExternalShadowMapDesc,
	// IN: Used to pass in platform specific shadow map (see GFSDK_ShadowLib_Common.h)
	const GFSDK_ShadowLib_ShaderResourceView* __GFSDK_RESTRICT__ const	pShadowMapSRV,
	// IN: Handle of the shadow buffer you wish to use
	GFSDK_ShadowLib_Buffer* __GFSDK_RESTRICT__ const					pShadowBufferHandle,
	// IN: The render params instructing the lib how to render the buffer (see GFSDK_ShadowLib_BufferRenderParams)
	const GFSDK_ShadowLib_BufferRenderParams* __GFSDK_RESTRICT__ const	pShadowBufferRenderParams ) = 0;


/*===========================================================================
Once done with accumulating shadows in the buffer, call this function to 
finalize the accumulated result and get back the shadow buffer SRV
===========================================================================*/
virtual GFSDK_ShadowLib_Status FinalizeBuffer(	
	// IN: Handle of the shadow buffer you wish to finalize
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const				pShadowBufferHandle,
	// OUT: A platform specific shadow buffer texture - which can the be used to modulate with scene color
	GFSDK_ShadowLib_ShaderResourceView*	__GFSDK_RESTRICT__ const	pShadowBufferSRV ) = 0;


/*===========================================================================
Combines the finalized shadow buffer with the color render target provided, 
using the supplied parameters
===========================================================================*/
virtual GFSDK_ShadowLib_Status ModulateBuffer(	
	// IN: Handle of the shadow buffer you wish to modulate with scene color
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const					pShadowBufferHandle,
	// IN: Platform specific color texture to modulate with
	const GFSDK_ShadowLib_RenderTargetView* __GFSDK_RESTRICT__ const	pColorRTV,
	// IN: The color of the shadow to be used in the modulation
	gfsdk_float3														v3ShadowColor,
	// IN: The write mask to use during modulative blending 
	GFSDK_ShadowLib_ModulateBufferMask									eModulateMask ) = 0;


/*===========================================================================
Stereo fix up resource, NULL if in mono mode
===========================================================================*/
virtual GFSDK_ShadowLib_Status SetStereoFixUpResource(	
	// IN: A platform specific texture containing stereo fix up information (NULL if stereo is nopt required)
	const GFSDK_ShadowLib_ShaderResourceView* __GFSDK_RESTRICT__ const	pStereoFixUpSRV ) = 0;


/*===========================================================================
Development mode functions...
===========================================================================*/


/*===========================================================================
Helper function that creates a GFSDK_ShadowLib_Texture2D
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeCreateTexture2D(	
	GFSDK_ShadowLib_Texture2D* __GFSDK_RESTRICT__ const					pTexture2D ) = 0;


/*===========================================================================
Enable perf makers
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeEnablePerfMarkers(	
	gfsdk_bool											bEnable ) = 0;


/*===========================================================================
Display the shadow map
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeDisplayMap(	
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const					pShadowBufferHandle,
	const GFSDK_ShadowLib_RenderTargetView* __GFSDK_RESTRICT__ const	pRTV,
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const						pShadowMapHandle,
	gfsdk_U32															uMapID,
	// The origin is considered to be: DX = Top Left, GL = Bottom Left 
	gfsdk_U32															uPosX,
	gfsdk_U32															uPosY,
	gfsdk_F32															fScale ) = 0;


/*===========================================================================
Display the shadow map frustum
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeDisplayMapFrustum(	
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const					pShadowBufferHandle,
	const GFSDK_ShadowLib_RenderTargetView* __GFSDK_RESTRICT__ const	pRTV,
	const GFSDK_ShadowLib_DepthStencilView* __GFSDK_RESTRICT__ const	pDSV,
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const						pShadowMapHandle,
	gfsdk_U32															uMapID,
	gfsdk_float3														v3Color ) = 0;


/*===========================================================================
Display the shadow map
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeDisplayExternalMap(	
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const						pShadowBufferHandle,
	const GFSDK_ShadowLib_RenderTargetView* __GFSDK_RESTRICT__ const		pRTV,
	const GFSDK_ShadowLib_ExternalMapDesc* __GFSDK_RESTRICT__ const			pExternalShadowMapDesc,
	const GFSDK_ShadowLib_ShaderResourceView*	__GFSDK_RESTRICT__ const	pShadowMapSRV,
	gfsdk_U32																uMapID,
	// The origin is considered to be: DX = Top Left, GL = Bottom Left 
	gfsdk_U32																uPosX,
	gfsdk_U32																uPosY,
	gfsdk_F32																fScale ) = 0;


/*===========================================================================
Display the external shadow map frustum
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeDisplayExternalMapFrustum(	
	GFSDK_ShadowLib_Buffer* __GFSDK_RESTRICT__ const					pShadowBufferHandle,
	const GFSDK_ShadowLib_RenderTargetView* __GFSDK_RESTRICT__ const	pRTV,
	const GFSDK_ShadowLib_DepthStencilView* __GFSDK_RESTRICT__ const	pDSV,
	const GFSDK_ShadowLib_ExternalMapDesc* __GFSDK_RESTRICT__ const		pExternalShadowMapDesc,
	gfsdk_U32															uMapID,
	gfsdk_float3														v3Color ) = 0;


/*===========================================================================
Display the finalized shadow buffer
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeDisplayBuffer(	
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const					pShadowBufferHandle,
	const GFSDK_ShadowLib_RenderTargetView* __GFSDK_RESTRICT__ const	pRTV,
	gfsdk_float2														v2Scale,
	const GFSDK_ShadowLib_ScissorRect*	__GFSDK_RESTRICT__ const		pSR ) = 0;


/*===========================================================================
Render cascades into the shadow buffer
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeToggleDebugCascadeShader(	
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const	pShadowBufferHandle,
	gfsdk_bool											bUseDebugShader ) = 0;


/*===========================================================================
Render eye depth into the shadow buffer
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeToggleDebugEyeDepthShader(	
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const	pShadowBufferHandle,
	gfsdk_bool											bUseDebugShader ) = 0;


/*===========================================================================
Render eye view z into the shadow buffer
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeToggleDebugEyeViewZShader(	
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const	pShadowBufferHandle,
	gfsdk_bool											bUseDebugShader ) = 0;


/*===========================================================================
Get shadow map data, basically used to test the external shadow map path
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeGetMapData(	
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const					pShadowMapHandle,
	GFSDK_ShadowLib_ShaderResourceView*	__GFSDK_RESTRICT__ const	pShadowMapSRV,
	gfsdk_float4x4* __GFSDK_RESTRICT__ const						pLightViewMatrix,								
	gfsdk_float4x4*	__GFSDK_RESTRICT__ const						pLightProjMatrix ) = 0;

};


#ifdef __cplusplus
extern "C" {
#endif


/*===========================================================================
Use this function to get the version of the DLL being used,
you should compare this to the version numbers you see in the header
file. If there is a mismatch please contact devrel.
===========================================================================*/
__GFSDK_SHADOWLIB_EXTERN_INTERFACE__ GFSDK_ShadowLib_GetDLLVersion( 
	// OUT: The version information of the DLL being used
	GFSDK_ShadowLib_Version* __GFSDK_RESTRICT__ const	pVersion );


/*===========================================================================
Call once on device creation to initialize the lib
===========================================================================*/
__GFSDK_SHADOWLIB_EXTERN_INTERFACE__ GFSDK_ShadowLib_Create(	
	// IN: Pass in the version numbers defined in this header file (these will be checked against the DLL version)
	const GFSDK_ShadowLib_Version* __GFSDK_RESTRICT__ const			pVersion, 
	// OUT: The library context which will be used during all susequent calls into the library   
	GFSDK_ShadowLib_Context** __GFSDK_RESTRICT__ const				ppContext,
	// IN: Used to pass in platform specific graphics device/context (see GFSDK_ShadowLib_Common.h)   
	const GFSDK_ShadowLib_DeviceContext* __GFSDK_RESTRICT__ const	pPlatformDevice,					
	// IN: Optionally provide your own custom allocator for the library to use
	gfsdk_new_delete_t*												customAllocator = NULL );


#ifdef __cplusplus
}; //extern "C" {
#endif


/*===========================================================================
EOF
===========================================================================*/