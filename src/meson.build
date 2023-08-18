conf_data = configuration_data()
conf_data.set('VERSION_STR', '"@0@"'.format(meson.project_version()))
configure_file(output: 'build-config.h', configuration: conf_data)



source_files = [
  generated_resources,
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
