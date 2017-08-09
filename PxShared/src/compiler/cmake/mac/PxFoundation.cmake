#
# Build PxFoundation
#

SET(PXSHARED_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../../src)

SET(LL_SOURCE_DIR ${PXSHARED_SOURCE_DIR}/foundation)

SET(PXFOUNDATION_LIBTYPE SHARED)

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
)

SET(PXFOUNDATION_PLATFORM_INCLUDES
	${LL_SOURCE_DIR}/include/mac
)

SET(PXFOUNDATION_COMPILE_DEFS

	# Common to all configurations
	${PXSHARED_MAC_COMPILE_DEFS}

	$<$<CONFIG:debug>:${PXSHARED_MAC_DEBUG_COMPILE_DEFS}>
	$<$<CONFIG:checked>:${PXSHARED_MAC_CHECKED_COMPILE_DEFS}>
	$<$<CONFIG:profile>:${PXSHARED_MAC_PROFILE_COMPILE_DEFS}>
	$<$<CONFIG:release>:${PXSHARED_MAC_RELEASE_COMPILE_DEFS}>
)

# include PxFoundation common
INCLUDE(../common/PxFoundation.cmake)