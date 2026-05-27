include_guard(GLOBAL)
include(FetchContent)

if(TAGVAL_WITH_NLOHMANN_JSON)
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    set(JSON_Install    OFF CACHE INTERNAL "")
    FetchContent_Declare(
        nlohmann_json
        URL      https://github.com/nlohmann/json/archive/refs/tags/v3.12.0.tar.gz
        URL_HASH SHA256=4b92eb0c06d10683f7447ce9406cb97cd4b453be18d7279320f7b2f025c10187
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        FIND_PACKAGE_ARGS 3.12.0
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# commons is a core (mandatory) dependency. Declare it before parcel so that
# FetchContent dedups against parcel's identical v0.1.3 pin, and so it is
# available even when TAGVAL_WITH_PARCEL is OFF.
FetchContent_Declare(
    commons
    URL      https://github.com/aurimasniekis/cpp-commons/archive/refs/tags/v0.1.3.tar.gz
    URL_HASH SHA256=2f5615ac96a1a1dddda5424ed75c0d1a0142f115f215502562f479ef138fc30d
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    FIND_PACKAGE_ARGS 0.1
)
FetchContent_MakeAvailable(commons)

if(TAGVAL_WITH_PARCEL)
    set(PARCEL_BUILD_TESTS    OFF CACHE INTERNAL "")
    set(PARCEL_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    FetchContent_Declare(
        parcel
        GIT_REPOSITORY https://github.com/aurimasniekis/cpp-parcel.git
        GIT_TAG        v0.2.0
        FIND_PACKAGE_ARGS 0.2
    )
    FetchContent_MakeAvailable(parcel)
endif()

if(TAGVAL_BUILD_TESTS)
    set(INSTALL_GTEST OFF CACHE INTERNAL "")
    set(BUILD_GMOCK   OFF CACHE INTERNAL "")
    FetchContent_Declare(
        googletest
        URL      https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
        URL_HASH SHA256=7b42b4d6ed48810c5362c265a17faebe90dc2373c885e5216439d37927f02926
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        FIND_PACKAGE_ARGS NAMES GTest
    )
    FetchContent_MakeAvailable(googletest)
endif()
