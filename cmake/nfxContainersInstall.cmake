#==============================================================================
# nfx-containers - Library installation
#==============================================================================

#----------------------------------------------
# Installation condition check
#----------------------------------------------

if(NOT NFX_CONTAINERS_INSTALL_PROJECT)
	message(STATUS "Installation disabled, skipping...")
	return()
endif()

#----------------------------------------------
# Install headers
#----------------------------------------------

install(
	DIRECTORY "${NFX_CONTAINERS_INCLUDE_DIR}/"
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	COMPONENT Development
	FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl"
)

#----------------------------------------------
# Install library targets
#----------------------------------------------

set(install_targets)

# Header-only interface library
list(APPEND install_targets ${PROJECT_NAME})

# Install nfx-hashing dependency if built via FetchContent
if(NOT nfx-hashing_FOUND AND TARGET nfx-hashing::nfx-hashing)
	# Add nfx-hashing to export set
	list(APPEND install_targets nfx-hashing)
	
	# Install nfx-hashing headers
	get_target_property(NFX_HASHING_SOURCE_DIR nfx-hashing::nfx-hashing SOURCE_DIR)
	if(NFX_HASHING_SOURCE_DIR)
		install(
			DIRECTORY "${NFX_HASHING_SOURCE_DIR}/include/"
			DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
			COMPONENT Development
			FILES_MATCHING
				PATTERN "*.h"
				PATTERN "*.hpp"
				PATTERN "*.inl"
		)
		
		# Install nfx-hashing CMake config files
		install(
			EXPORT nfx-hashing-targets
			FILE nfx-hashing-targets.cmake
			NAMESPACE nfx-hashing::
			DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nfx-hashing
			COMPONENT Development
		)
		
		# Configure and install nfx-hashing package config
		include(CMakePackageConfigHelpers)
		configure_package_config_file(
			"${NFX_HASHING_SOURCE_DIR}/cmake/nfx-hashing-config.cmake.in"
			"${CMAKE_CURRENT_BINARY_DIR}/nfx-hashing-config.cmake"
			INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nfx-hashing
			PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
		)
		
		write_basic_package_version_file(
			"${CMAKE_CURRENT_BINARY_DIR}/nfx-hashing-config-version.cmake"
			VERSION 0.1.1
			COMPATIBILITY SameMajorVersion
		)
		
		install(
			FILES
				"${CMAKE_CURRENT_BINARY_DIR}/nfx-hashing-config.cmake"
				"${CMAKE_CURRENT_BINARY_DIR}/nfx-hashing-config-version.cmake"
			DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nfx-hashing
			COMPONENT Development
		)
	endif()
endif()

if(install_targets)
	# Separate nfx-hashing from nfx-containers targets
	list(REMOVE_ITEM install_targets nfx-hashing)
	set(NFX_HASHING_TARGET "")
	if(NOT nfx-hashing_FOUND AND TARGET nfx-hashing::nfx-hashing)
		set(NFX_HASHING_TARGET nfx-hashing)
	endif()
	
	# Install nfx-hashing target separately if needed
	if(NFX_HASHING_TARGET)
		install(
			TARGETS ${NFX_HASHING_TARGET}
			EXPORT nfx-hashing-targets
			ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
				COMPONENT Development
			LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
				COMPONENT Runtime
			RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
				COMPONENT Runtime
			INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		)
	endif()
	
	# Install nfx-containers targets
	install(
		TARGETS ${install_targets}
		EXPORT nfx-containers-targets
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
			COMPONENT Development
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
			COMPONENT Runtime
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
			COMPONENT Runtime
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	)
endif()

#----------------------------------------------
# Install CMake config files
#----------------------------------------------

install(
	EXPORT nfx-containers-targets
	FILE nfx-containers-targets.cmake
	NAMESPACE nfx-containers::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nfx-containers
	COMPONENT Development
)

# Install separate target files for each configuration (multi-config generators)
if(CMAKE_CONFIGURATION_TYPES)
	foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
		install(
			EXPORT nfx-containers-targets
			FILE nfx-containers-targets-${CONFIG}.cmake
			NAMESPACE nfx-containers::
			DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nfx-containers
			CONFIGURATIONS ${CONFIG}
			COMPONENT Development
		)
	endforeach()
endif()

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/nfx-containers-config-version.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/nfx-containers-config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/nfx-containers-config.cmake"
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nfx-containers
	PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
)

install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/nfx-containers-config.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/nfx-containers-config-version.cmake"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nfx-containers
	COMPONENT Development
)

#----------------------------------------------
# Install license files
#----------------------------------------------

install(
	FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt"
	DESTINATION "${CMAKE_INSTALL_DOCDIR}/licenses"
	RENAME "LICENSE-${PROJECT_NAME}.txt"
)

file(GLOB license_files "${CMAKE_CURRENT_SOURCE_DIR}/licenses/LICENSE-*")
foreach(license_file ${license_files})
	get_filename_component(license_name ${license_file} NAME)
	install(
		FILES ${license_file}
		DESTINATION "${CMAKE_INSTALL_DOCDIR}/licenses"
		RENAME "${license_name}.txt"
	)
endforeach()

#----------------------------------------------
# Install documentation
#----------------------------------------------

if(NFX_CONTAINERS_BUILD_DOCUMENTATION)
	install(
		DIRECTORY "${CMAKE_BINARY_DIR}/doc/html"
		DESTINATION ${CMAKE_INSTALL_DOCDIR}
		OPTIONAL
		COMPONENT Documentation
	)
	
	if(WIN32)
		# Install Windows .cmd batch file
		install(
			FILES "${CMAKE_BINARY_DIR}/doc/index.html.cmd"
			DESTINATION ${CMAKE_INSTALL_DOCDIR}
			OPTIONAL
			COMPONENT Documentation
		)
	else()
		# Install Unix symlink
		install(
			FILES "${CMAKE_BINARY_DIR}/doc/index.html"
			DESTINATION ${CMAKE_INSTALL_DOCDIR}
			OPTIONAL
			COMPONENT Documentation
		)
	endif()
endif()

message(STATUS "Installation configured for targets: ${install_targets}")
