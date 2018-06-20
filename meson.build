project('mlbf', 'c')
add_global_arguments('-std=c11', language: 'c')

incdir = include_directories('src')

sources = [
  'src/mlbf.c',
  'src/interpreter.c',
  'src/program.c',
  'src/compiler.c',
]

dependencies = []

exe = executable(
  'mlbf',
  sources: sources,
  dependencies: dependencies,
  include_directories: incdir,
  # link_args: ''
)
