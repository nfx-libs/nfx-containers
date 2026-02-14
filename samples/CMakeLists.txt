#==============================================================================
# nfx-containers - Samples
#==============================================================================

#----------------------------------------------
# Sample condition check
#----------------------------------------------

if(NOT NFX_CONTAINERS_BUILD_SAMPLES)
    return()
endif()

#----------------------------------------------
# Samples source files
#----------------------------------------------

set(sample_sources)

list(APPEND sample_sources
    Sample_FastHashMap.cpp
    Sample_FastHashSet.cpp
    Sample_OrderedHashMap.cpp
    Sample_OrderedHashSet.cpp
    Sample_PerfectHashMap.cpp
    Sample_StackHashMap.cpp
    Sample_StackHashSet.cpp
    Sample_StackVector.cpp
    Sample_TransparentHashMap.cpp
    Sample_TransparentHashSet.cpp
)

#----------------------------------------------
# Configure samples executables
#----------------------------------------------

foreach(sample_source ${sample_sources})
    get_filename_component(sample_target_name ${sample_source} NAME_WE)

    if(NOT TARGET ${sample_target_name})
        add_executable(${sample_target_name} ${sample_source})

        #----------------------------------------------
        # Target linking
        #----------------------------------------------

        target_link_libraries(${sample_target_name}
            PRIVATE
                nfx-containers::nfx-containers
        )

        #----------------------------------------------
        # Enable specific CPU features
        #----------------------------------------------

        target_compile_options(${sample_target_name}
            PRIVATE
                $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>
                $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-msse4.2>

                $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
                $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra -Werror>
        )

        #----------------------------------------------
        # Properties
        #----------------------------------------------

        set_target_properties(${sample_target_name}
            PROPERTIES
                CXX_STANDARD 20
                CXX_STANDARD_REQUIRED ON
                CXX_EXTENSIONS OFF
                POSITION_INDEPENDENT_CODE ON
                DEBUG_POSTFIX "-d"
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
    endif()
endforeach()
