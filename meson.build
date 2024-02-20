# Library specification
project('libre-6502', 'c')
inc_dir = include_directories('include')
sources = files(
  'src/addressing.c',
  'src/debug.c',
  'src/decoder.c',
  'src/processor.c',
  )

lib6502 = static_library('6502',
  sources: sources,
  include_directories: inc_dir,
  install: false,
  )

# Tests specification
e0 = executable('test_adc',
  sources: 'tests/test_adc.c',
  include_directories: inc_dir,
  link_with: lib6502,
  )
test('TEST add instruction', e0, is_parallel : true)
