#
# Build PxPvdSDK
#

SET(PXSHARED_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../../src)

SET(LL_SOURCE_DIR ${PXSHARED_SOURCE_DIR}/pvd)

SET(PXPVDSDK_LIBTYPE STATIC)

# Use generator expressions to set config specific preprocessor definitions
SET(PXPVDSDK_COMPILE_DEFS 
	# Common to all configurations
	${PXSHARED_ANDROID_COMPILE_DEFS};PxShared_STATIC_LIB;

	$<$<CONFIG:debug>:${PXSHARED_ANDROID_DEBUG_COMPILE_DEFS};>
	$<$<CONFIG:checked>:${PXSHARED_ANDROID_CHECKED_COMPILE_DEFS};>
	$<$<CONFIG:profile>:${PXSHARED_ANDROID_PROFILE_COMPILE_DEFS};>
	$<$<CONFIG:release>:${PXSHARED_ANDROID_RELEASE_COMPILE_DEFS};>
)

# include PxPvdSDK common
INCLUDE(../common/PxPvdSDK.cmake)

# Add linked libraries
TARGET_LINK_LIBRARIES(PxPvdSDK PRIVATE PxFoundation)

# enable -fPIC so we can link static libs with the editor
SET_TARGET_PROPERTIES(PxPvdSDK PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
