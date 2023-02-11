set(RES_EXECUTABLE ResourceCompiler)

macro(ADD_RESOURCES OUT FILENAME NAMESPACE_NAME)
    

    # add_templates_impl(${OUT} -o ${FILENAME} -n NAMESPACENAME ${ARGN} )

    set(OUT)
    set(TEMPLATES)



    foreach (File ${ARGN})
        get_filename_component(N ${File} ABSOLUTE)
        list(APPEND TEMPLATES ${N})
    endforeach(File)

    set(OUTFILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}.h
        ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}.cpp
    )

    add_custom_command(
	    OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}.cpp
	    COMMAND ${RES_EXECUTABLE} -o ${FILENAME} -n ${NAMESPACE_NAME} ${TEMPLATES}
	    DEPENDS ${RES_EXECUTABLE} ${TEMPLATES}
	    COMMENT "Converting Resources"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

    set_source_files_properties(${OUTFILES} PROPERTIES GENERATED TRUE)

    set(${OUT} ${OUTFILES})

endmacro(ADD_RESOURCES)

