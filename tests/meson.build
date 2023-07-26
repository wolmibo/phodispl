test('sample config',
  phodispl,
  args : ['--check-default-config', files('../doc/config.ini')]
)


test('path-sort',
  executable('path-sort',
             ['path-sort.cpp', '../src/path-compare.cpp'],
             include_directories: ['../include']))
