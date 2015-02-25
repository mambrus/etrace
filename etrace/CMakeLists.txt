cmake_minimum_required (VERSION 2.6)
project (etrace C)

#SET(BUILD_PARAVIEW_PLUGIN ON CACHE STRING "Build Paraview plugin?" FORCE)
set(SYSROOT
	${CMAKE_SYSROOT}
	CACHE STRING
	"System path (--system=)")

# Enable any logging
option(ENABLE_LOGGING
    "Enable log messages." ON)

option(ENABLE_SYSLOG
    "Enable logging to syslog (Linux/OSX)" OFF)

if (NOT SYSROOT STREQUAL "")
	message( "** INFO: SYSROOT was either set or defaulted from toolchain file" )
	set(CMAKE_SYSROOT "${SYSROOT}")
	set(CFLAGS_SYSROOT "--sysroot=${CMAKE_SYSROOT}")
endif (NOT SYSROOT STREQUAL "")

set(CFLAGS_SYSROOT "--sysroot=${CMAKE_SYSROOT}")

set(CMAKE_C_FLAGS "${CFLAGS_SYSROOT} -no-integrated-cpp -Wno-unused-function -g3 -ggdb3 -O0 -Wall")
set(CMAKE_C_FLAGS_DEBUG "${CFLAGS_SYSROOT} -no-integrated-cpp -Wno-unused-function -g3 -ggdb3 -O0 -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CFLAGS_SYSROOT} -no-integrated-cpp -Wno-unused-function -g3 -ggdb3 -O0 -Wall")


#Man-, help- and other generated pages
include_directories ("${PROJECT_SOURCE_DIR}/doc/include")
add_subdirectory (doc)
set (EXTRA_LIBS ${EXTRA_LIBS} doc)

if (ENABLE_LOGGING)
    include_directories ("${PROJECT_SOURCE_DIR}/lib/logging/include")
    add_subdirectory (lib/logging)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--undefined=__logging_init")
	set (EXTRA_LIBS ${EXTRA_LIBS} logging)
endif (ENABLE_LOGGING)

# The version number.
set (etrace_VERSION_MAJOR 0)
set (etrace_VERSION_MINOR 0)
set (etrace_VERSION_PATCH 1)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")

set(ETRACE_SOURCE
	etrace.c
	opts.c
	opts_help.c
)

install(PROGRAMS ${PROJECT_BINARY_DIR}/etrace DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(etrace ${ETRACE_SOURCE})
target_link_libraries (etrace ${EXTRA_LIBS})
