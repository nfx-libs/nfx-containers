#==============================================================================
# nfx-containers - CMake build configuration
#==============================================================================

#----------------------------------------------
# Build validation
#----------------------------------------------

# --- Validate CMake version ---
if(CMAKE_VERSION VERSION_LESS "3.20")
    message(FATAL_ERROR "CMake 3.20 or higher is required for reliable C++20 support")
endif()

# --- Prevent in-source builds ---
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

#----------------------------------------------
# Directory configuration
#----------------------------------------------

set(NFX_CONTAINERS_LICENSE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(NFX_CONTAINERS_DIR         "${PROJECT_SOURCE_DIR}"         CACHE PATH  "Root directory"   )
set(NFX_CONTAINERS_INCLUDE_DIR "${NFX_CONTAINERS_DIR}/include" CACHE PATH  "Include directory")

#----------------------------------------------
# Output directory configuration
#----------------------------------------------

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
