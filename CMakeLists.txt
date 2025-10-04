cmake_minimum_required(VERSION 3.16)
project(ShadPs4Manager VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_GUI "Build the GUI application" ON)
option(BUILD_CLI "Build the CLI application" ON)

# Platform-specific settings
if(WIN32)
    add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external
    ${CMAKE_SOURCE_DIR}/external/zlib
    ${CMAKE_SOURCE_DIR}/external/cryptopp
)

# Configure zlib
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "Build zlib examples" FORCE)
set(ZLIB_BUILD_TESTS OFF CACHE BOOL "Build zlib tests" FORCE)
add_subdirectory(external/zlib EXCLUDE_FROM_ALL)

# Configure cryptopp
set(CRYPTOPP_BUILD_TESTING OFF CACHE BOOL "Build cryptopp tests" FORCE)
set(CRYPTOPP_BUILD_DOCUMENTATION OFF CACHE BOOL "Build cryptopp documentation" FORCE)
set(CRYPTOPP_INSTALL OFF CACHE BOOL "Install cryptopp library" FORCE)
set(CRYPTOPP_SOURCES ${CMAKE_SOURCE_DIR}/external/cryptopp)
add_subdirectory(external/cryptopp-cmake EXCLUDE_FROM_ALL)

# Generate version info
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/common/scm_rev.cpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/src/common/scm_rev.cpp"
    @ONLY
)

# Common source files
set(COMMON_SOURCES
    src/common/io_file.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/common/scm_rev.cpp
)

# Core library sources
set(CORE_SOURCES
    src/core/crypto/crypto.cpp
    src/core/file_format/pkg.cpp
    src/core/file_format/pkg_type.cpp
)

# PKG tool library sources
set(PKG_TOOL_SOURCES
    src/pkg_tool/lib.cpp
)

# Core library (disabled for now due to compatibility issues)
# Will be re-enabled once source code issues are resolved
# add_library(shadps4_manager_core STATIC
#     ${COMMON_SOURCES}
#     ${CORE_SOURCES}
#     ${PKG_TOOL_SOURCES}
# )
# 
# target_include_directories(shadps4_manager_core PUBLIC 
#     ${CMAKE_SOURCE_DIR}/src
# )
# 
# target_link_libraries(shadps4_manager_core PUBLIC 
#     cryptopp::cryptopp 
#     zlibstatic
# )
# 
# if(WIN32)
#     target_link_libraries(shadps4_manager_core PUBLIC ws2_32)
# endif()

# CLI Application (Simple version for now)
if(BUILD_CLI)
    add_executable(shadps4-cli src/cli/simple_main.cpp)
    
    # No dependencies needed for simple version
    if(WIN32)
        target_link_libraries(shadps4-cli PRIVATE ws2_32)
    endif()
endif()

# GUI Application
if(BUILD_GUI)
    find_package(Qt6 COMPONENTS Widgets QUIET)
    
    if(Qt6Widgets_FOUND)
        message(STATUS "Building ShadPs4Manager GUI (Qt6)")
        
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)
        
        add_executable(shadps4-manager-gui
            src/gui/working_gui.cpp
        )
        
        target_link_libraries(shadps4-manager-gui PRIVATE 
            Qt6::Widgets
        )
        
        # On Windows, set it as a GUI application
        if(WIN32)
            set_target_properties(shadps4-manager-gui PROPERTIES
                WIN32_EXECUTABLE TRUE
            )
        endif()
        
    else()
        message(WARNING "Qt6 not found; GUI application will not be built")
    endif()
endif()

# Installation
if(BUILD_CLI)
    install(TARGETS shadps4-cli RUNTIME DESTINATION bin)
endif()

if(BUILD_GUI AND TARGET shadps4-manager-gui)
    install(TARGETS shadps4-manager-gui RUNTIME DESTINATION bin)
endif()

# CPack configuration
set(CPACK_PACKAGE_NAME "ShadPs4Manager")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "PS4 PKG File Extractor")
set(CPACK_PACKAGE_VENDOR "ShadPs4Manager")

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)