shader_dir = 'shader'
icon_dir = 'icons'

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


  ['icons_font',                icon_dir   / 'icons-font.otf'],
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
  'generate-resources',
  'generate-resources.cpp',
  dependencies: [logcerr_dep]
)

generated_resources = custom_target(
  'generate_resources',
  output:  [resource_header, resource_source],
  input:   resource_input,
  command: [resource_compiler, '@OUTPUT@', resource_id, '@INPUT@']
)
