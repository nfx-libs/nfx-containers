#==============================================================================
# nfx-containers - Benchmark suite CMake configuration
#==============================================================================

#----------------------------------------------
# Benchmark condition check
#----------------------------------------------

if(NOT NFX_CONTAINERS_BUILD_BENCHMARKS)
	message(STATUS "Benchmarks disabled, skipping...")
	return()
endif()

#----------------------------------------------
# Benchmarks source files
#----------------------------------------------

set(benchmark_sources)

list(APPEND benchmark_sources
	BM_FastHashMap.cpp
	BM_FastHashSet.cpp
	BM_PerfectHashMap.cpp
	BM_TransparentHashMap.cpp
	BM_TransparentHashSet.cpp
)

#----------------------------------------------
# Configure benchmark executables
#----------------------------------------------

foreach(benchmark_source ${benchmark_sources})
	get_filename_component(benchmark_target_name ${benchmark_source} NAME_WE)
	if(NOT TARGET ${benchmark_target_name})
		add_executable(${benchmark_target_name} ${benchmark_source})

		#----------------------------------------------
		# Target linking
		#----------------------------------------------

		target_link_libraries(${benchmark_target_name} PRIVATE
			nfx-containers::nfx-containers
			benchmark::benchmark
		)

		#----------------------------------------------
		# Enable specific CPU features
		#----------------------------------------------

		target_compile_options(${benchmark_target_name} PRIVATE
			$<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>
			$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-msse4.2>
		)

		#----------------------------------------------
		# Properties
		#----------------------------------------------

		set_target_properties(${benchmark_target_name} PROPERTIES
			CXX_STANDARD 20
			CXX_STANDARD_REQUIRED ON
			CXX_EXTENSIONS OFF
			POSITION_INDEPENDENT_CODE ON
			DEBUG_POSTFIX "-d"
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/benchmarks"
			RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/benchmarks"
			RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/benchmarks"
		)
	endif()
endforeach()
