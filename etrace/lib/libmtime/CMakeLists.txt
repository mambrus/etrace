include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")

set(LIBTIME_SOURCE
	time.c initfini.c
)

set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--undefined=__mtime_init")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--undefined=__mtime_init")

set(LIBTIME_SOURCE
	${LIBTIME_SOURCE}
)

add_library(libmtime ${LIBTIME_SOURCE})
#target_link_libraries(libmtime rt)
target_link_libraries(libmtime)
