cmake_minimum_required(VERSION 3.10)

project(ElysianVmuTest VERSION ${EVMU_VERSION} DESCRIPTION "ElysianVmu Tests" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(Qt6 COMPONENTS Core Test REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTO_RCC ON)
set(CMAKE_AUTOIC ON)

if (NOT TARGET ElysianTest)
    add_subdirectory(lib/elysian-test)
endif()

add_executable(ElysianVmuTests
    main.cpp
    include/test_evmu_context.hpp)

target_link_libraries(ElysianVmuTests
    ElysianTest)

add_test(NAME ElysianVmuTests COMMAND ElysianVmuTests)
