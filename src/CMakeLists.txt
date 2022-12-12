# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
cmake_minimum_required(VERSION 3.14)
include(FetchContent)

file(GLOB SRC CONFIGURE_DEPENDS *.cpp)
file(GLOB INC ../include/*.h ../include/*.hpp ../interface/*.h *.h *.hpp)
file(GLOB INL ../include/*.inl *.inl)

file(GLOB_RECURSE EXTERNAL_INC ../external/*.h ../external/*.hpp)

add_library(d3d11on12 SHARED ${SRC} ${INC} ${INL} ${EXTERNAL_INC} d3d11on12.rc d3d11on12.def)

if (CMAKE_VERSION VERSION_GREATER 3.16)
	target_precompile_headers(d3d11on12 PRIVATE ../include/pch.hpp)
endif()

FetchContent_Declare(
    d3d12translationlayer
    GIT_REPOSITORY https://github.com/microsoft/D3D12TranslationLayer.git
    GIT_TAG        5277e2b2994bb475a709033a77bcc7a239def943
)
FetchContent_MakeAvailable(d3d12translationlayer)

target_link_libraries(d3d11on12 d3d12translationlayer_wdk)
target_include_directories(d3d11on12
    PRIVATE ../include
    PRIVATE ../external
    PUBLIC ../interface
    PRIVATE ./)
source_group(Inlines FILES ${INL})
source_group("Header Files\\External" FILES ${EXTERNAL_INC})
