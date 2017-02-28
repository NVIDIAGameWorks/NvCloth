#Platform specific compile flags and project includes

SET(GW_DEPS_ROOT $ENV{GW_DEPS_ROOT})

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}; $ENV{GW_DEPS_ROOT}/sw/physx/tools/CMakeModules)

IF(EXISTS $ENV{GW_DEPS_ROOT}/Externals/CMakeModules)
	SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}; $ENV{GW_DEPS_ROOT}/Externals/CMakeModules)
ENDIF()


IF(EXISTS $ENV{GW_DEPS_ROOT}/Externals/CMakeModules)
	SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}; $ENV{GW_DEPS_ROOT}/Externals/CMakeModules)
ENDIF()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}; "${CMAKE_CURRENT_SOURCE_DIR}/..")
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}; "${CMAKE_CURRENT_SOURCE_DIR}/external/CMakeModules")
MESSAGE("CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})

#NOTE: Warnings lowered on the sample projects as it's got a lot of warnings. Defines below hide more.
SET(CMAKE_CXX_FLAGS "/GR- /GF /MP /Gy /EHsc /d2Zi+ /errorReport:prompt /fp:fast /Gd /Gm- /GS- /nologo /W3 /WX /Zc:forScope /Zc:inline /Zc:wchar_t /Zi")

# Are we using the static or dynamic RT library? Whatever we use, it needs to be the same in any dependencies
#  we pull in or we're potentially having mismatch issues.
IF(STATIC_WINCRT)
	SET(WINCRT_NDEBUG "/MT")
	SET(WINCRT_DEBUG "/MTd")
ELSE()
	SET(WINCRT_NDEBUG "/MD")
	SET(WINCRT_DEBUG "/MDd")
ENDIF()

SET(CMAKE_CXX_FLAGS_DEBUG "/Od /RTCsu ${WINCRT_DEBUG}")
SET(CMAKE_CXX_FLAGS_CHECKED "/Ox ${WINCRT_NDEBUG}")
SET(CMAKE_CXX_FLAGS_PROFILE "/Ox ${WINCRT_NDEBUG}")
SET(CMAKE_CXX_FLAGS_RELEASE "/Ox ${WINCRT_NDEBUG}")

# Build PDBs for all configurations
SET(CMAKE_SHARED_LINKER_FLAGS "/DEBUG")

IF(CMAKE_CL_64)
	ADD_DEFINITIONS(-DWIN64)
ENDIF(CMAKE_CL_64)				

SET(SAMPLES_SLN_COMPILE_DEFS _UNICODE;UNICODE;WIN32;WIN64;_CRT_SECURE_NO_DEPRECATE;_CRT_NONSTDC_NO_DEPRECATE;_ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH;_ALLOW_RUNTIME_LIBRARY_MISMATCH;__GFSDK_DX11__;)
#NvBlastExt doesn't have the _CONSOLE flag

SET(SAMPLES_SLN_DEBUG_COMPILE_DEFS _DEBUG;NV_DEBUG=1;)
SET(SAMPLES_SLN_CHECKED_COMPILE_DEFS NDEBUG;NV_CHECKED=1;)
SET(SAMPLES_SLN_PROFILE_COMPILE_DEFS NDEBUG;NV_PROFILE=1;)
SET(SAMPLES_SLN_RELEASE_COMPILE_DEFS NDEBUG;)

IF(CMAKE_CL_64)
	SET(LIBPATH_SUFFIX "x64")
ELSE(CMAKE_CL_64)
	SET(LIBPATH_SUFFIX "x86")
ENDIF(CMAKE_CL_64)				

SET(CMAKE_DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}_${LIBPATH_SUFFIX}")
SET(CMAKE_PROFILE_POSTFIX "${CMAKE_PROFILE_POSTFIX}_${LIBPATH_SUFFIX}")
SET(CMAKE_CHECKED_POSTFIX "${CMAKE_CHECKED_POSTFIX}_${LIBPATH_SUFFIX}")
SET(CMAKE_RELEASE_POSTFIX "${CMAKE_RELEASE_POSTFIX}_${LIBPATH_SUFFIX}")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}; "${CMAKE_CURRENT_SOURCE_DIR}/../../../..")

SET(PX_SELECT_COMPONENTS PxFoundation)
FIND_PACKAGE(PxShared REQUIRED)
FIND_PACKAGE(NvCloth REQUIRED)

# Include the PxShared-exports.cmake file that PxShared created. There's other ways to do this, but there's a bunch of issues with it in a CI environment
#  such as - we don't want to be installing any built artifacts on this server, etc. So for now we hack it.

# Add PxShared as a dependency so that we can use project references
#ADD_SUBDIRECTORY(${PXSHARED_ROOT_DIR}/src/compiler/cmake/windows "${CMAKE_CURRENT_BINARY_DIR}/pxshared_bin")
#MESSAGE("NVCLOTH_ROOT_DIR = ${NVCLOTH_ROOT_DIR}")
ADD_SUBDIRECTORY(${NVCLOTH_ROOT_DIR}/compiler/cmake/windows "${CMAKE_CURRENT_BINARY_DIR}/NvCloth_bin")


# Include all of the projects
INCLUDE(${PROJECT_CMAKE_FILES_DIR}/SampleBase.cmake)


