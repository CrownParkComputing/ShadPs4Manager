cmake_minimum_required(VERSION 3.16)
project(ShadPs4Manager VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_GUI "Build the GUI application" ON)

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
    src/core/file_format/pkg_optimized.cpp
    src/core/update_merger.cpp
)

# PKG tool library sources
set(PKG_TOOL_SOURCES
    src/pkg_tool/lib.cpp
)

# Core library (temporarily enabled to diagnose issues)
add_library(shadps4_manager_core STATIC
    ${COMMON_SOURCES}
    ${CORE_SOURCES}
    ${PKG_TOOL_SOURCES}
)

target_include_directories(shadps4_manager_core PUBLIC 
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(shadps4_manager_core PUBLIC 
    cryptopp::cryptopp 
    zlibstatic
)

if(WIN32)
    target_link_libraries(shadps4_manager_core PUBLIC ws2_32)
endif()

# CLI PKG Extractor Tool
add_executable(shadps4-pkg-extractor
    src/pkg-extractor/pkg_extractor.cpp
)

target_link_libraries(shadps4-pkg-extractor PRIVATE
    shadps4_manager_core
)

target_include_directories(shadps4-pkg-extractor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# GUI Application
if(BUILD_GUI)
    # Find OpenGL first (required for Qt6 on some systems)
    find_package(OpenGL)
    
    # Try to find Qt6 with different methods
    find_package(Qt6 COMPONENTS Widgets Network Multimedia QUIET)
    
    # If not found and CMAKE_PREFIX_PATH is set, try again
    if(NOT Qt6Widgets_FOUND AND CMAKE_PREFIX_PATH)
        message(STATUS "Retrying Qt6 search with CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
        find_package(Qt6 COMPONENTS Widgets Network Multimedia QUIET)
    endif()
    
    # Try to find Qt6 in common locations if still not found
    if(NOT Qt6Widgets_FOUND)
        if(WIN32)
            # Common Windows locations
            set(QT_SEARCH_PATHS
                "$ENV{Qt6_DIR}"
                "$ENV{QTDIR}"
                "C:/Qt/6.5.3/msvc2019_64"
                "C:/Qt/6.5.2/msvc2019_64"
                "C:/Qt/6.5.1/msvc2019_64"
                "C:/Qt/6.5.0/msvc2019_64"
            )
        else()
            # Common Linux locations
            set(QT_SEARCH_PATHS
                "/usr/lib/x86_64-linux-gnu/cmake/Qt6"
                "/usr/lib64/cmake/Qt6"
                "/usr/local/Qt/6.5.3/lib/cmake/Qt6"
            )
        endif()
        
        foreach(QT_PATH ${QT_SEARCH_PATHS})
            if(EXISTS "${QT_PATH}" AND NOT Qt6Widgets_FOUND)
                message(STATUS "Trying Qt6 path: ${QT_PATH}")
                set(CMAKE_PREFIX_PATH "${QT_PATH};${CMAKE_PREFIX_PATH}")
                find_package(Qt6 COMPONENTS Widgets Network Multimedia QUIET)
            endif()
        endforeach()
    endif()
    
    if(Qt6Widgets_FOUND)
        message(STATUS "Building ShadPs4Manager GUI (Qt6)")
        message(STATUS "Qt6 Location: ${Qt6_DIR}")
        
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)
        
        # Main Manager GUI Application
        add_executable(shadps4-manager-gui
            src/manager-gui/working_gui.cpp
            src/manager-gui/settings.cpp
            src/manager-gui/settings_page.cpp
            src/manager-gui/game_library.cpp
            src/manager-gui/gamecard.cpp
            src/manager-gui/downloads_folder.cpp
            src/manager-gui/installation_folder.cpp
            src/manager-gui/param_sfo.cpp
            src/manager-gui/igdbservice.cpp
            src/manager-gui/credential_manager.cpp
            src/manager-gui/welcome_tab.cpp
        )
        
        target_link_libraries(shadps4-manager-gui PRIVATE
            Qt6::Widgets
            Qt6::Network
            Qt6::Multimedia
            shadps4_manager_core
        )
        
        # Link OpenGL if found
        if(OpenGL_FOUND)
            target_link_libraries(shadps4-manager-gui PRIVATE ${OPENGL_LIBRARIES})
            target_include_directories(shadps4-manager-gui PRIVATE ${OPENGL_INCLUDE_DIRS})
        endif()

        target_include_directories(shadps4-manager-gui PRIVATE
            ${Qt6Widgets_INCLUDE_DIRS}
        )
        
        # Unlock Code Generator (Admin Tool)
        add_executable(shadps4-unlock-code-generator
            src/unlock-generator/unlock_code_generator.cpp
        )
        
        set_target_properties(shadps4-unlock-code-generator PROPERTIES
            AUTOMOC ON
        )
        
        target_link_libraries(shadps4-unlock-code-generator PRIVATE
            Qt6::Widgets
        )
        
        # On Windows, set GUI applications
        if(WIN32)
            set_target_properties(shadps4-manager-gui PROPERTIES
                WIN32_EXECUTABLE TRUE
            )
            set_target_properties(shadps4-unlock-code-generator PROPERTIES
                WIN32_EXECUTABLE TRUE
            )
        endif()
        
    else()
        message(WARNING "Qt6 not found; GUI application will not be built")
        if(CMAKE_PREFIX_PATH)
            message(STATUS "CMAKE_PREFIX_PATH was: ${CMAKE_PREFIX_PATH}")
        endif()
    endif()
endif()

# Installation
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
