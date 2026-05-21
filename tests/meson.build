gtest_dep      = dependency('gtest',      fallback: ['gtest', 'gtest_dep'])
gtest_main_dep = dependency('gtest_main', fallback: ['gtest', 'gtest_main_dep'])

tagval_test_sources = [
    'test_closed_ended.cpp',
    'test_closed_static_assert.cpp',
    'test_open_ended.cpp',
    'test_open_extern.cpp',
    'test_entry_as.cpp',
    'test_handle_basics.cpp',
    'test_cross_kind.cpp',
    'test_hash.cpp',
    'test_format.cpp',
    'test_ostream.cpp',
    'test_descriptor.cpp',
    'test_kind_registry.cpp',
]

if get_option('json')
    tagval_test_sources += 'test_json.cpp'
endif

if get_option('parcel') and parcel_dep.found()
    tagval_test_sources += 'test_parcel.cpp'
endif

tagval_tests = executable('tagval_tests',
    sources: tagval_test_sources,
    dependencies: [tagval_dep, gtest_dep, gtest_main_dep],
)

test('tagval', tagval_tests, protocol: 'gtest')

# Vendor TUs compiled directly into the extern-split test exe; their .o files
# survive the link only because [[gnu::used]] on the registrar variables
# prevents linker dead-stripping of the otherwise-unreferenced symbols.
tagval_test_extern_split = executable('tagval_test_extern_split',
    sources: [
        'extern_split/test_extern_split.cpp',
        'extern_split/vendor_a.cpp',
        'extern_split/vendor_b.cpp',
    ],
    include_directories: include_directories('extern_split'),
    dependencies: [tagval_dep, gtest_dep, gtest_main_dep],
)

test('tagval_extern_split', tagval_test_extern_split, protocol: 'gtest')
