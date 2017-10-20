#
# Build PxFoundation
#

SET(PXSHARED_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../../src)

SET(LL_SOURCE_DIR ${PXSHARED_SOURCE_DIR}/foundation)

SET(PXFOUNDATION_LIBTYPE STATIC)

SET(PXFOUNDATION_PLATFORM_FILES
	${LL_SOURCE_DIR}/src/unix/PsUnixAtomic.cpp
	${LL_SOURCE_DIR}/src/unix/PsUnixCpu.cpp
	${LL_SOURCE_DIR}/src/unix/PsUnixFPU.cpp
	${LL_SOURCE_DIR}/src/unix/PsUnixMutex.cpp
	${LL_SOURCE_DIR}/src/unix/PsUnixPrintString.cpp
	${LL_SOURCE_DIR}/src/unix/PsUnixSList.cpp
	${LL_SOURCE_DIR}/src/unix/PsUnixSocket.cpp
	${LL_SOURCE_DIR}/src/unix/PsUnixSync.cpp
	${LL_SOURCE_DIR}/src/unix/PsUnixThread.cpp
	${LL_SOURCE_DIR}/src/unix/PsUnixTime.cpp
	${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c 
)

SET(PXFOUNDATION_PLATFORM_INCLUDES
	${LL_SOURCE_DIR}/include/linux
	${ANDROID_NDK}/sources/android/cpufeatures
)

SET(PXFOUNDATION_COMPILE_DEFS
	# Common to all configurations
	${PXSHARED_ANDROID_COMPILE_DEFS};PxShared_STATIC_LIB;

	$<$<CONFIG:debug>:${PXSHARED_ANDROID_DEBUG_COMPILE_DEFS};>
	$<$<CONFIG:checked>:${PXSHARED_ANDROID_CHECKED_COMPILE_DEFS};>
	$<$<CONFIG:profile>:${PXSHARED_ANDROID_PROFILE_COMPILE_DEFS};>
	$<$<CONFIG:release>:${PXSHARED_ANDROID_RELEASE_COMPILE_DEFS};>
)

# include PxFoundation common
INCLUDE(../common/PxFoundation.cmake)

TARGET_LINK_LIBRARIES(PxFoundation PUBLIC log)

# enable -fPIC so we can link static libs with the editor
SET_TARGET_PROPERTIES(PxFoundation PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
