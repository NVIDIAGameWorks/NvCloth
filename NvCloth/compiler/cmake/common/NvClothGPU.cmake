#
# Build NvCloth GPU
#
MESSAGE("[NvCloth]cmake/common/NvClothGPU.cmake")

IF(UNIX AND NOT APPLE)
        LIST(APPEND CMAKE_PREFIX_PATH
                        "/usr/local/cuda/lib64/stubs"
                        )
ENDIF()
FIND_PACKAGE(CUDA 8 REQUIRED)
MESSAGE("Found CUDA:" ${CUDA_INCLUDE_DIRS})

LIST(APPEND NVCLOTH_PLATFORM_INCLUDES
                PRIVATE ${CUDA_INCLUDE_DIRS}
                )

LIST(APPEND NVCLOTH_PLATFORM_SOURCE_FILES
                ${PROJECT_ROOT_DIR}/src/cuda/CuCheckSuccess.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuCloth.cpp
                ${PROJECT_ROOT_DIR}/src/cuda/CuCloth.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuClothClone.cpp
                ${PROJECT_ROOT_DIR}/src/cuda/CuClothData.cpp
                ${PROJECT_ROOT_DIR}/src/cuda/CuClothData.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuCollision.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuContextLock.cpp
                ${PROJECT_ROOT_DIR}/src/cuda/CuContextLock.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuDevicePointer.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuDeviceVector.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuFabric.cpp
                ${PROJECT_ROOT_DIR}/src/cuda/CuFabric.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuFactory.cpp
                ${PROJECT_ROOT_DIR}/src/cuda/CuFactory.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuPhaseConfig.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuPinnedAllocator.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuSelfCollision.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuSolver.cpp
                ${PROJECT_ROOT_DIR}/src/cuda/CuSolver.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuSolverKernel.h
                ${PROJECT_ROOT_DIR}/src/cuda/CuSolverKernelBlob.h
                )
SET(CUDA_NVCC_FLAGS
        ${CUDA_NVCC_FLAGS};
        # Options are documented on http://docs.nvidia.com/cuda/cuda-compiler-driver-nvcc/index.html#command-option-types-and-notation
        # Important: to be able to debug CUDA kernel (correct stepping and workable warp watches),
        # you should add the -G (short for --device-debug) option to the line below.
        # Don't forget to remove it afterwards, since the code execution will be very slow with this flag.
        -use_fast_math -ftz=true -prec-div=false -prec-sqrt=false -lineinfo -gencode arch=compute_20,code=sm_20 -gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_50,code=sm_50 -gencode arch=compute_50,code=compute_50 -D_USE_MATH_DEFINES -DNDEBUG -DPX_PROFILE=1
)

SET(NVCLOTH_CUDA_FILES
        ${PROJECT_ROOT_DIR}/src/cuda/CuSolverKernel.cu
)

SET(CUDA_BLOB "${PROJECT_ROOT_DIR}/src/cuda/CuSolverKernelBlob.h")

cuda_include_directories(
        ${PXSHARED_ROOT_DIR}/include
        ${PXSHARED_ROOT_DIR}/src/foundation/include
        ${PROJECT_ROOT_DIR}/include/NvCloth
)

cuda_compile_fatbin(CUDA_FATBIN ${NVCLOTH_CUDA_FILES})

SET(NVCLOTH_PLATFORM_SOURCE_FILES ${NVCLOTH_PLATFORM_SOURCE_FILES} ${CUDA_BLOB} ${NVCLOTH_CUDA_FILES})

add_custom_command(
        OUTPUT ${CUDA_BLOB}
COMMAND ${CUDA_TOOLKIT_ROOT_DIR}/bin/bin2c --name kCuSolverKernel ${CUDA_FATBIN} > ${CUDA_BLOB}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
DEPENDS ${CUDA_FATBIN}
        VERBATIM
)

LIST(APPEND NVCLOTH_COMPILE_DEFS
        NV_CLOTH_ENABLE_CUDA=1
)
MESSAGE("[NvCloth]cmake/common/NvClothGPU.cmake END")
