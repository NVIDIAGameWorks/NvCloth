#
# Build SampleBase Windows
#

FIND_PACKAGE(CUDA 8 REQUIRED)

SET(SAMPLEBASE_PLATFORM_COMMON_FILES
)

SET(SAMPLEBASE_PLATFORM_INCLUDES ${CUDA_INCLUDE_DIRS}
)

SET(SAMPLEBASE_COMPILE_DEFS
	# Common to all configurations
	${SAMPLES_SLN_COMPILE_DEFS}
	
	$<$<CONFIG:debug>:${SAMPLES_SLN_DEBUG_COMPILE_DEFS}>
	$<$<CONFIG:checked>:${SAMPLES_SLN_CHECKED_COMPILE_DEFS}>
	$<$<CONFIG:profile>:${SAMPLES_SLN_PROFILE_COMPILE_DEFS}>
	$<$<CONFIG:release>:${SAMPLES_SLN_RELEASE_COMPILE_DEFS}>
)

SET(SAMPLEBASE_LINK_FLAGS "/SUBSYSTEM:WINDOWS")

SET(SAMPLEBASE_ADDITIONAL_DLLS "")

#TODO: Move this to an external dep
if (CMAKE_CL_64)
	SET(SHADOW_LIB ${NVCLOTH_ROOT_DIR}/samples/external/shadow_lib/GFSDK_ShadowLib_DX11.win64.lib)
	SET(SHADOW_DLLS ${NVCLOTH_ROOT_DIR}/samples/external/shadow_lib/GFSDK_ShadowLib_DX11.win64.dll)

	SET(HBAO_LIB ${NVCLOTH_ROOT_DIR}/samples/external/hbao/GFSDK_SSAO_D3D11.win64.lib)
	SET(HBAO_DLLS ${NVCLOTH_ROOT_DIR}/samples/external/hbao/GFSDK_SSAO_D3D11.win64.dll)

	SET(D3DCOMPILER_DLL "\"$(VC_ExecutablePath_x64_x64)/d3dcompiler_47.dll\"")
else()
	SET(SHADOW_LIB ${NVCLOTH_ROOT_DIR}/samples/external/shadow_lib/GFSDK_ShadowLib_DX11.win32.lib)
	SET(SHADOW_DLLS ${NVCLOTH_ROOT_DIR}/samples/external/shadow_lib/GFSDK_ShadowLib_DX11.win32.dll)
	
	SET(HBAO_LIB ${NVCLOTH_ROOT_DIR}/samples/external/hbao/GFSDK_SSAO_D3D11.win32.lib)
	SET(HBAO_DLLS ${NVCLOTH_ROOT_DIR}/samples/external/hbao/GFSDK_SSAO_D3D11.win32.dll)

	SET(D3DCOMPILER_DLL "\"$(VC_ExecutablePath_x86_x86)/d3dcompiler_47.dll\"")
endif()

