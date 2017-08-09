#
# Build NvCloth common
#

MESSAGE("[NvCloth]cmake/common/NvCloth.cmake")

#run 
#> dir /a-d /b /s
#in /src, /include, /extensions to list files

IF(NOT DEFINED NV_CLOTH_ENABLE_CUDA)
	SET(NV_CLOTH_ENABLE_CUDA 1)
ENDIF()

IF(${NV_CLOTH_ENABLE_CUDA})
	INCLUDE(../common/NvClothGPU.cmake)
ELSE()
	LIST(APPEND NVCLOTH_COMPILE_DEFS
		NV_CLOTH_ENABLE_CUDA=0
	)
ENDIF()

SET( NV_CLOTH_SOURCE_LIST
	${NVCLOTH_PLATFORM_SOURCE_FILES}
	${PROJECT_ROOT_DIR}/include/NvCloth/Allocator.h
	${PROJECT_ROOT_DIR}/include/NvCloth/Callbacks.h
	${PROJECT_ROOT_DIR}/include/NvCloth/Cloth.h
	${PROJECT_ROOT_DIR}/include/NvCloth/DxContextManagerCallback.h
	${PROJECT_ROOT_DIR}/include/NvCloth/Fabric.h
	${PROJECT_ROOT_DIR}/include/NvCloth/Factory.h
	${PROJECT_ROOT_DIR}/include/NvCloth/PhaseConfig.h
	${PROJECT_ROOT_DIR}/include/NvCloth/Range.h
	${PROJECT_ROOT_DIR}/include/NvCloth/Solver.h
	
	${PROJECT_ROOT_DIR}/src/Allocator.cpp
	${PROJECT_ROOT_DIR}/src/BoundingBox.h
	${PROJECT_ROOT_DIR}/src/Callbacks.cpp
	${PROJECT_ROOT_DIR}/src/ClothBase.h
	${PROJECT_ROOT_DIR}/src/ClothClone.h
	${PROJECT_ROOT_DIR}/src/ClothImpl.h
	${PROJECT_ROOT_DIR}/src/Factory.cpp
	${PROJECT_ROOT_DIR}/src/IndexPair.h
	${PROJECT_ROOT_DIR}/src/IterationState.h
	${PROJECT_ROOT_DIR}/src/MovingAverage.h
	${PROJECT_ROOT_DIR}/src/PhaseConfig.cpp
	${PROJECT_ROOT_DIR}/src/PointInterpolator.h
	${PROJECT_ROOT_DIR}/src/Simd.h
	${PROJECT_ROOT_DIR}/src/StackAllocator.h
	${PROJECT_ROOT_DIR}/src/SwCloth.cpp
	${PROJECT_ROOT_DIR}/src/SwCloth.h
	${PROJECT_ROOT_DIR}/src/SwClothData.cpp
	${PROJECT_ROOT_DIR}/src/SwClothData.h
	${PROJECT_ROOT_DIR}/src/SwCollision.cpp
	${PROJECT_ROOT_DIR}/src/SwCollision.h
	${PROJECT_ROOT_DIR}/src/SwCollisionHelpers.h
	${PROJECT_ROOT_DIR}/src/SwFabric.cpp
	${PROJECT_ROOT_DIR}/src/SwFabric.h
	${PROJECT_ROOT_DIR}/src/SwFactory.cpp
	${PROJECT_ROOT_DIR}/src/SwFactory.h
	${PROJECT_ROOT_DIR}/src/SwInterCollision.cpp
	${PROJECT_ROOT_DIR}/src/SwInterCollision.h
	${PROJECT_ROOT_DIR}/src/SwSelfCollision.cpp
	${PROJECT_ROOT_DIR}/src/SwSelfCollision.h
	${PROJECT_ROOT_DIR}/src/SwSolver.cpp
	${PROJECT_ROOT_DIR}/src/SwSolver.h
	${PROJECT_ROOT_DIR}/src/SwSolverKernel.cpp
	${PROJECT_ROOT_DIR}/src/SwSolverKernel.h
	${PROJECT_ROOT_DIR}/src/TripletScheduler.cpp
	${PROJECT_ROOT_DIR}/src/TripletScheduler.h
	${PROJECT_ROOT_DIR}/src/Vec4T.h
	${PROJECT_ROOT_DIR}/src/NvSimd/NvSimd4f.h
	${PROJECT_ROOT_DIR}/src/NvSimd/NvSimd4i.h
	${PROJECT_ROOT_DIR}/src/NvSimd/NvSimdTypes.h
	${PROJECT_ROOT_DIR}/src/NvSimd/neon/NvNeonSimd4f.h
	${PROJECT_ROOT_DIR}/src/NvSimd/neon/NvNeonSimd4i.h
	${PROJECT_ROOT_DIR}/src/NvSimd/neon/NvNeonSimdTypes.h
	${PROJECT_ROOT_DIR}/src/NvSimd/scalar/NvScalarSimd4f.h
	${PROJECT_ROOT_DIR}/src/NvSimd/scalar/NvScalarSimd4i.h
	${PROJECT_ROOT_DIR}/src/NvSimd/scalar/NvScalarSimdTypes.h
	${PROJECT_ROOT_DIR}/src/NvSimd/sse2/NvSse2Simd4f.h
	${PROJECT_ROOT_DIR}/src/NvSimd/sse2/NvSse2Simd4i.h
	${PROJECT_ROOT_DIR}/src/NvSimd/sse2/NvSse2SimdTypes.h
	${PROJECT_ROOT_DIR}/src/scalar/SwCollisionHelpers.h
	${PROJECT_ROOT_DIR}/src/sse2/SwCollisionHelpers.h
	${PROJECT_ROOT_DIR}/src/sse2/SwSolveConstraints.h
	
	${PROJECT_ROOT_DIR}/extensions/include/NvClothExt/ClothFabricCooker.h
	${PROJECT_ROOT_DIR}/extensions/include/NvClothExt/ClothMeshDesc.h
	${PROJECT_ROOT_DIR}/extensions/include/NvClothExt/ClothMeshQuadifier.h
	${PROJECT_ROOT_DIR}/extensions/include/NvClothExt/ClothTetherCooker.h
	${PROJECT_ROOT_DIR}/extensions/src/ClothFabricCooker.cpp
	${PROJECT_ROOT_DIR}/extensions/src/ClothGeodesicTetherCooker.cpp
	${PROJECT_ROOT_DIR}/extensions/src/ClothMeshQuadifier.cpp
	${PROJECT_ROOT_DIR}/extensions/src/ClothSimpleTetherCooker.cpp
)

