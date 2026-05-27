project('tagval', 'cpp',
    version: '0.2.0',
    license: 'MIT',
    default_options: [
        'cpp_std=c++23',
        'warning_level=3',
        'werror=false',
    ],
    meson_version: '>=1.3.0',
)

version_parts = meson.project_version().split('.')

tagval_version_conf = configuration_data()
tagval_version_conf.set('PROJECT_VERSION_MAJOR', version_parts[0])
tagval_version_conf.set('PROJECT_VERSION_MINOR', version_parts[1])
tagval_version_conf.set('PROJECT_VERSION_PATCH', version_parts[2])
tagval_version_conf.set('PROJECT_VERSION', meson.project_version())

subdir('include/tagval')

# include_directories('include') resolves to both the source-side `include/`
# and the matching build-side `include/`, so the generated `tagval/version.hpp`
# is found automatically.
tagval_inc = include_directories('include')

tagval_deps = []
tagval_pkgconfig_requires = []

# commons is a mandatory core dependency (FixedString, Color, Icon).
commons_dep = dependency('commons', version: '>=0.1.3',
    fallback: ['commons', 'commons_dep'])
tagval_deps += commons_dep
tagval_pkgconfig_requires += 'commons'

if get_option('json')
    json_dep = dependency('nlohmann_json', version: '>=3.12.0',
        fallback: ['nlohmann_json', 'nlohmann_json_dep'])
    tagval_deps += json_dep
    tagval_pkgconfig_requires += 'nlohmann_json'
    add_project_arguments('-DTAGVAL_WITH_NLOHMANN_JSON=1', language: 'cpp')
endif

parcel_dep = dependency('', required: false)
if get_option('parcel')
    parcel_dep = dependency('parcel', required: false)
    if parcel_dep.found()
        tagval_deps += parcel_dep
        tagval_pkgconfig_requires += 'parcel'
        add_project_arguments('-DTAGVAL_WITH_PARCEL=1', language: 'cpp')
    endif
endif

tagval_dep = declare_dependency(
    include_directories: tagval_inc,
    dependencies: tagval_deps,
    version: meson.project_version(),
)

meson.override_dependency('tagval', tagval_dep)

install_subdir('include/tagval',
    install_dir: get_option('includedir'),
    exclude_files: ['version.hpp.in', 'meson.build'],
)

import('pkgconfig').generate(
    name: 'tagval',
    description: 'Modern C++23 header-only library of tagged values',
    version: meson.project_version(),
    requires: tagval_pkgconfig_requires,
)

if get_option('tests')
    subdir('tests')
endif
if get_option('examples')
    subdir('examples')
endif
