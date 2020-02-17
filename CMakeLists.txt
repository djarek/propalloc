#
# Copyright (c) 2020 Damian Jarek (damian.jarek93@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/djarek/propalloc
#

cmake_minimum_required(VERSION 3.8)
project(propalloc VERSION 1 LANGUAGES CXX)


find_package(Boost 1.70 REQUIRED)

add_library(propalloc INTERFACE)
add_library(propalloc::propalloc ALIAS propalloc)

target_include_directories(propalloc INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(
    propalloc
    INTERFACE
        Boost::boost)

target_compile_features(propalloc INTERFACE cxx_std_20)

include(CTest)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(examples)
endif()


include(GNUInstallDirs)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING
            PATTERN "*.hpp"
            PATTERN "*.ipp")

install(TARGETS propalloc
        EXPORT propallocTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set(propalloc_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P})
unset(CMAKE_SIZEOF_VOID_P)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "propallocConfigVersion.cmake"
    COMPATIBILITY AnyNewerVersion)

set(CMAKE_SIZEOF_VOID_P ${propalloc_SIZEOF_VOID_P})
unset(propalloc_SIZEOF_VOID_P)

install(FILES
            "propallocConfig.cmake"
            "${CMAKE_BINARY_DIR}/propallocConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/propalloc)

install(EXPORT propallocTargets
        FILE propallocTargets.cmake
        NAMESPACE propalloc::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/propalloc)

option(PROPALLOC_BUILD_DOCS "Build propalloc documentation." OFF)
if (PROPALLOC_BUILD_DOCS)
    file(GLOB PROPALLOC_HEADERS "${CMAKE_SOURCE_DIR}/include/propalloc/*.hpp")
    find_package(standardese REQUIRED)
    standardese_generate(propalloc_docs
        INCLUDE_DIRECTORY
            ${CMAKE_SOURCE_DIR}/include
            ${Boost_INCLUDE_DIR}
        MACRO_DEFINITION
            PROPALLOC_SEPARATE_COMPILATION
        INPUT ${PROPALLOC_HEADERS})
endif()
