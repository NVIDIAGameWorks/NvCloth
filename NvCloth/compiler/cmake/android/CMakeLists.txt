cmake_minimum_required(VERSION 3.3)
MESSAGE("[NvCloth]cmake/android/CMakeList.txt")
include(../common/CMakeLists.txt)

STRING(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWERCASE)

IF(NOT DEFINED TARGET_BUILD_PLATFORM) # Not defined, default to android
	SET(TARGET_BUILD_PLATFORM "android")
ENDIF()

SET(PLATFORM_LIST android)

IF (NOT ${TARGET_BUILD_PLATFORM} IN_LIST PLATFORM_LIST)
	MESSAGE(FATAL_ERROR "Invalid platform:" ${TARGET_BUILD_PLATFORM})
ENDIF()

if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11 -fno-rtti -fno-exceptions -ffunction-sections -fdata-sections -funwind-tables -fomit-frame-pointer -funswitch-loops -finline-limit=300 -fno-strict-aliasing -fstack-protector -Wno-invalid-offsetof ")
	SET(NVCLOTH_ANDROID_ABI_DEFS __arm__;PX_SIMD_DISABLED)
elseif(${ANDROID_ABI} STREQUAL "armeabi-v7a with NEON")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -ffast-math -fno-exceptions -ffunction-sections -funwind-tables -fomit-frame-pointer -funswitch-loops -finline-limit=300 -fno-strict-aliasing -fstack-protector -Wno-invalid-offsetof ")
	SET(NVCLOTH_ANDROID_ABI_DEFS __arm__;__ARM_NEON__;NV_SIMD_SCALAR=1)
elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11 -fno-rtti -ffast-math -fno-exceptions -ffunction-sections -fdata-sections -Wno-invalid-offsetof ")
	SET(NVCLOTH_ANDROID_ABI_DEFS __arm64__;__ARM_NEON__)
elseif(${ANDROID_ABI} STREQUAL "x86")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11 -fno-rtti -fno-exceptions -ffunction-sections -fdata-sections -Wno-invalid-offsetof -fpack-struct=8 -malign-double ")
	SET(NVCLOTH_ANDROID_ABI_DEFS __i386__)
elseif(${ANDROID_ABI} STREQUAL "x86_64")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11 -fno-rtti -fno-exceptions -ffunction-sections -fdata-sections -Wno-invalid-offsetof -mstackrealign -msse3  ")
	SET(NVCLOTH_ANDROID_ABI_DEFS __x86_64__)
endif()
	
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_CXX_FLAGS_CHECKED "-O3")
SET(CMAKE_CXX_FLAGS_PROFILE "-O3")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")

# a version from pxshared
#SET(NVCLOTH_ANDROID_COMPILE_DEFS _LIB;__STDC_LIMIT_MACROS;)
SET(NVCLOTH_ANDROID_COMPILE_DEFS _LIB;__STDC_LIMIT_MACROS;_CRT_SECURE_NO_DEPRECATE;_CRT_NONSTDC_NO_DEPRECATE;${NVCLOTH_ANDROID_ABI_DEFS})
SET(NVCLOTH_ANDROID_DEBUG_COMPILE_DEFS _DEBUG;PX_DEBUG=1;PX_CHECKED=1)
SET(NVCLOTH_ANDROID_CHECKED_COMPILE_DEFS NDEBUG;PX_CHECKED=1)
SET(NVCLOTH_ANDROID_PROFILE_COMPILE_DEFS NDEBUG;PX_PROFILE=1)
SET(NVCLOTH_ANDROID_RELEASE_COMPILE_DEFS NDEBUG)


SET(PX_SELECT_COMPONENTS PxFoundation)
FIND_PACKAGE(PxShared REQUIRED)

# Include all of the projects
INCLUDE(NvCloth.cmake)

MESSAGE("[NvCloth]cmake/android/CMakeList.txt END")