cmake_minimum_required(VERSION 3.16)
project(jexpr)
enable_testing()

# Enable /bigobj for all MSVC builds
if (MSVC)
    add_compile_options(-bigobj)
endif()

option(JEXPR_CATCH2 "Build Catch2 tests" OFF)
option(JEXPR_PYBIND11 "Build pybind11 interface" OFF)
option(JEXPR_EMBIND "Build Emscripten wrapper" OFF)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/externals/nlohmann_json")

set(CMAKE_CXX_STANDARD 20)

add_executable(main "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")



add_executable(invalid_reads "${CMAKE_CURRENT_SOURCE_DIR}/src/invalid_reads.cpp")

# Single-source the version, either from scikit, or from parsing the pyproject.toml
if (SKBUILD)
    add_definitions("-DJEXPRVERSION=\"${SKBUILD_PROJECT_VERSION_FULL}\"")
else()
    file(READ "pyproject.toml" TOML_CONTENT)
    set(REG "version = \"([0-9]+\\.[0-9]+\\.[0-9]+)\"")
    string(REGEX MATCH "${REG}" VERSION_MATCH "${TOML_CONTENT}")
    if (NOT VERSION_MATCH)
        message(FATAL_ERROR "Can't parse the version")
    else()
        string(REGEX REPLACE "${REG}" "\\1" PROJECT_VERSION_FULL "${VERSION_MATCH}")
        message(STATUS "Version: ${PROJECT_VERSION_FULL}")
        add_definitions("-DJEXPRVERSION=\"${PROJECT_VERSION_FULL}\"")
    endif()
endif()

if (JEXPR_CATCH2)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/externals/Catch2")
    add_executable(catch_tests "${CMAKE_CURRENT_SOURCE_DIR}/src/tests/tests.cxx")
    target_link_libraries(catch_tests PUBLIC Catch2WithMain)
    add_test(normal_tests catch_tests)
endif()

if (JEXPR_PYBIND11)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/externals/pybind11" "pybind11")
    file(GLOB pybind11_files "${CMAKE_CURRENT_SOURCE_DIR}/interface/CPP/*.cpp")
    pybind11_add_module(jexpr "${pybind11_files}")
    #target_include_directories(jexpr PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/externals/pybind11_json/include")
    
    # Install directive for scikit-build-core
    install(TARGETS jexpr LIBRARY DESTINATION jexpr)
endif()

if (JEXPR_EMBIND)
    add_definitions(-Wno-missing-template-arg-list-after-template-kw)
    
    # If you want a monolithic file with no async memory loading, define EMSCRIPTEN_NO_MEMORY_INIT_FILE
    # if(EMSCRIPTEN_NO_MEMORY_INIT_FILE)
    #   set(EMSCRIPTEN_INIT_FLAG "--memory-init-file 0")
    # else()
    #   set(EMSCRIPTEN_INIT_FLAG "--memory-init-file 1")
    # endif()
    
    add_definitions( -sDISABLE_EXCEPTION_CATCHING=0)
    set(CMAKE_EXE_LINKER_FLAGS "--bind ${EMSCRIPTEN_INIT_FLAG} -sASSERTIONS=1 -sDISABLE_EXCEPTION_CATCHING=0")
    set(CMAKE_BUILD_TYPE Release)
    set(APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/interface/js/emscripten_interface.cxx")
    add_executable(jexpr ${APP_SOURCES})
    SET_TARGET_PROPERTIES(jexpr PROPERTIES PREFIX "" SUFFIX .js)
endif()
