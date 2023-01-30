cmake_minimum_required(VERSION 3.10)

set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO)
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
#set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.test")
set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "Falco Girgis")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")

project(ElysianVmuTests VERSION ${EVMU_VERSION} DESCRIPTION "ElysianVMU Tests" LANGUAGES C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_EXTENSIONS OFF)


add_executable(ElysianVmuTests
    source/evmu_test_main.c
    source/evmu_cpu_test_suite.c
    include/evmu_cpu_test_suite.h
    source/evmu_memory_test_suite.c
    include/evmu_memory_test_suite.h
    source/evmu_isa_test_suite.c
    include/evmu_isa_test_suite.h)

target_link_libraries(ElysianVmuTests
    libLibElysianVMU)

if(VITA)
    add_compile_definitions(ElysianVmuTests PUBLIC VITA=1)
    include("${VITASDK}/share/vita.cmake" REQUIRED)

    target_link_libraries(ElysianVmuTests
        debugNet
    )

    vita_create_self(eboot.bin ElysianVmuTests UNSAFE)

    vita_create_vpk("EvmuTests_v${EVMU_CORE_VERSION}.vpk" EVMUTESTS0 eboot.bin
        VERSION "01.00"
        NAME "ElysianVMU Tests v${EVMU_CORE_VERSION}"
    )
endif()

target_include_directories(ElysianVmuTests
    PRIVATE include)

set_target_properties(ElysianVmuTests PROPERTIES
    MACOSX_BUNDLE_INFO_STRING "ElysianVMU Tests v${EVMU_CORE_VERSION}"
    MACOSX_BUNDLE_BUNDLE_NAME "ElysianVmuTests"
    MACOSX_BUNDLE_BUNDLE_VERSION ${EVMU_CORE_VERSION}
    MACOSX_BUNDLE_GUI_IDENTIFIER "ElysianVmuTests"
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${EVMU_CORE_VERSION}
    XCODE_ATTRIBUTE_PRODUCT_NAME "ElysianVmuTests"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")

add_test(NAME ElysianVmuTests COMMAND ElysianVmuTests)
