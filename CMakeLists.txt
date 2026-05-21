cmake_minimum_required(VERSION 3.25)

project(tagval
    VERSION     0.1.0
    DESCRIPTION "Modern C++23 header-only library of tagged values (open/closed enumerations with metadata)"
    LANGUAGES   CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

option(TAGVAL_BUILD_TESTS         "Build tagval tests"                  ${PROJECT_IS_TOP_LEVEL})
option(TAGVAL_BUILD_EXAMPLES      "Build tagval examples"               ${PROJECT_IS_TOP_LEVEL})
option(TAGVAL_BUILD_DOCS          "Build Doxygen API documentation"     OFF)
option(TAGVAL_ENABLE_CLANG_TIDY   "Run clang-tidy during build"         OFF)
option(TAGVAL_ENABLE_SANITIZERS   "Build with ASan + UBSan (Debug)"     OFF)
option(TAGVAL_ENABLE_COVERAGE     "Build with Clang source-based coverage" OFF)
option(TAGVAL_WARNINGS_AS_ERRORS  "Treat compiler warnings as errors"   ${PROJECT_IS_TOP_LEVEL})
option(TAGVAL_INSTALL             "Generate install rules for tagval"   ${PROJECT_IS_TOP_LEVEL})
option(TAGVAL_WITH_NLOHMANN_JSON  "Enable optional nlohmann/json integration" ON)
option(TAGVAL_WITH_PARCEL         "Enable optional cpp-parcel integration" ON)

include(CompilerWarnings)
include(Sanitizers)
include(Coverage)
include(Dependencies)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tagval/version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/tagval/version.hpp"
    @ONLY
)

add_library(tagval INTERFACE)
add_library(tagval::tagval ALIAS tagval)

target_include_directories(tagval
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(tagval INTERFACE cxx_std_23)

if(TAGVAL_WITH_NLOHMANN_JSON AND TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(tagval INTERFACE nlohmann_json::nlohmann_json)
    target_compile_definitions(tagval INTERFACE TAGVAL_WITH_NLOHMANN_JSON=1)
endif()

if(TAGVAL_WITH_PARCEL AND TARGET parcel::parcel)
    target_link_libraries(tagval INTERFACE parcel::parcel)
    target_compile_definitions(tagval INTERFACE TAGVAL_WITH_PARCEL=1)
endif()

if(TAGVAL_ENABLE_CLANG_TIDY)
    find_program(TAGVAL_CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
    set(_tagval_tidy_cmd
        "${TAGVAL_CLANG_TIDY_EXE}"
        "--config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy"
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        execute_process(
            COMMAND "${CMAKE_CXX_COMPILER}" -print-search-dirs
            OUTPUT_VARIABLE _tagval_gcc_search_dirs
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(_tagval_gcc_search_dirs MATCHES "install: ([^\n]+)")
            list(APPEND _tagval_tidy_cmd
                "--extra-arg-before=--gcc-install-dir=${CMAKE_MATCH_1}")
            message(STATUS
                "tagval: pinning clang-tidy to gcc install dir ${CMAKE_MATCH_1}")
        endif()
    endif()
    set(CMAKE_CXX_CLANG_TIDY "${_tagval_tidy_cmd}")
    message(STATUS "tagval: clang-tidy enabled (${TAGVAL_CLANG_TIDY_EXE})")
endif()

if(TAGVAL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(TAGVAL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(TAGVAL_BUILD_DOCS)
    find_package(Doxygen REQUIRED)
    set(TAGVAL_DOXYGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in"
        "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        @ONLY
    )
    add_custom_target(tagval_docs
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Generating Doxygen documentation"
        VERBATIM
    )
endif()

if(TAGVAL_INSTALL AND (DEFINED nlohmann_json_SOURCE_DIR OR DEFINED parcel_SOURCE_DIR))
    message(STATUS
        "tagval: TAGVAL_INSTALL disabled — a fetched dependency (nlohmann_json or parcel) "
        "cannot be re-exported. Install it via system package or find_package, or disable "
        "TAGVAL_WITH_NLOHMANN_JSON / TAGVAL_WITH_PARCEL, to re-enable install rules."
    )
    set(TAGVAL_INSTALL OFF)
endif()

if(TAGVAL_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(TARGETS tagval
        EXPORT  tagvalTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(DIRECTORY include/tagval
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp"
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/include/tagval/version.hpp"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tagval
    )

    install(EXPORT tagvalTargets
        FILE        tagvalTargets.cmake
        NAMESPACE   tagval::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tagval
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/tagvalConfigVersion.cmake"
        VERSION       ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
        ARCH_INDEPENDENT
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/tagvalConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/tagvalConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tagval
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/tagvalConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/tagvalConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tagval
    )
endif()
