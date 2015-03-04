include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")

set(LOGGING_SOURCE
	log.c
)

if (ENABLE_SYSLOG)
	set(LOGGING_SOURCE
		${LOGGING_SOURCE}
		initfini.c
	)
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--undefined=__liblog_init")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--undefined=__liblog_init")
endif (ENABLE_SYSLOG)

set(LOGGING_SOURCE
	${LOGGING_SOURCE}
)

add_library(liblog ${LOGGING_SOURCE})
