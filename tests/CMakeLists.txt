include(GoogleTest)

set(_tagval_test_sources
    test_closed_ended.cpp
    test_closed_static_assert.cpp
    test_open_ended.cpp
    test_open_extern.cpp
    test_entry_as.cpp
    test_handle_basics.cpp
    test_cross_kind.cpp
    test_hash.cpp
    test_format.cpp
    test_ostream.cpp
    test_descriptor.cpp
    test_kind_registry.cpp
)

if(TAGVAL_WITH_NLOHMANN_JSON AND TARGET nlohmann_json::nlohmann_json)
    list(APPEND _tagval_test_sources test_json.cpp)
endif()

if(TAGVAL_WITH_PARCEL AND TARGET parcel::parcel)
    list(APPEND _tagval_test_sources test_parcel.cpp)
endif()

add_executable(tagval_tests ${_tagval_test_sources})

target_link_libraries(tagval_tests
    PRIVATE
        tagval::tagval
        GTest::gtest_main
)

tagval_set_warnings(tagval_tests PRIVATE)
tagval_enable_sanitizers(tagval_tests)
tagval_enable_coverage(tagval_tests)

gtest_discover_tests(tagval_tests)

# Multi-TU coverage for extern entries. The vendor TUs are built as an OBJECT
# library so the resulting .o files are linked directly into the test exe —
# nothing in the test exe references the inline `_registered_` registrar, so
# the entries land in Registry only because [[gnu::used]] prevents the linker
# (Apple ld -dead_strip / GNU ld --gc-sections) from culling the registrar
# symbol after the object is in.
add_library(tagval_test_vendor OBJECT
    extern_split/vendor_a.cpp
    extern_split/vendor_b.cpp
)
target_link_libraries(tagval_test_vendor PUBLIC tagval::tagval)
target_include_directories(tagval_test_vendor PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/extern_split"
)
tagval_set_warnings(tagval_test_vendor PRIVATE)
tagval_enable_sanitizers(tagval_test_vendor)
tagval_enable_coverage(tagval_test_vendor)

add_executable(tagval_test_extern_split extern_split/test_extern_split.cpp)
target_link_libraries(tagval_test_extern_split
    PRIVATE
        tagval_test_vendor
        tagval::tagval
        GTest::gtest_main
)
tagval_set_warnings(tagval_test_extern_split PRIVATE)
tagval_enable_sanitizers(tagval_test_extern_split)
tagval_enable_coverage(tagval_test_extern_split)

gtest_discover_tests(tagval_test_extern_split)
