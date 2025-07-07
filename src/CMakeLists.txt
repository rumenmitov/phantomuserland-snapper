cmake_minimum_required(VERSION 3.10)
project(Isomem)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# disable -rdynamic flag
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

if (DEFINED ENV{PHANTOM_BUILD_NO_DISPLAY})
    set (PHANTOM_BUILD_NO_DISPLAY $ENV{PHANTOM_BUILD_NO_DISPLAY})
elseif (NOT DEFINED PHANTOM_BUILD_NO_DISPLAY)
    set (PHANTOM_BUILD_NO_DISPLAY 0)
endif ()

if (DEFINED ENV{PHANTOM_BUILD_TESTS_ONLY})
    set (PHANTOM_BUILD_TESTS_ONLY $ENV{PHANTOM_BUILD_TESTS_ONLY})
elseif (NOT DEFINED PHANTOM_BUILD_TESTS_ONLY)
    set (PHANTOM_BUILD_TESTS_ONLY 0)
endif ()

message ("-- Isomem Build Configurations:")
if (PHANTOM_BUILD_TESTS_ONLY EQUAL 0)
    message ("    BUILD : KERNEL + TESTS")
else ()
    add_definitions(-DPHANTOM_TESTS_ONLY=1)
    message ("    BUILD : TESTS ONLY")
endif ()
if (PHANTOM_BUILD_NO_DISPLAY EQUAL 0)
    message ("    Display enabled")
else ()
    add_definitions(-DPHANTOM_NO_DISPLAY=1)
    message ("    Display disabled")
endif ()

include (phantom/gl/phantom_gl.cmake)
include (genode_env/genode_env.cmake)
include (phantom/isomem/isomem.cmake)
include (phantom/libwin/libwin.cmake)
include (phantom/libfreetype/libfreetype.cmake)
include (phantom/vm/vm.cmake)

# this is a bit of a workaround that makes libc_setjmp work with goa
# only required if libc is not ported to phatnom yet
include (genode_env/libc_setjmp.cmake)

set(ISOMEM_SOURCE
    ${PHANTOM_GENODE_ENV_SOURCE}
    ${PHANTOM_GL_SOURCE}
    ${PHANTOM_ISOMEM_SOURCE}
    ${PHANTOM_LIBWIN_SOURCE}
    ${PHANTOM_LIBFREETYPE_SOURCE}
    ${PHANTOM_PVM_SOURCE}
    ${LIBC_SETJMP_SOURCES}
)

set(ARCH genode-amd64)

include_directories(include/stub_libc)
include_directories(include)
include_directories(include/${ARCH})

add_definitions(-DKERNEL)
add_definitions(-DPHANTOM_GENODE)
add_definitions(-DNO_NETWORK)

get_filename_component(relConfigInclude "include/kernel/config.h"
                       REALPATH BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
add_compile_options(-include ${relConfigInclude})

add_executable(isomem ${ISOMEM_SOURCE})

target_link_libraries (isomem vmlib)

set_property(TARGET isomem PROPERTY C_STANDARD 99)
set_property(TARGET isomem PROPERTY CXX_STANDARD 17)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=implicit-function-declaration")
