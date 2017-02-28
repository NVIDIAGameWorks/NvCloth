# - Try to find DirectXTex SDK
# - Sets DIRECTXTEX_LIBRARIES - list of the libraries found
# - Sets DIRECTXTEX_INCLUDE_DIRS 

include(FindPackageHandleStandardArgs)

# Find the includes

# TODO: Do the version stuff properly!
find_path(DIRECTXTEXSDK_PATH include/DirectXTex.h
	PATHS ${GW_DEPS_ROOT}/DirectXTex/${DirectXTex_FIND_VERSION}
	${NVCLOTH_ROOT_DIR}/samples/external/DirectXTex/${DirectXTex_FIND_VERSION}
)

# If the project pulling in this dependency needs the static crt, then append that to the path.

if (STATIC_WINCRT)
	SET(DIRECTXTEXSDK_CRT_FOLDER "staticcrt")
else()
	SET(DIRECTXTEXSDK_CRT_FOLDER "dynamiccrt")
endif()

if (CMAKE_CL_64)
	SET(DIRECTXTEXSDK_ARCH_FOLDER "x64")
else()
	SET(DIRECTXTEXSDK_ARCH_FOLDER "Win32")
endif()

if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 18.0.0.0 AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0.0.0)
	SET(VS_STR "vs2013")
elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.0.0.0)
	SET(VS_STR "vs2015")
else()
	MESSAGE(FATAL_ERROR "Failed to find compatible FBXSDK - Only supporting VS2013 and VS2015")
endif()


SET(LIB_PATH ${DIRECTXTEXSDK_PATH}/bin/${DIRECTXTEXSDK_CRT_FOLDER}/${VS_STR}/${DIRECTXTEXSDK_ARCH_FOLDER}/)

find_library(DIRECTXTEX_LIB
	NAMES DirectXTex
	PATHS ${LIB_PATH}/Release
)
find_library(DIRECTXTEX_LIB_DEBUG
	NAMES DirectXTex
	PATHS ${LIB_PATH}/Debug
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(DIRECTXTEXSDK
	DEFAULT_MSG
	DIRECTXTEXSDK_PATH
	
	DIRECTXTEX_LIB
	
	DIRECTXTEX_LIB_DEBUG
)

if (DIRECTXTEXSDK_FOUND)
	# NOTE: This include list is way too long and reaches into too many internals.
	# Also may not be good enough for all users.
	SET(DIRECTXTEX_INCLUDE_DIRS 
		${DIRECTXTEXSDK_PATH}/include 
	)

	SET(DIRECTXTEX_LIBRARIES optimized ${DIRECTXTEX_LIB} debug ${DIRECTXTEX_LIB_DEBUG})
endif()
