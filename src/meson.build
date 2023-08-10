shader_dir = '../shader'

resource_files = [
  ['shader_plane_fs',           shader_dir / 'fragment/plane.fs.glsl'],
  ['shader_plane_solid_fs',     shader_dir / 'fragment/plane-solid.fs.glsl'],
  ['shader_plane_colorize_fs',  shader_dir / 'fragment/plane-colorize.fs.glsl'],
  ['shader_plane_uv_vs',        shader_dir / 'vertex/plane-uv.vs.glsl'],
  ['shader_plane_object_vs',    shader_dir / 'vertex/plane-object.vs.glsl'],
  ['shader_progress_circle_vs', shader_dir / 'vertex/progress-circle.vs.glsl'],
  ['shader_progress_circle_fs', shader_dir / 'fragment/progress-circle.fs.glsl'],

  ['shader_nav_button_vs',      shader_dir / 'vertex/nav-button.vs.glsl'],
  ['shader_nav_button_fs',      shader_dir / 'fragment/nav-button.fs.glsl'],

  ['shader_crossfade_vs',       shader_dir / 'vertex/crossfade.vs.glsl'],
  ['shader_crossfade_fs',       shader_dir / 'fragment/crossfade.fs.glsl'],
]

resource_header = 'resources.hpp'
resource_source = 'resources.cpp'

resource_input = []
resource_id = []
foreach resfile: resource_files
  resource_input += resfile[1]
  resource_id    += resfile[0]
endforeach

resource_compiler = executable(
  'static-resources',
  '../tools/static-resources.cpp',
  dependencies: [logcerr_dep]
)

generated_resource = custom_target(
  'gen_resource',
  output: [resource_header, resource_source],
  input:  resource_input,
  command: [resource_compiler, '@OUTPUT@', resource_id, '@INPUT@']
)



conf_data = configuration_data()
conf_data.set('VERSION_STR', '"@0@"'.format(meson.project_version()))
configure_file(output: 'build-config.h', configuration: conf_data)







source_files = [
  generated_resource,
  'box.cpp',
  'config.cpp',
  'continuous-scale.cpp',
  'fade-widget.cpp',
  'file-listing.cpp',
  'font-name.cpp',
  'formatting.cpp',
  'fs-watcher.cpp',
  'image-cache.cpp',
  'image-display.cpp',
  'image-source.cpp',
  'image.cpp',
  'infobar.cpp',
  'main.cpp',
  'message-box.cpp',
  'nav-button.cpp',
  'path-compare.cpp',
  'progress-circle.cpp',
  'window.cpp',
]


dependencies = [
  utils_dep,
  logcerr_dep,
  iconfigp_dep,
  gl_dep,
  win_dep,
  dependency('pixglot'),
  dependency('threads'),
  dependency('fontconfig', version: '>=2.14.1')
]


phodispl = executable('phodispl',
  source_files,
  dependencies:        dependencies,
  include_directories: ['../include', '..'],
  gui_app:             true,
  install:             true,
)




if get_option('desktop-file')
  desktop_file = custom_target('desktop_file',
    output:      'PhoDispl.desktop',
    command:     [phodispl, '--desktop-file',
                  get_option('prefix') / get_option('bindir') / 'phodispl'],
    capture:     true,
    install_dir: get_option('datadir') / 'applications',
    install:     true
  )
endif
