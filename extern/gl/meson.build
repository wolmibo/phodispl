deps = [
  utils_dep,
  logcerr_dep,
  dependency('epoxy',     version: '>=1.5.10'),
  dependency('freetype2', version: '>=24.3.18'),
]

sources = [
  'src/base.cpp',
  'src/glyphs.cpp',
  'src/mesh.cpp',
  'src/program.cpp',

  'src/primitives/quad.cpp'
]

gl_lib = static_library(
  'glutils',
  sources,
  dependencies:        deps,
  include_directories: ['include'],
)

gl_dep = declare_dependency(
  link_with:           gl_lib,
  dependencies:        deps,
  include_directories: ['include']
)
