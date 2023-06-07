project('PhoDispl',
  ['cpp', 'c'],
  default_options: ['warning_level=3'],
  version:         '0.2.1'
)

add_project_arguments('-std=c++2b', language: 'cpp')



utils_dep = declare_dependency(include_directories: ['extern/utils/include'])

logcerr_dep = dependency(
  'logcerr',
  fallback: ['logcerr', 'logcerr_dep'],
  default_options: ['default_library=static', 'install_as_subproject=false']
)

iconfigp_dep = dependency(
  'iconfigp',
  fallback: ['iconfigp', 'iconfigp_dep'],
  default_options: [
    'default_library=static',
    'install_as_subproject=false',
    'tests=false'
  ]
)

pixglot_dep = dependency(
  'pixglot',
  fallback: ['pixglot', 'pixglot_dep'],
  default_options: [
    'default_library=static',
    'install_as_subproject=false',
    'tests=false'
  ]
)


subdir('extern/win')
subdir('extern/gl')

subdir('src')



summary({
  'window-backends': ', '.join(backends),
})
