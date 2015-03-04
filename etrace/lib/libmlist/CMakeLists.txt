include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")

set(LIBMLIST_SOURCE
	mlist.c  modglobals.c initfini.c
)

set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--undefined=__mlist_init")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--undefined=__mlist_init")

set(LIBMLIST_SOURCE
	${LIBMLIST_SOURCE}
)

add_library(libmlist ${LIBMLIST_SOURCE})
