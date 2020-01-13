# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
cmake_minimum_required(VERSION 3.13)

file(GLOB SRC CONFIGURE_DEPENDS *.cpp)
file(GLOB INC ../include/*.h ../include/*.hpp ../interface/*.h *.h *.hpp)
file(GLOB INL ../include/*.inl *.inl)

file(GLOB_RECURSE EXTERNAL_INC ../external/*.h ../external/*.hpp)

add_library(d3d11on12 SHARED ${SRC} ${INC} ${INL} ${EXTERNAL_INC} d3d11on12.rc d3d11on12.def)
target_link_libraries(d3d11on12 d3d12translationlayer_wdk)
target_include_directories(d3d11on12
    PRIVATE ../include
    PRIVATE ../external
    PUBLIC ../interface
    PRIVATE ./)
source_group(Inlines FILES ${INL})
source_group("Header Files\\External" FILES ${EXTERNAL_INC})
