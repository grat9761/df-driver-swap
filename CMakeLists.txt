cmake_minimum_required(VERSION 3.22)
project(dfDriverSwap CXX)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(zygisk     GIT_REPOSITORY https://github.com/topjohnwu/libzygisk.git   GIT_TAG master)
FetchContent_Declare(adrenotools GIT_REPOSITORY https://github.com/bylaws/libadrenotools.git GIT_TAG master)
FetchContent_MakeAvailable(zygisk adrenotools)

add_library(zygisk_module SHARED main.cpp)
target_include_directories(zygisk_module PRIVATE ${zygisk_SOURCE_DIR} ${adrenotools_SOURCE_DIR}/include)
target_link_libraries(zygisk_module PRIVATE zygisk adrenotools log dl)
set_target_properties(zygisk_module PROPERTIES OUTPUT_NAME "module")
