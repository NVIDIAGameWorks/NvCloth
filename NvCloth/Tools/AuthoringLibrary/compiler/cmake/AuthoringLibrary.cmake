#
# Build AUTHORINGLIBRARY Common
#

MESSAGE("AuthoringLibrary.cmake [begin]")

MESSAGE("CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})

#SET(SB_SOURCE_DIR ${PROJECT_SOURCE_DIR}/..)


FIND_PACKAGE(PxShared "1.0.21467209.1" REQUIRED)


# Include here after the directories are defined so that the platform specific file can use the variables.
include(${PROJECT_CMAKE_FILES_DIR}/${TARGET_BUILD_PLATFORM}/AuthoringLibrary.cmake)


SET(HEADERS
	${PROJECT_SOURCE_DIR}/include/NvClothAuthoringLibrary/Parameters.h
	${PROJECT_SOURCE_DIR}/include/NvClothAuthoringLibrary/DebugVisualization.h
	${PROJECT_SOURCE_DIR}/include/NvClothAuthoringLibrary/CollisionVisualization.h
)

SET(SOURCE
	${PROJECT_SOURCE_DIR}/src/Parameters.cpp
	${PROJECT_SOURCE_DIR}/src/DebugVisualization.cpp
	${PROJECT_SOURCE_DIR}/src/CollisionVisualization.cpp
)

ADD_LIBRARY(NvClothAuthoringLibrary
	${HEADERS}
	${SOURCE}
)

SOURCE_GROUP("include\\NvClothAuthoringLibrary" FILES ${HEADERS})
SOURCE_GROUP("src" FILES ${SOURCE})


# Target specific compile options

TARGET_INCLUDE_DIRECTORIES(NvClothAuthoringLibrary 
	PRIVATE ${AUTHORINGLIBRARY_PLATFORM_INCLUDES}
	
	PRIVATE ${PXSHARED_ROOT_DIR}/include/foundation
	PRIVATE ${PXSHARED_ROOT_DIR}/include
	PRIVATE ${PXSHARED_ROOT_DIR}/src/foundation/include
	
	PRIVATE ${NVCLOTH_ROOT_DIR}/include
	PRIVATE ${NVCLOTH_ROOT_DIR}/extensions/include
	
	PRIVATE ${PROJECT_SOURCE_DIR}/include
)

TARGET_COMPILE_DEFINITIONS(NvClothAuthoringLibrary
	PRIVATE ${AUTHORINGLIBRARY_COMPILE_DEFS}
)

SET_TARGET_PROPERTIES(NvClothAuthoringLibrary PROPERTIES 
	COMPILE_PDB_NAME_DEBUG "NvClothAuthoringLibrary${CMAKE_DEBUG_POSTFIX}"
	COMPILE_PDB_NAME_CHECKED "NvClothAuthoringLibrary${CMAKE_CHECKED_POSTFIX}"
	COMPILE_PDB_NAME_PROFILE "NvClothAuthoringLibrary${CMAKE_PROFILE_POSTFIX}"
	COMPILE_PDB_NAME_RELEASE "NvClothAuthoringLibrary${CMAKE_RELEASE_POSTFIX}"
)

TARGET_COMPILE_OPTIONS(NvClothAuthoringLibrary PRIVATE /wd4005 /wd4244)
	
TARGET_LINK_LIBRARIES(NvClothAuthoringLibrary PUBLIC NvCloth)

SET_TARGET_PROPERTIES(NvClothAuthoringLibrary PROPERTIES LINK_FLAGS ${AUTHORINGLIBRARY_LINK_FLAGS})

MESSAGE("AUTHORINGLIBRARY.cmake [end]")