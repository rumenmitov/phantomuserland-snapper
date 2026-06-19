
# set PHANTOM_GENODE_ENV_DIR to this directory's path
set (PHANTOM_GENODE_ENV_DIR ${CMAKE_CURRENT_LIST_DIR})

# add this directory as path to search include files in
include_directories(${PHANTOM_GENODE_ENV_DIR})

# look for sources
# file (GLOB_RECURSE source_all ${PHANTOM_GENODE_ENV_DIR}/*.cc)
set (source_all 
    main.cc 
    tests_adapters.cc 
    tests_hal_sync.cc 
    tests_hal_vmem.cc 
    tests_hal_threads.cc 
    tests_ph_api.cc 
    util.cc 
    phantom_snapper.cc
    phantom_malloc.cc 
    phantom_threads.cc 
    phantom_vmem.cc 
    phantom_disk.cc 
    phantom_sync.cc 
    phantom_timer.cc 
    phantom_unsorted.cc
    phantom_framebuffer.cc
    phantom_fs.cc
    phantom_fs_internal.cc
)

# convert filenames to absolute paths
list(TRANSFORM source_all PREPEND ${PHANTOM_GENODE_ENV_DIR}/)

# put all sources in PHANTOM_GENODE_ENV_SOURCE
set (PHANTOM_GENODE_ENV_SOURCE ${source_all})
