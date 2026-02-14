#==============================================================================
# nfx-containers - CMake target
#==============================================================================

#----------------------------------------------
# Library definition
#----------------------------------------------

# --- Create header-only interface library ---
add_library(${PROJECT_NAME} INTERFACE)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

#----------------------------------------------
# Target properties
#----------------------------------------------

# --- Configure interface library ---
target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${NFX_CONTAINERS_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Set interface compile features for C++20
target_compile_features(${PROJECT_NAME}
    INTERFACE
        cxx_std_20
)

#----------------------------------------------
# Compiler warnings
#----------------------------------------------

# --- Strict warnings for consuming targets ---
target_compile_options(${PROJECT_NAME}
    INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra -Werror>
)

#----------------------------------------------
# Target dependency linking
#----------------------------------------------

target_link_libraries(${PROJECT_NAME}
    INTERFACE
        nfx-hashing::nfx-hashing
)
