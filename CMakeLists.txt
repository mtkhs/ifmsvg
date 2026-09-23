cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static-md")
endif()

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    else()
        message(FATAL_ERROR "VCPKG_ROOT not set and CMAKE_TOOLCHAIN_FILE not provided")
    endif()
endif()

project(ifmsvg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "ifmsvg requires a 64-bit (x64) build. Pass -A x64.")
endif()

if(MSVC)
    add_compile_options(/W4 /utf-8 /Zc:__cplusplus)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# --- Corrosion (CMake-Rust bridge) via FetchContent ---
# vcpkg の corrosion ポートはこの環境では利用不可のため FetchContent で取得。
include(FetchContent)
FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG        v0.5.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Corrosion)

# --- resvg C-API crate via FetchContent ---
FetchContent_Declare(
    resvg_src
    GIT_REPOSITORY https://github.com/RazrFalcon/resvg.git
    GIT_TAG        v0.45.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(resvg_src)

corrosion_import_crate(
    MANIFEST_PATH ${resvg_src_SOURCE_DIR}/crates/c-api/Cargo.toml
    CRATE_TYPES staticlib
    PROFILE release
)
set(RESVG_INCLUDE_DIR ${resvg_src_SOURCE_DIR}/crates/c-api)

# Sources shared between plugin and tests.
set(MODULE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/path_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/bmp.cpp
    ${CMAKE_SOURCE_DIR}/src/svg_detect.cpp
    ${CMAKE_SOURCE_DIR}/src/svgz.cpp
    ${CMAKE_SOURCE_DIR}/src/config.cpp
    ${CMAKE_SOURCE_DIR}/src/compositor.cpp
    ${CMAKE_SOURCE_DIR}/src/font_db.cpp
    ${CMAKE_SOURCE_DIR}/src/svg_render.cpp
)

add_library(ifmsvg SHARED
    src/ifmsvg.cpp
    ${MODULE_SOURCES}
)
target_include_directories(ifmsvg PRIVATE include)
set_target_properties(ifmsvg PROPERTIES
    PREFIX ""
    OUTPUT_NAME "ifmsvg"
    SUFFIX ".sph"
    LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/ifmsvg.def"
)

find_package(ZLIB REQUIRED)
target_link_libraries(ifmsvg PRIVATE ZLIB::ZLIB)
target_include_directories(ifmsvg PRIVATE ${RESVG_INCLUDE_DIR})
target_link_libraries(ifmsvg PRIVATE resvg)

# /EHa: catch(...) catches SEH exceptions (e.g. Rust panics propagated as
# Windows structured exceptions). Applied to all files that call resvg C-API
# or contain catch(...) guards that must intercept Rust panics.
set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/src/ifmsvg.cpp
    ${CMAKE_SOURCE_DIR}/src/font_db.cpp
    ${CMAKE_SOURCE_DIR}/src/svg_render.cpp
    PROPERTIES COMPILE_OPTIONS "/EHa"
)

option(IFMSVG_BUILD_TESTS "Build tests" ON)
if(IFMSVG_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
