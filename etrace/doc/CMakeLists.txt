

################################################################################
# Regenerate interactive help
################################################################################
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gen)
include_directories ("${CMAKE_CURRENT_BINARY_DIR}/gen")
find_program(PANDOC_EXECUTABLE pandoc)

if(PANDOC_EXECUTABLE)
    # Run the generation script
    message(STATUS "Will use pandoc to generate interactive help")
    add_custom_command(
        OUTPUT cmd_help.h cmd_help.man
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/bin/generate.bash ${CMAKE_CURRENT_SOURCE_DIR}/../../wikidata/interactive-help.page
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gen
    )
else(PANDOC_EXECUTABLE)
    message( FATAL_ERROR  "Could not find pandoc. Can't continue.")
endif(PANDOC_EXECUTABLE)

set(DOC_SOURCE
	cmd_help.h
	doc.c
)

add_library(doc ${DOC_SOURCE})
