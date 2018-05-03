#
# Build PxFoundation common
#

SET(PXSHARED_SOURCE_DIR ${PXSHARED_ROOT_DIR}/src)
SET(LL_SOURCE_DIR ${PXSHARED_SOURCE_DIR}/foundation)

# Include here after the directories are defined so that the platform specific file can use the variables.
include(${PXSHARED_ROOT_DIR}/${PROJECT_CMAKE_FILES_DIR}/${TARGET_BUILD_PLATFORM}/PxFoundation.cmake)

SET(PXFOUNDATION_HEADERS	
	${PXSHARED_ROOT_DIR}/include/foundation/Px.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxAllocatorCallback.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxAssert.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxBitAndData.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxBounds3.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxErrorCallback.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxErrors.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxFlags.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxFoundation.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxFoundationVersion.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxIntrinsics.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxIO.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxMat33.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxMat44.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxMath.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxMathUtils.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxMemory.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxPlane.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxPreprocessor.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxProfiler.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxQuat.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxSimpleTypes.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxStrideIterator.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxTransform.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxUnionCast.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxVec2.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxVec3.h
	${PXSHARED_ROOT_DIR}/include/foundation/PxVec4.h
)
SOURCE_GROUP(include FILES ${PXFOUNDATION_HEADERS})

SET(PXFOUNDATION_SOURCE
	${LL_SOURCE_DIR}/src/PsAllocator.cpp
	${LL_SOURCE_DIR}/src/PsAssert.cpp
	${LL_SOURCE_DIR}/src/PsFoundation.cpp
	${LL_SOURCE_DIR}/src/PsMathUtils.cpp
	${LL_SOURCE_DIR}/src/PsString.cpp
	${LL_SOURCE_DIR}/src/PsTempAllocator.cpp
	${LL_SOURCE_DIR}/src/PsUtilities.cpp
)
SOURCE_GROUP(src\\src FILES ${PXFOUNDATION_SOURCE})

SET(PXFOUNDATION_SOURCE_HEADERS
	${LL_SOURCE_DIR}/include/Ps.h
	${LL_SOURCE_DIR}/include/PsAlignedMalloc.h
	${LL_SOURCE_DIR}/include/PsAlloca.h
	${LL_SOURCE_DIR}/include/PsAllocator.h
	${LL_SOURCE_DIR}/include/PsAoS.h
	${LL_SOURCE_DIR}/include/PsArray.h
	${LL_SOURCE_DIR}/include/PsAtomic.h
	${LL_SOURCE_DIR}/include/PsBasicTemplates.h
	${LL_SOURCE_DIR}/include/PsBitUtils.h
	${LL_SOURCE_DIR}/include/PsBroadcast.h
	${LL_SOURCE_DIR}/include/PsCpu.h
	${LL_SOURCE_DIR}/include/PsFoundation.h
	${LL_SOURCE_DIR}/include/PsFPU.h
	${LL_SOURCE_DIR}/include/PsHash.h
	${LL_SOURCE_DIR}/include/PsHashInternals.h
	${LL_SOURCE_DIR}/include/PsHashMap.h
	${LL_SOURCE_DIR}/include/PsHashSet.h
	${LL_SOURCE_DIR}/include/PsInlineAllocator.h
	${LL_SOURCE_DIR}/include/PsInlineAoS.h
	${LL_SOURCE_DIR}/include/PsInlineArray.h
	${LL_SOURCE_DIR}/include/PsIntrinsics.h
	${LL_SOURCE_DIR}/include/PsMathUtils.h
	${LL_SOURCE_DIR}/include/PsMutex.h
	${LL_SOURCE_DIR}/include/PsPool.h
	${LL_SOURCE_DIR}/include/PsSList.h
	${LL_SOURCE_DIR}/include/PsSocket.h
	${LL_SOURCE_DIR}/include/PsSort.h
	${LL_SOURCE_DIR}/include/PsSortInternals.h
	${LL_SOURCE_DIR}/include/PsString.h
	${LL_SOURCE_DIR}/include/PsSync.h
	${LL_SOURCE_DIR}/include/PsTempAllocator.h
	${LL_SOURCE_DIR}/include/PsThread.h
	${LL_SOURCE_DIR}/include/PsTime.h
	${LL_SOURCE_DIR}/include/PsUserAllocated.h
	${LL_SOURCE_DIR}/include/PsUtilities.h
	${LL_SOURCE_DIR}/include/PsVecMath.h
	${LL_SOURCE_DIR}/include/PsVecMathAoSScalar.h
	${LL_SOURCE_DIR}/include/PsVecMathAoSScalarInline.h
	${LL_SOURCE_DIR}/include/PsVecMathSSE.h
	${LL_SOURCE_DIR}/include/PsVecMathUtilities.h
	${LL_SOURCE_DIR}/include/PsVecQuat.h
	${LL_SOURCE_DIR}/include/PsVecTransform.h
)
SOURCE_GROUP("src\\include" FILES ${PXFOUNDATION_SOURCE_HEADERS})

