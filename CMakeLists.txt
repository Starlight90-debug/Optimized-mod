cmake_minimum_required(VERSION 3.21)
project(MultiversionBridge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── CPM (dependency manager) ──────────────────────────────────────────────────
file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake
    ${CMAKE_BINARY_DIR}/cmake/CPM.cmake
    EXPECTED_HASH SHA256=c8cdc32c03816538ce22781ed72964dc864b2a34a310d3b7104d5c0d9cd92965
)
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

# ── LeviLamina 1.26.10.4 SDK ─────────────────────────────────────────────────
CPMAddPackage(
    NAME           LeviLamina
    GITHUB_REPOSITORY LiteLDev/LeviLamina
    GIT_TAG        1.26.10.4
    OPTIONS
        "LL_INSTALL_SDK ON"
)

# ── SculkCatalystMC/Protocol ──────────────────────────────────────────────────
# Protocol requires OpenSSL + the SculkCatalystMC/Reflection header-only lib.
find_package(OpenSSL REQUIRED)

CPMAddPackage(
    NAME           SculkReflection
    GITHUB_REPOSITORY SculkCatalystMC/Reflection
    GIT_TAG        main
)

CPMAddPackage(
    NAME           Protocol
    GITHUB_REPOSITORY SculkCatalystMC/Protocol
    GIT_TAG        main
    OPTIONS
        "CMAKE_CXX_STANDARD 23"  # Protocol requires C++23 internally
)

# ── Plugin target ─────────────────────────────────────────────────────────────
file(GLOB_RECURSE PLUGIN_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    LeviLamina::LeviLamina
    Protocol::Protocol
    OpenSSL::Crypto
)

# Protocol itself uses C++23; our plugin code is C++20 but links to it fine.
set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""        # avoid lib prefix on Windows/Linux
    OUTPUT_NAME "multiversion-bridge"
)

# Windows-specific: export all symbols needed by LL plugin loader
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()
