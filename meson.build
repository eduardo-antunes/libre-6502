project('libre-6502', 'c')
inc_dir = include_directories('include')
sources = files(
  'src/addressing.c',
  'src/decoder.c',
  'src/processor.c',
  'src/computer.c',
  'src/main.c',
)
executable('libre-6502', sources,
  include_directories: inc_dir,
)
