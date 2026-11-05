cmake_minimum_required(VERSION 4.2)

project(vex)
set(CMAKE_CXX_STANDARD 20)


file(GLOB_RECURSE dependency "ext/*.cpp" "ext/*.hpp"  "ext/*.h")
file(GLOB_RECURSE source  "src/base/*.cpp" "src/base/*.hpp"  "src/base/*.h")

add_executable( vex main.cpp
        ${source}
        ${dependency}
        src/base/features/impl/settings/theme_manager.hpp

)


include_directories(ext)

target_link_libraries( vex
        d3d11
        dxgi
        d3dcompiler
        dwmapi
        dcomp      
        d2d1
        dwrite
)

target_compile_definitions(vex PRIVATE
        MAPPINGS_ROOT_DIR="${CMAKE_SOURCE_DIR}/ext/mappings"
)

add_compile_options(--static)