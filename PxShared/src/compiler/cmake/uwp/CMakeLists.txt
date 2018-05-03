
SET(CMAKE_CXX_FLAGS "/Wall /wd4514 /wd4820 /wd4127 /wd4710 /wd4711 /wd4577 /wd4530 /d2Zi+ /WX /W4 /ZW /GF /GS- /GR- /Gd /fp:fast ${DISABLE_ITERATOR_DEBUGGING}")

SET(CMAKE_CXX_FLAGS_DEBUG   "/Od ${WINCRT_NDEBUG} /Zi")
SET(CMAKE_CXX_FLAGS_CHECKED "/Ox ${WINCRT_NDEBUG} /Zi")
SET(CMAKE_CXX_FLAGS_PROFILE "/Ox ${WINCRT_NDEBUG} /Zi")
SET(CMAKE_CXX_FLAGS_RELEASE "/Ox ${WINCRT_NDEBUG} /Zi")

# Build PDBs for all configurations
SET(CMAKE_SHARED_LINKER_FLAGS "/DEBUG")

# Controls PX_NVTX for all projects on windows
SET(PXSHARED_UWP_ENABLE_NVTX 0)	

# Disable cuda and dx for all projects on windows
SET(PXSHARED_UWP_COMPILE_DEFS WIN32;_CRT_SECURE_NO_DEPRECATE;_CRT_NONSTDC_NO_DEPRECATE;_WINSOCK_DEPRECATED_NO_WARNINGS;DISABLE_CUDA_PHYSX;)

SET(PXSHARED_UWP_DEBUG_COMPILE_DEFS   PX_DEBUG=1;PX_CHECKED=1;PX_NVTX=${PXSHARED_UWP_ENABLE_NVTX})
SET(PXSHARED_UWP_CHECKED_COMPILE_DEFS PX_CHECKED=1;PX_NVTX=${PXSHARED_UWP_ENABLE_NVTX})
SET(PXSHARED_UWP_PROFILE_COMPILE_DEFS PX_PROFILE=1;PX_NVTX=${PXSHARED_UWP_ENABLE_NVTX})
SET(PXSHARED_UWP_RELEASE_COMPILE_DEFS )

IF(PX_SCALAR_MATH)
	ADD_DEFINITIONS(-DPX_SIMD_DISABLED)
ENDIF()

IF(CMAKE_CL_64)
	ADD_DEFINITIONS(-DWIN64)
ENDIF(CMAKE_CL_64)		


# Include project cmake files here
IF(DEFINED PX_SELECT_COMPONENTS)
	if ("PxFoundation" IN_LIST PX_SELECT_COMPONENTS)
		INCLUDE(PxFoundation.cmake)
	endif()
ELSE()
	INCLUDE(PxFoundation.cmake)

	INCLUDE(CMakePackageConfigHelpers)

	configure_file(templates/ProjectVersion.cmake.in ${PXSHARED_ROOT_DIR}/pxshared-config-version.cmake @ONLY)

#	WRITE_BASIC_PACKAGE_VERSION_FILE(${PXSHARED_ROOT_DIR}/pxshared-config-version.cmake VERSION ${PXSHARED_VERSION} COMPATIBILITY ExactVersion)

	SET(PXSHARED_LIBS PxFoundation)
		
	# PX_ROOT_LIB_DIR is set by NvidiaBuildOptions and put into the cache. It's a relative path to the lib dir without config

	install(
		TARGETS ${PXSHARED_LIBS}
		EXPORT PxShared
		DESTINATION $<$<CONFIG:debug>:${PX_ROOT_LIB_DIR}/debug>$<$<CONFIG:release>:${PX_ROOT_LIB_DIR}/release>$<$<CONFIG:checked>:${PX_ROOT_LIB_DIR}/checked>$<$<CONFIG:profile>:${PX_ROOT_LIB_DIR}/profile>	)

	install(EXPORT PxShared FILE pxshared-config.cmake DESTINATION cmake)
	
	install(FILES ${PXSHARED_ROOT_DIR}/pxshared-config-version.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake)
	
	# This doesn't use libpath-suffix, as it won't have the "wrapper" top
	export(EXPORT PxShared FILE ${PXSHARED_ROOT_DIR}/pxshared-config.cmake)
	
	
ENDIF()

