tagval_examples = [
    ['tagval_closed_ended',    'closed_ended.cpp'],
    ['tagval_open_ended',      'open_ended.cpp'],
    ['tagval_extern_entries',  'extern_entries.cpp'],
    ['tagval_metadata',        'metadata.cpp'],
    ['tagval_formatting',      'formatting.cpp'],
]

if get_option('json')
    tagval_examples += [['tagval_json_integration', 'json_integration.cpp']]
endif

if get_option('parcel') and parcel_dep.found()
    tagval_examples += [['tagval_parcel_integration', 'parcel_integration.cpp']]
endif

foreach ex : tagval_examples
    executable(ex[0], ex[1], dependencies: [tagval_dep])
endforeach
