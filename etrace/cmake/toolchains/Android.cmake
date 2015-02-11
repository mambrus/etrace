include (CMakeForceCompiler)
set(CMAKE_SYSTEM_NAME Generic)

set(HOME $ENV{HOME})
set(CMAKE_SYSROOT ${HOME}/usr/local/android-ndk-r10d/platforms/android-21/arch-arm)
set(CMAKE_STAGING_PREFIX /tmp/stage/Android)

set(triple arm-linux-androideabi-)
set(CMAKE_C_COMPILER_TARGET ${triple})
#set(CMAKE_C_COMPILER ${triple}gcc)
#set(CMAKE_CXX_COMPILER ${triple}g++)

CMAKE_FORCE_C_COMPILER(${triple}gcc ${triple}gcc)
CMAKE_FORCE_CXX_COMPILER(${triple}g++ ${triple}g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