ADD_LIBRARY(PxFoundation ${PXFOUNDATION_LIBTYPE} 
	${PXFOUNDATION_SOURCE}	
	${PXFOUNDATION_SOURCE_HEADERS}
	${PXFOUNDATION_HEADERS}
	
	${PXFOUNDATION_PLATFORM_FILES}
)

# Add the headers to the install
INSTALL(FILES ${PXFOUNDATION_HEADERS} DESTINATION include/foundation)

INSTALL(FILES ${PXFOUNDATION_SOURCE_HEADERS} DESTINATION src/foundation/include)

TARGET_INCLUDE_DIRECTORIES(PxFoundation 
	PRIVATE ${PXSHARED_ROOT_DIR}/include
	PRIVATE ${LL_SOURCE_DIR}/include

	PRIVATE ${PXFOUNDATION_PLATFORM_INCLUDES}
		
	INTERFACE $<INSTALL_INTERFACE:include>$<BUILD_INTERFACE:${PXSHARED_ROOT_DIR}/include> ${PXFOUNDATION_PLATFORM_INTERFACE_HEADERS}
	INTERFACE $<INSTALL_INTERFACE:include/foundation>$<BUILD_INTERFACE:${PXSHARED_ROOT_DIR}/include/foundation>
	#INTERFACE $<INSTALL_INTERFACE:include/foundation/windows>$<BUILD_INTERFACE:${PXSHARED_ROOT_DIR}/include/foundation/windows>

	# FIXME: This is really terrible! Don't export src directories
	INTERFACE $<INSTALL_INTERFACE:src/foundation/include>$<BUILD_INTERFACE:${PXSHARED_ROOT_DIR}/src/foundation/include>

)

TARGET_COMPILE_DEFINITIONS(PxFoundation 
	PRIVATE ${PXFOUNDATION_COMPILE_DEFS}
)

# Add linked libraries
TARGET_LINK_LIBRARIES(PxFoundation 
	PRIVATE ${PXFOUNDATION_PLATFORM_LINKED_LIBS}
)


IF(USE_GAMEWORKS_OUTPUT_DIRS AND PXFOUNDATION_LIBTYPE STREQUAL "STATIC")
	SET_TARGET_PROPERTIES(PxFoundation PROPERTIES 
		ARCHIVE_OUTPUT_NAME_DEBUG "PxFoundation_static"
		ARCHIVE_OUTPUT_NAME_CHECKED "PxFoundation_static"
		ARCHIVE_OUTPUT_NAME_PROFILE "PxFoundation_static"
		ARCHIVE_OUTPUT_NAME_RELEASE "PxFoundation_static"
	)
	
	IF(DEFINED CMAKE_DEBUG_POSTFIX)	
		SET_TARGET_PROPERTIES(PxFoundation PROPERTIES 	
			COMPILE_PDB_NAME_DEBUG "PxFoundation_static_${CMAKE_DEBUG_POSTFIX}"
			COMPILE_PDB_NAME_CHECKED "PxFoundation_static_${CMAKE_CHECKED_POSTFIX}"
			COMPILE_PDB_NAME_PROFILE "PxFoundation_static_${CMAKE_PROFILE_POSTFIX}"			
			COMPILE_PDB_NAME_RELEASE "PxFoundation_static_${CMAKE_RELEASE_POSTFIX}"
		)
	ELSE()
		SET_TARGET_PROPERTIES(PxFoundation PROPERTIES 	
			COMPILE_PDB_NAME_DEBUG "PxFoundation_static"
			COMPILE_PDB_NAME_CHECKED "PxFoundation_static"
			COMPILE_PDB_NAME_PROFILE "PxFoundation_static"
			COMPILE_PDB_NAME_RELEASE "PxFoundation_static"
		)
	ENDIF()		
ELSE()
	SET_TARGET_PROPERTIES(PxFoundation PROPERTIES 
		COMPILE_PDB_NAME_DEBUG "PxFoundation${CMAKE_DEBUG_POSTFIX}"
		COMPILE_PDB_NAME_CHECKED "PxFoundation${CMAKE_CHECKED_POSTFIX}"
		COMPILE_PDB_NAME_PROFILE "PxFoundation${CMAKE_PROFILE_POSTFIX}"
		COMPILE_PDB_NAME_RELEASE "PxFoundation${CMAKE_RELEASE_POSTFIX}"
	)
ENDIF()

# enable -fPIC so we can link static libs with the editor
SET_TARGET_PROPERTIES(PxFoundation PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
