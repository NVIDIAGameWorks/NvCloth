# Copy the dlls from the deps

MESSAGE("Windows/SampleBase-AT.cmake [begin]")
ADD_CUSTOM_COMMAND(TARGET SampleBase POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different 
	${SHADOW_DLLS} 
	${BL_EXE_OUTPUT_DIR}
)
ADD_CUSTOM_COMMAND(TARGET SampleBase POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different 
	${HBAO_DLLS} 
	${BL_EXE_OUTPUT_DIR}
)
ADD_CUSTOM_COMMAND(TARGET SampleBase POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different 
	${D3DCOMPILER_DLL} 
	${BL_EXE_OUTPUT_DIR}
)
#ADD_CUSTOM_COMMAND(TARGET SampleBase POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy_if_different 
#	${ASSIMP_DLLS}
#	${BL_EXE_OUTPUT_DIR}
#)

MESSAGE("Windows/SampleBase.cmake [end]")