ADD_LIBRARY(NvCloth ${NVCLOTH_LIBTYPE} ${NV_CLOTH_SOURCE_LIST})

foreach(source IN LISTS NV_CLOTH_SOURCE_LIST)
	string(LENGTH ${PROJECT_ROOT_DIR} strlen)
	string(SUBSTRING ${source} ${strlen} -1 timmedSource)
	#MESSAGE("${source} -> ${timmedSource}")
	get_filename_component(source_path "${timmedSource}" PATH)
	string(REPLACE "/" "\\" source_path_msvc "${source_path}")
	source_group("${source_path_msvc}" FILES "${source}")
endforeach()

# Target specific compile options


TARGET_INCLUDE_DIRECTORIES(NvCloth 
	PRIVATE ${NVCLOTH_PLATFORM_INCLUDES}

	PRIVATE ${PXSHARED_ROOT_DIR}/include
	PRIVATE ${PXSHARED_ROOT_DIR}/src/foundation/include

	PRIVATE ${PROJECT_ROOT_DIR}/include
	PRIVATE ${PROJECT_ROOT_DIR}/extensions/include
	PRIVATE ${PROJECT_ROOT_DIR}/src
	PRIVATE ${PROJECT_ROOT_DIR}/extensions/src
)

SET(NVCLOTH_COMPILE_DEFS
	${NVCLOTH_COMPILE_DEFS};


	# Common to all configurations

)

# Use generator expressions to set config specific preprocessor definitions
TARGET_COMPILE_DEFINITIONS(NvCloth

	# Common to all configurations
	PRIVATE ${NVCLOTH_COMPILE_DEFS}
)

IF(NOT ${NVCLOTH_LIBTYPE} STREQUAL "OBJECT")
	SET_TARGET_PROPERTIES(NvCloth PROPERTIES 
		COMPILE_PDB_NAME_DEBUG "NvCloth${CMAKE_DEBUG_POSTFIX}"
		COMPILE_PDB_NAME_CHECKED "NvCloth${CMAKE_CHECKED_POSTFIX}"
		COMPILE_PDB_NAME_PROFILE "NvCloth${CMAKE_PROFILE_POSTFIX}"
		COMPILE_PDB_NAME_RELEASE "NvCloth${CMAKE_RELEASE_POSTFIX}"
	)
ENDIF()
MESSAGE("[NvCloth]cmake/common/NvCloth.cmake END")