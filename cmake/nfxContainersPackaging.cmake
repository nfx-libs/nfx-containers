#==============================================================================
# nfx-containers - Library packaging configuration (CPack)
#==============================================================================

#----------------------------------------------
# Packaging condition check
#----------------------------------------------

if(NOT NFX_CONTAINERS_PACKAGE_SOURCE)
    return()
endif()

#----------------------------------------------
# CPack configuration
#----------------------------------------------

# --- Common settings ---
set(CPACK_PACKAGE_NAME                  ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR                "nfx")
set(CPACK_PACKAGE_DIRECTORY             "${CMAKE_BINARY_DIR}/packages")
set(CPACK_PACKAGE_VERSION_MAJOR         ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR         ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH         ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION               ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY   "Modern C++20 header-only library providing high-performance hash containers with Robin Hood and perfect hashing")
set(CPACK_PACKAGE_HOMEPAGE_URL          ${CMAKE_PROJECT_HOMEPAGE_URL})
set(CPACK_PACKAGE_CHECKSUM              "SHA256")
set(CPACK_RESOURCE_FILE_LICENSE         ${NFX_CONTAINERS_LICENSE_FILE})
set(CPACK_VERBATIM_VARIABLES            TRUE)

# --- Source package settings ---
set(CPACK_SOURCE_PACKAGE_FILE_NAME      "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Source")
set(CPACK_SOURCE_GENERATOR              "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    ".git/"
    ".gitignore"
    "build/"
    ".deps/"
    "Testing/"
    ".vs/"
    ".vscode/"
    ".*~$"
)

message(STATUS "CPack configured:")
message(STATUS "  Source packages: TGZ, ZIP")
message(STATUS "  Output dir: ${CPACK_PACKAGE_DIRECTORY}")

#----------------------------------------------
# Include CPack
#----------------------------------------------

include(CPack)
