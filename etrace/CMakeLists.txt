cmake_minimum_required (VERSION 2.6)
project (etrace C)

################################################################################
# Configuration options
################################################################################
set(DEF_DEBUGFS_PATH
	"/sys/kernel/debug"
	CACHE STRING
	"Default path to where debugfs is mounted")

set(DEF_WORKDIR
	"./"
	CACHE STRING
	"Default workdir/where output is placed")

set(DEF_PTIME
	"1000"
	CACHE STRING
	"Default harvest time (uS)")

set(DEF_FTRACE_CLOCK
	"local"
	CACHE STRING
	"ftrace_clock to use. Note: Valid ones depend on target.")

set(DEF_FTRACE_BUFF_SIZE_KB
	"10000"
	CACHE STRING
	"Size of each CPU trace buffer in kB.")

#SET(BUILD_PARAVIEW_PLUGIN ON CACHE STRING "Build Paraview plugin?" FORCE)
set(SYSROOT
	${CMAKE_SYSROOT}
	CACHE STRING
	"System path (--system=)")

# Enable any liblog
option(ENABLE_LOGGING
    "Enable log messages." ON)

option(ENABLE_SYSLOG
    "Enable log to syslog (Linux/OSX)" OFF)
################################################################################

if (NOT SYSROOT STREQUAL "")
	message( "** INFO: SYSROOT was either set or defaulted from toolchain file" )
	set(CMAKE_SYSROOT "${SYSROOT}")
	set(CFLAGS_SYSROOT "--sysroot=${CMAKE_SYSROOT}")
endif (NOT SYSROOT STREQUAL "")

set(CFLAGS_SYSROOT "--sysroot=${CMAKE_SYSROOT}")

set(CMAKE_C_FLAGS "${CFLAGS_SYSROOT} -no-integrated-cpp -Wno-unused-function -g3 -ggdb3 -O0 -Wall")
set(CMAKE_C_FLAGS_DEBUG "${CFLAGS_SYSROOT} -no-integrated-cpp -Wno-unused-function -g3 -ggdb3 -O0 -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CFLAGS_SYSROOT} -no-integrated-cpp -Wno-unused-function -g3 -ggdb3 -O0 -Wall")


################################################################################
# Libraries (src binding)
################################################################################
# -Man-, help- and other generated pages
include_directories ("${PROJECT_SOURCE_DIR}/doc/include")
add_subdirectory (doc)
set (EXTRA_LIBS ${EXTRA_LIBS} doc)

if (ENABLE_LOGGING)
    include_directories ("${PROJECT_SOURCE_DIR}/lib/liblog/include")
    add_subdirectory (lib/liblog)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--undefined=__liblog_init")
	set (EXTRA_LIBS ${EXTRA_LIBS} liblog)
endif (ENABLE_LOGGING)

# -Lists-
include_directories ("${PROJECT_SOURCE_DIR}/lib/libmlist/include")
add_subdirectory (lib/libmlist)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--undefined=__mlist_init")
set (EXTRA_LIBS ${EXTRA_LIBS} libmlist)

# -time-
include_directories ("${PROJECT_SOURCE_DIR}/lib/libmtime/include")
add_subdirectory (lib/libmtime)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--undefined=__mtime_init")
set (EXTRA_LIBS ${EXTRA_LIBS} libmtime)

################################################################################
# Project
################################################################################
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
	proc.c
)

install(PROGRAMS ${PROJECT_BINARY_DIR}/etrace DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(etrace ${ETRACE_SOURCE})
target_link_libraries (etrace ${EXTRA_LIBS})
