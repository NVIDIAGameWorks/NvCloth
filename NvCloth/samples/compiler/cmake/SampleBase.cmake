#
# Build SampleBase Common
#

MESSAGE("SampleBase.cmake [begin]")

MESSAGE("CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})

SET(SB_SOURCE_DIR ${PROJECT_SOURCE_DIR}/SampleBase)
SET(SB_CORE_SOURCE_DIR ${SB_SOURCE_DIR}/core)
SET(SB_RENDERER_SOURCE_DIR ${SB_SOURCE_DIR}/renderer)
SET(SB_SCENE_SOURCE_DIR ${SB_SOURCE_DIR}/scene)
SET(SB_UI_SOURCE_DIR ${SB_SOURCE_DIR}/ui)
SET(SB_UTILS_SOURCE_DIR ${SB_SOURCE_DIR}/utils)

FIND_PACKAGE(PxShared "1.0.21467209.1" REQUIRED)
FIND_PACKAGE(DXUT "9.15.2016.1" REQUIRED)
FIND_PACKAGE(DirectXTex "10.5.2016.2" REQUIRED)
FIND_PACKAGE(imgui "1.49" REQUIRED)
#FIND_PACKAGE(tinyObjLoader $ENV{PM_tinyObjLoader_VERSION} REQUIRED)
#FIND_PACKAGE(tclap $ENV{PM_tclap_VERSION} REQUIRED)

# Include here after the directories are defined so that the platform specific file can use the variables.
include(${PROJECT_CMAKE_FILES_DIR}/${TARGET_BUILD_PLATFORM}/SampleBase.cmake)

SET(CORE_FILES
	${SB_CORE_SOURCE_DIR}/Application.cpp
	${SB_CORE_SOURCE_DIR}/Application.h
	${SB_CORE_SOURCE_DIR}/DeviceManager.cpp
	${SB_CORE_SOURCE_DIR}/DeviceManager.h
	${SB_CORE_SOURCE_DIR}/SampleController.cpp
	${SB_CORE_SOURCE_DIR}/SampleController.h
	${SB_CORE_SOURCE_DIR}/SampleManager.cpp
	${SB_CORE_SOURCE_DIR}/SampleManager.h
)

SET(RENDERER_FILES
	${SB_RENDERER_SOURCE_DIR}/ConvexRenderMesh.cpp
	${SB_RENDERER_SOURCE_DIR}/ConvexRenderMesh.h
	${SB_RENDERER_SOURCE_DIR}/CustomRenderMesh.cpp
	${SB_RENDERER_SOURCE_DIR}/CustomRenderMesh.h
	${SB_RENDERER_SOURCE_DIR}/ClothRenderMesh.h
	${SB_RENDERER_SOURCE_DIR}/ClothRenderMesh.cpp
	${SB_RENDERER_SOURCE_DIR}/DebugRenderBuffer.h
	${SB_RENDERER_SOURCE_DIR}/Mesh.cpp
	${SB_RENDERER_SOURCE_DIR}/Mesh.h
	${SB_RENDERER_SOURCE_DIR}/PrimitiveRenderMesh.cpp
	${SB_RENDERER_SOURCE_DIR}/PrimitiveRenderMesh.h
	${SB_RENDERER_SOURCE_DIR}/PxPhysXCommonConfig.h
	${SB_RENDERER_SOURCE_DIR}/PxRenderBuffer.h
	${SB_RENDERER_SOURCE_DIR}/Renderable.cpp
	${SB_RENDERER_SOURCE_DIR}/Renderable.h
	${SB_RENDERER_SOURCE_DIR}/Renderer.cpp
	${SB_RENDERER_SOURCE_DIR}/Renderer.h
	${SB_RENDERER_SOURCE_DIR}/RendererHBAO.cpp
	${SB_RENDERER_SOURCE_DIR}/RendererHBAO.h
	${SB_RENDERER_SOURCE_DIR}/RendererShadow.cpp
	${SB_RENDERER_SOURCE_DIR}/RendererShadow.h
	${SB_RENDERER_SOURCE_DIR}/RenderMaterial.cpp
	${SB_RENDERER_SOURCE_DIR}/RenderMaterial.h
	${SB_RENDERER_SOURCE_DIR}/RenderUtils.h
	${SB_RENDERER_SOURCE_DIR}/ResourceManager.cpp
	${SB_RENDERER_SOURCE_DIR}/ResourceManager.h
	${SB_RENDERER_SOURCE_DIR}/ShaderUtils.h
	${SB_RENDERER_SOURCE_DIR}/SkinnedRenderMesh.cpp
	${SB_RENDERER_SOURCE_DIR}/SkinnedRenderMesh.h
)

SET(SCENE_FILES
	${SB_SCENE_SOURCE_DIR}/SceneController.cpp
	${SB_SCENE_SOURCE_DIR}/SceneController.h
	${SB_SCENE_SOURCE_DIR}/Scene.cpp
	${SB_SCENE_SOURCE_DIR}/Scene.h
)

SET(SCENES_FILES
	${SB_SCENE_SOURCE_DIR}/scenes/SimpleScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/SimpleScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/WindScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/WindScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/TetherScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/TetherScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/FrictionScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/FrictionScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/FreeFallScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/FreeFallScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/SphereScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/SphereScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/StiffnessPerConstraintScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/StiffnessPerConstraintScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/GeodesicScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/GeodesicScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/LocalGlobalScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/LocalGlobalScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/DistanceConstraintScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/DistanceConstraintScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/InterCollisionScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/InterCollisionScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/SelfCollisionScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/SelfCollisionScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/TriangleScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/TriangleScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/ConvexCollisionScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/ConvexCollisionScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/PlaneCollisionScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/PlaneCollisionScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/CapsuleScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/CapsuleScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/CCDScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/CCDScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/MultiSolverScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/MultiSolverScene.h
	${SB_SCENE_SOURCE_DIR}/scenes/ScaledScene.cpp
	${SB_SCENE_SOURCE_DIR}/scenes/ScaledScene.h
)

SET(UI_FILES
	${SB_UI_SOURCE_DIR}/CommonUIController.cpp
	${SB_UI_SOURCE_DIR}/CommonUIController.h
	${SB_UI_SOURCE_DIR}/imgui_impl_dx11.cpp
	${SB_UI_SOURCE_DIR}/imgui_impl_dx11.h
)

SET(UTIL_FILES
	${SB_UTILS_SOURCE_DIR}/SampleProfiler.cpp
	${SB_UTILS_SOURCE_DIR}/SampleProfiler.h
	${SB_UTILS_SOURCE_DIR}/SampleTime.h
	${SB_UTILS_SOURCE_DIR}/UIHelpers.h
	${SB_UTILS_SOURCE_DIR}/Utils.cpp
	${SB_UTILS_SOURCE_DIR}/Utils.h
	${SB_UTILS_SOURCE_DIR}/ClothMeshGenerator.cpp
	${SB_UTILS_SOURCE_DIR}/ClothMeshGenerator.h
	${SB_UTILS_SOURCE_DIR}/CallbackImplementations.cpp
	${SB_UTILS_SOURCE_DIR}/CallbackImplementations.h
	${SB_UTILS_SOURCE_DIR}/JobManager.cpp
	${SB_UTILS_SOURCE_DIR}/JobManager.h
	${SB_UTILS_SOURCE_DIR}/DebugLineRenderBuffer.cpp
	${SB_UTILS_SOURCE_DIR}/DebugLineRenderBuffer.h
	${SB_UTILS_SOURCE_DIR}/MeshGenerator.cpp
	${SB_UTILS_SOURCE_DIR}/MeshGenerator.h
	
)

SET(ROOT_FILES
	${SB_SOURCE_DIR}/Main.cpp
	${SB_SOURCE_DIR}/Sample.h
)

ADD_EXECUTABLE(SampleBase
	${CORE_FILES}
	${RENDERER_FILES}
	${SCENE_FILES}
	${SCENES_FILES}
	
	${UI_FILES}
	${UTIL_FILES}
	${ROOT_FILES}
	
	
	${IMGUI_SOURCE_FILES}
)

SOURCE_GROUP("Source" FILES ${ROOT_FILES})

SOURCE_GROUP("Source\\core" FILES ${CORE_FILES})
SOURCE_GROUP("Source\\imgui" FILES ${IMGUI_SOURCE_FILES})
SOURCE_GROUP("Source\\renderer" FILES ${RENDERER_FILES})
SOURCE_GROUP("Source\\scene" FILES ${SCENE_FILES})
SOURCE_GROUP("Source\\scene\\scenes" FILES ${SCENES_FILES})
SOURCE_GROUP("Source\\ui" FILES ${UI_FILES})
SOURCE_GROUP("Source\\utils" FILES ${UTIL_FILES})

# Target specific compile options

TARGET_INCLUDE_DIRECTORIES(SampleBase 
	PRIVATE ${SAMPLEBASE_PLATFORM_INCLUDES}

	PRIVATE ${DIRECTXTEX_INCLUDE_DIRS}
	PRIVATE ${DXUT_INCLUDE_DIRS}
	
	PRIVATE ${NVCLOTH_ROOT_DIR}/samples/external/shadow_lib
	PRIVATE ${NVCLOTH_ROOT_DIR}/samples/external/hbao
	
	PRIVATE ${IMGUI_INCLUDE_DIRS}
	
	PRIVATE ${SB_SOURCE_DIR}
	PRIVATE ${SB_SOURCE_DIR}/core
	PRIVATE ${SB_SOURCE_DIR}/renderer
	PRIVATE ${SB_SOURCE_DIR}/scene
	PRIVATE ${SB_SOURCE_DIR}/ui
	PRIVATE ${SB_SOURCE_DIR}/utils

	PRIVATE ${PXSHARED_ROOT_DIR}/include/foundation
	PRIVATE ${PXSHARED_ROOT_DIR}/include
	PRIVATE ${PXSHARED_ROOT_DIR}/src/foundation/include
	
	PRIVATE ${NVCLOTH_ROOT_DIR}/include
	PRIVATE ${NVCLOTH_ROOT_DIR}/extensions/include
)

TARGET_COMPILE_DEFINITIONS(SampleBase
	PRIVATE ${SAMPLEBASE_COMPILE_DEFS}
)

SET_TARGET_PROPERTIES(SampleBase PROPERTIES 
	COMPILE_PDB_NAME_DEBUG "SampleBase${CMAKE_DEBUG_POSTFIX}"
	COMPILE_PDB_NAME_CHECKED "SampleBase${CMAKE_CHECKED_POSTFIX}"
	COMPILE_PDB_NAME_PROFILE "SampleBase${CMAKE_PROFILE_POSTFIX}"
	COMPILE_PDB_NAME_RELEASE "SampleBase${CMAKE_RELEASE_POSTFIX}"
)

TARGET_COMPILE_OPTIONS(SampleBase PRIVATE /wd4005 /wd4244)

# Do final direct sets after the target has been defined
TARGET_LINK_LIBRARIES(SampleBase 
	PUBLIC d3dcompiler.lib d3d11.lib dxgi.lib comctl32.lib
	PUBLIC ${HBAO_LIB} ${SHADOW_LIB} ${DXUT_LIBRARIES} ${DIRECTXTEX_LIBRARIES})
	
TARGET_LINK_LIBRARIES(SampleBase PUBLIC PxFoundation)
TARGET_LINK_LIBRARIES(SampleBase PUBLIC NvCloth)
TARGET_LINK_LIBRARIES(SampleBase PUBLIC ${CUDA_CUDA_LIBRARY})

SET_TARGET_PROPERTIES(SampleBase PROPERTIES LINK_FLAGS ${SAMPLEBASE_LINK_FLAGS})

include(${PROJECT_CMAKE_FILES_DIR}/${TARGET_BUILD_PLATFORM}/SampleBase-AT.cmake OPTIONAL)
MESSAGE("SampleBase.cmake [end]")