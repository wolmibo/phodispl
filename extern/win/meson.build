config = configuration_data()

deps = [
  utils_dep,
  logcerr_dep,
  dependency('epoxy', version: '>=1.5.10')
]

sources = [
  'src/application.cpp',
  'src/window-native.cpp'
]


#Wayland stuff

wlscanner = dependency('wayland-scanner',
  native:   true,
  required: get_option('wayland-native'),
  version:  '>=1.21.0'
)

scanner = find_program(wlscanner.get_variable('wayland_scanner'),
                           required: get_option('wayland-native'))

wlprotocols = dependency('wayland-protocols',
  required: get_option('wayland-native'),
  version:  '>=1.31'
)

wl_deps = [
  dependency('wayland-client', required: get_option('wayland-native'), version: '>=1.21.0'),
  dependency('wayland-egl',    required: get_option('wayland-native'), version: '>=18.1.0'),
  dependency('egl',            required: get_option('wayland-native'), version: '>=1.5'),
  dependency('xkbcommon',      required: get_option('wayland-native'), version: '>=1.4.1')
]

wlavailable = wlscanner.found() and scanner.found() and wlprotocols.found()
foreach dep: wl_deps
  wlavailable = wlavailable and dep.found()
endforeach

backends = []

if wlavailable
  scanner_src = generator(
    scanner,
    output: '@BASENAME@-protocol.c',
    arguments: ['private-code', '@INPUT@', '@OUTPUT@'],
  )

  scanner_header = generator(
    scanner,
    output: '@BASENAME@-client-protocol.h',
    arguments: ['client-header', '@INPUT@', '@OUTPUT@'],
  )

  protocols = [
    'stable/viewporter/viewporter.xml',
    'stable/xdg-shell/xdg-shell.xml',
    'unstable/xdg-decoration/xdg-decoration-unstable-v1.xml',
    'unstable/pointer-gestures/pointer-gestures-unstable-v1.xml',
  ]

  proto_root = wlprotocols.get_variable('pkgdatadir')
  proto_source = []

  foreach proto: protocols
    proto_source += scanner_src.process(proto_root / proto)
    proto_source += scanner_header.process(proto_root / proto)
  endforeach

  sources += [
    proto_source,
    'src/context-wayland.cpp',
    'src/global-egl.cpp',
    'src/global-wayland.cpp',
    'src/input-manager-wayland.cpp',
    'src/window-wayland.cpp',
  ]

  deps += wl_deps

  config.set('WIN_WITH_BACKEND_WAYLAND', 1)
  backends += 'wayland-native'
endif

glfw = dependency('glfw3', required: get_option('glfw'), version: '>=3.3.8')
if glfw.found()
  deps    += glfw
  sources += [
    'src/window-glfw.cpp',
    'src/context-glfw.cpp',
  ]

  config.set('WIN_WITH_BACKEND_GLFW', 1)
  backends += 'glfw'
endif



configure_file(output: 'win-config.h', configuration: config)


win_lib = static_library(
  'win',
  sources,
  include_directories: ['include'],
  dependencies:        deps
)

win_dep = declare_dependency(
  link_with:           win_lib,
  dependencies:        deps,
  include_directories: ['include']
)
