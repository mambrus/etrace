cmake_minimum_required (VERSION 2.6)
project (etrace)

# Enable any logging
option(ENABLE_LOGGING
    "Enable log messages." ON)

option(ENABLE_SYSLOG
    "Enable logging to syslog (Linux/OSX)" OFF)

set(CMAKE_C_FLAGS "-no-integrated-cpp -Wno-unused-function -g3 -ggdb3 -O0 -Wall")
set(CMAKE_C_FLAGS_DEBUG "-no-integrated-cpp -Wno-unused-function -g3 -ggdb3 -O0 -Wall")
set(CMAKE_C_FLAGS_RELEASE "-no-integrated-cpp -Wno-unused-function -g3 -ggdb3 -O0 -Wall")

if (ENABLE_LOGGING)
    include_directories ("${PROJECT_SOURCE_DIR}/lib/logging/include")
    add_subdirectory (lib/logging)
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

add_executable(etrace etrace.c)
target_link_libraries (etrace logging)
