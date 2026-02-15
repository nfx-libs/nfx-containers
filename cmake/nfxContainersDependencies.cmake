#==============================================================================
# nfx-containers - Dependencies configuration
#==============================================================================

#----------------------------------------------
# Output configuration
#----------------------------------------------

set(_SAVED_CMAKE_REQUIRED_QUIET    ${CMAKE_REQUIRED_QUIET})
set(_SAVED_CMAKE_MESSAGE_LOG_LEVEL ${CMAKE_MESSAGE_LOG_LEVEL})
set(_SAVED_CMAKE_FIND_QUIETLY      ${CMAKE_FIND_QUIETLY})

set(CMAKE_REQUIRED_QUIET    ON     )
set(CMAKE_MESSAGE_LOG_LEVEL VERBOSE) # [ERROR, WARNING, NOTICE, STATUS, VERBOSE, DEBUG]
set(CMAKE_FIND_QUIETLY      ON     )

#----------------------------------------------
# Dependency versions
#----------------------------------------------

set(NFX_CONTAINERS_NFX_HASHING_VERSION "0.4.0")

#----------------------------------------------
# FetchContent dependencies
#----------------------------------------------

include(FetchContent)

if(DEFINED ENV{CI})
    set(FETCHCONTENT_UPDATES_DISCONNECTED OFF)
else()
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
endif()
set(FETCHCONTENT_QUIET OFF)

# --- nfx-hashing ---
find_package(nfx-hashing ${NFX_CONTAINERS_NFX_HASHING_VERSION} QUIET)
if(NOT nfx-hashing_FOUND)
    set(NFX_HASHING_ENABLE_SIMD         ON  CACHE BOOL "")
    set(NFX_HASHING_BUILD_TESTS         OFF CACHE BOOL "")
    set(NFX_HASHING_BUILD_SAMPLES       OFF CACHE BOOL "")
    set(NFX_HASHING_BUILD_BENCHMARKS    OFF CACHE BOOL "")
    set(NFX_HASHING_BUILD_DOCUMENTATION OFF CACHE BOOL "")
    set(NFX_HASHING_INSTALL_PROJECT     OFF CACHE BOOL "")
    set(NFX_HASHING_PACKAGE_SOURCE      OFF CACHE BOOL "")
    set(NFX_HASHING_PACKAGE_ARCHIVE     OFF CACHE BOOL "")
    set(NFX_HASHING_PACKAGE_DEB         OFF CACHE BOOL "")
    set(NFX_HASHING_PACKAGE_RPM         OFF CACHE BOOL "")

    FetchContent_Declare(
        nfx-hashing
            GIT_REPOSITORY https://github.com/nfx-libs/nfx-hashing.git
            GIT_TAG        ${NFX_CONTAINERS_NFX_HASHING_VERSION}
            GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(nfx-hashing)
endif()

#----------------------------------------------
# Cleanup
#----------------------------------------------

set(CMAKE_REQUIRED_QUIET ${_SAVED_CMAKE_REQUIRED_QUIET})
set(CMAKE_MESSAGE_LOG_LEVEL ${_SAVED_CMAKE_MESSAGE_LOG_LEVEL})
set(CMAKE_FIND_QUIETLY ${_SAVED_CMAKE_FIND_QUIETLY})
