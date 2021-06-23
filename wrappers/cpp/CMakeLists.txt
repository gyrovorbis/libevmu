cmake_minimum_required(VERSION 3.10)

project(libElysianVmuCpp VERSION ${EVMU_VERSION} DESCRIPTION "ElysianVmu C++ Wrappers" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(EVMU_CPP_SOURCES
    source/evmu_context.cpp)


set(EVMU_CPP_INCLUDES
    api/evmu/cpp/util/evmu_context.hpp)

add_library(libElysianVmuCpp STATIC
    ${EVMU_CPP_SOURCES}
    ${EVMU_CPP_INCLUDES})

target_include_directories(libElysianVmuCpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api>
        $<INSTALL_INTERFACE:api>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/api
)

target_link_libraries(libElysianVmuCpp
    libLibElysianVMU)

