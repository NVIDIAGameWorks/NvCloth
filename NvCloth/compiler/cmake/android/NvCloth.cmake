#
# Build NvCloth
#

SET(GW_DEPS_ROOT $ENV{GW_DEPS_ROOT})

SET(NVCLOTH_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../)

if(${ANDROID_ABI} STREQUAL "armeabi-v7a with NEON" OR ${ANDROID_ABI} STREQUAL "arm64-v8a")
	SET(NVCLOTH_PLATFORM_NEON_FILES
		${PROJECT_ROOT_DIR}/src/neon/NeonCollision.cpp
		${PROJECT_ROOT_DIR}/src/neon/NeonSelfCollision.cpp
		${PROJECT_ROOT_DIR}/src/neon/NeonSolverKernel.cpp
		${PROJECT_ROOT_DIR}/src/neon/SwCollisionHelpers.h
	)
else()
	SET(NVCLOTH_PLATFORM_NEON_FILES
	)
endif()

SET(NVCLOTH_PLATFORM_SOURCE_FILES
	${PROJECT_ROOT_DIR}/src/ps/unix/PsUnixAtomic.cpp
	${PROJECT_ROOT_DIR}/src/ps/unix/PsUnixFPU.h
	${PROJECT_ROOT_DIR}/src/ps/android/cpu-features.c
	${PROJECT_ROOT_DIR}/src/ps/android/cpu-features.h

	${NVCLOTH_PLATFORM_NEON_FILES}
)

SET(NVCLOTH_PLATFORM_INCLUDES
	${LL_SOURCE_DIR}/include/linux
	${ANDROID_NDK}/sources/android/cpufeatures
)

IF(PX_STATIC_LIBRARIES)
	SET(NVCLOTH_API_COMPILE_DEFS NV_CLOTH_IMPORT=;PX_CALL_CONV=;)
ELSE()
	SET(NVCLOTH_API_COMPILE_DEFS NV_CLOTH_IMPORT=PX_DLL_EXPORT;)
ENDIF()

# Use generator expressions to set config specific preprocessor definitions
SET(NVCLOTH_COMPILE_DEFS 
	${NVCLOTH_ANDROID_COMPILE_DEFS};
	${NVCLOTH_API_COMPILE_DEFS}
	
	NV_ANDROID
	NV_CLOTH_ENABLE_DX11=0
	NV_CLOTH_ENABLE_CUDA=0

	$<$<CONFIG:debug>:${NVCLOTH_ANDROID_DEBUG_COMPILE_DEFS};>
	$<$<CONFIG:checked>:${NVCLOTH_ANDROID_CHECKED_COMPILE_DEFS};>
	$<$<CONFIG:profile>:${NVCLOTH_ANDROID_PROFILE_COMPILE_DEFS};>
	$<$<CONFIG:release>:${NVCLOTH_ANDROID_RELEASE_COMPILE_DEFS};>
)

IF(PX_STATIC_LIBRARIES)
	SET(NVCLOTH_LIBTYPE STATIC)
ELSE()
	SET(NVCLOTH_LIBTYPE SHARED)
ENDIF()

# include common low level settings
INCLUDE(../common/NvCloth.cmake)
