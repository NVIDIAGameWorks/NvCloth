# - Try to find imgui SDK
# - Sets IMGUI_LIBRARIES - list of the libraries found
# - Sets IMGUI_SOURCE_FILES - list of files to include in project
# - Sets IMGUI_INCLUDE_DIRS 


include(FindPackageHandleStandardArgs)

# Find the includes

# TODO: Do the version stuff properly!
find_path(IMGUISDK_PATH imgui.h
	PATHS ${GW_DEPS_ROOT}/imgui/${imgui_FIND_VERSION}
	${NVCLOTH_ROOT_DIR}/samples/external/imgui/${imgui_FIND_VERSION}
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(IMGUISDK
	DEFAULT_MSG
	IMGUISDK_PATH
)

if (IMGUISDK_FOUND)
	# NOTE: This include list is way too long and reaches into too many internals.
	# Also may not be good enough for all users.
	SET(IMGUI_INCLUDE_DIRS 
		${IMGUISDK_PATH}
	)
	
	# imgui is added as source to a project that uses it, so put together a list of files here
	SET(IMGUI_SOURCE_FILES
		${IMGUISDK_PATH}/imconfig.h
		${IMGUISDK_PATH}/imgui.cpp
		${IMGUISDK_PATH}/imgui.h
		${IMGUISDK_PATH}/imgui_demo.cpp
		${IMGUISDK_PATH}/imgui_draw.cpp
		${IMGUISDK_PATH}/imgui_internal.h
		${IMGUISDK_PATH}/stb_rect_pack.h
		${IMGUISDK_PATH}/stb_textedit.h
		${IMGUISDK_PATH}/stb_truetype.h
	)
	

	SET(IMGUI_LIBRARIES optimized IMGUI_LIB debug IMGUI_LIB_DEBUG optimized DXUTOPT_LIB debug DXUTOPT_LIB_DEBUG)
endif()
