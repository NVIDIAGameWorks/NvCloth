# - Try to find DXUT SDK
# - Sets DXUT_LIBRARIES - list of the libraries found
# - Sets DXUT_INCLUDE_DIRS 

include(FindPackageHandleStandardArgs)

# Find the includes

# TODO: Do the version stuff properly!
find_path(DXUTSDK_PATH Core/DXUT.h
	PATHS ${GW_DEPS_ROOT}/DXUT/${DXUT_FIND_VERSION}
	${NVCLOTH_ROOT_DIR}/samples/external/DXUT/${DXUT_FIND_VERSION}
)

# If the project pulling in this dependency needs the static crt, then append that to the path.

if (STATIC_WINCRT)
	SET(DXUTSDK_CRT_FOLDER "staticcrt")
else()
	SET(DXUTSDK_CRT_FOLDER "dynamiccrt")
endif()

if (CMAKE_CL_64)
	SET(DXUTSDK_ARCH_FOLDER "x64")
else()
	SET(DXUTSDK_ARCH_FOLDER "Win32")
endif()

if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 18.0.0.0 AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0.0.0)
	SET(VS_STR "vs2013")
elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.0.0.0)
	SET(VS_STR "vs2015")
else()
	MESSAGE(FATAL_ERROR "Failed to find compatible FBXSDK - Only supporting VS2013 and VS2015")
endif()


SET(CORE_LIB_PATH ${DXUTSDK_PATH}/Core/Bin/${DXUTSDK_CRT_FOLDER}/${VS_STR}/${DXUTSDK_ARCH_FOLDER}/)
SET(OPT_LIB_PATH ${DXUTSDK_PATH}/Optional/Bin/${DXUTSDK_CRT_FOLDER}/${VS_STR}/${DXUTSDK_ARCH_FOLDER}/)

find_library(DXUT_LIB
	NAMES DXUT
	PATHS ${CORE_LIB_PATH}/Release
)
find_library(DXUT_LIB_DEBUG
	NAMES DXUT
	PATHS ${CORE_LIB_PATH}/Debug
)

find_library(DXUTOPT_LIB
	NAMES DXUTOpt
	PATHS ${OPT_LIB_PATH}/Release
)

find_library(DXUTOPT_LIB_DEBUG
	NAMES DXUTOpt
	PATHS ${OPT_LIB_PATH}/Debug
)

MESSAGE("DXUT: ${DXUT_LIB}")

FIND_PACKAGE_HANDLE_STANDARD_ARGS(DXUTSDK
	DEFAULT_MSG
	DXUTSDK_PATH
	
	DXUT_LIB
	DXUTOPT_LIB
	
	DXUT_LIB_DEBUG
	DXUTOPT_LIB_DEBUG
)

if (DXUTSDK_FOUND)
	# NOTE: This include list is way too long and reaches into too many internals.
	# Also may not be good enough for all users.
	SET(DXUT_INCLUDE_DIRS 
		${DXUTSDK_PATH}/Core
		${DXUTSDK_PATH}/Optional
	)

	SET(DXUT_LIBRARIES optimized ${DXUT_LIB} debug ${DXUT_LIB_DEBUG} optimized ${DXUTOPT_LIB} debug ${DXUTOPT_LIB_DEBUG})
endif()
