# - Try to find Assimp
# - Sets ASSIMP_LIBRARIES - list of the libraries found
# - Sets ASSIMP_INCLUDE_DIRS 

include(FindPackageHandleStandardArgs)

# Find the includes

find_path(ASSIMP_PATH include/assimp/version.h
	PATHS 
	${NVCLOTH_ROOT_DIR}/samples/external/assimp-${Assimp_FIND_VERSION}
)

# If the project pulling in this dependency needs the static crt, then append that to the path.

#if (STATIC_WINCRT)
#	SET(ASSIMP_CRT_FOLDER "staticcrt")
#else()
#	SET(ASSIMP_CRT_FOLDER "dynamiccrt")
#endif()

if (CMAKE_CL_64)
	SET(ASSIMP_ARCH_FOLDER "x64")
else()
	SET(ASSIMP_ARCH_FOLDER "Win32")
endif()

if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 18.0.0.0 AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0.0.0)
	#SET(VS_STR "vc13win64-cmake")
	MESSAGE(FATAL_ERROR "Failed to find compatible ASSIMP - Only supporting VS2015 currently")
elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.0.0.0)
	if (CMAKE_CL_64)
		SET(VS_STR "vc14win64-cmake")
	else()
		SET(VS_STR "vc14win32-cmake")
	endif()
else()
	MESSAGE(FATAL_ERROR "Failed to find compatible ASSIMP - Only supporting VS2015 currently")
endif()


SET(LIB_PATH ${ASSIMP_PATH}/compiler/${VS_STR}/code)

find_library(ASSIMP_LIB
	NAMES assimp-vc140-mt.lib
	PATHS ${LIB_PATH}/Release
)
find_library(ASSIMP_LIB_DEBUG
	NAMES assimp-vc140-mt.lib
	PATHS ${LIB_PATH}/Debug
)

#MESSAGE(${LIB_PATH}/Release)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ASSIMP
	DEFAULT_MSG
	ASSIMP_PATH
	
	ASSIMP_LIB
	
	ASSIMP_LIB_DEBUG
)

if (ASSIMP_FOUND)
	# NOTE: This include list is way too long and reaches into too many internals.
	# Also may not be good enough for all users.
	SET(ASSIMP_INCLUDE_DIRS 
		${ASSIMP_PATH}/include 
	)
	
	SET(ASSIMP_DLL_PATH
		${LIB_PATH}
	)

	SET(ASSIMP_LIBRARIES optimized ${ASSIMP_LIB} debug ${ASSIMP_LIB_DEBUG})
endif()
