cmake_minimum_required(VERSION 3.17)
project(untitled)

set(CMAKE_CXX_STANDARD 20)

add_library(libcurl STATIC IMPORTED cmake-build-debug/resources.h)
add_library(zlib STATIC IMPORTED)

set_target_properties(libcurl PROPERTIES IMPORTED_LOCATION
        "${CMAKE_SOURCE_DIR}/libs/lib/libcurl.lib")
set_target_properties(zlib PROPERTIES IMPORTED_LOCATION
        "${CMAKE_SOURCE_DIR}/libs/lib/zlib.lib")

include_directories("${CMAKE_SOURCE_DIR}/libs/include")

set(VCPKG_CRT_LINKAGE "static")
set(VCPKG_LIBRARY_LINKAGE "static")

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib")
endif()

add_executable(untitled main.cpp sendemail.cpp sendemail.h screenshot.cpp screenshot.h base64.h main.h resources.rc)
target_link_libraries(untitled libcurl zlib Ws2_32 Wldap32 normaliz crypt32)