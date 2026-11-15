cmake_minimum_required(VERSION 4.2)

project(vex LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "vex supports Windows x64 and Windows ARM64 only.")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Select an x64 or ARM64 toolchain; 32-bit builds are not supported.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(MSVC)
    if(NOT CMAKE_CXX_COMPILER_ARCHITECTURE_ID MATCHES "^(x64|ARM64)$")
        message(FATAL_ERROR "Select native x64 or ARM64, rather than ARM64EC or Win32.")
    endif()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    if(NOT DEFINED CMAKE_MSVC_DEBUG_INFORMATION_FORMAT)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")
    endif()
    add_compile_options(/utf-8)
endif()

file(GLOB_RECURSE mjni_source CONFIGURE_DEPENDS "ext/mJNI/*.cpp")
add_library(mjni STATIC ${mjni_source}
        src/base/sdk/events/subscriber.cpp
        src/base/sdk/events/subscriber.h)
target_include_directories(mjni PUBLIC ext)
target_compile_definitions(mjni PUBLIC NOMINMAX)
target_link_libraries(mjni PUBLIC psapi)

file(GLOB_RECURSE dependency CONFIGURE_DEPENDS "ext/*.cpp" "ext/*.hpp" "ext/*.h")
list(FILTER dependency EXCLUDE REGEX "/ext/mJNI/")
file(GLOB_RECURSE source CONFIGURE_DEPENDS "src/base/*.cpp" "src/base/*.hpp" "src/base/*.h")
add_executable(vex main.cpp ${source} ${dependency})

target_link_libraries(vex PRIVATE
        mjni
        d3d11
        dxgi
        d3dcompiler
        dwmapi
        dcomp      
        d2d1
        dwrite
        advapi32
)

target_compile_definitions(vex PRIVATE
        MAPPINGS_ROOT_DIR="${CMAKE_SOURCE_DIR}/ext/mappings"
)

