# Library specification

project('libre-6502', 'c')
inc_dir = include_directories('include')
sources = files(
  'src/addressing.c',
  'src/decoder.c',
  'src/processor.c',
  'src/debug.c',
  )

lib6502 = library('6502',
  sources: sources,
  include_directories: inc_dir,
  install: false,
  )

# Tests specification

t0 = executable('adc',
  sources: files('test/adc.c', 'test/utils.c'),
  include_directories: inc_dir,
  link_with: lib6502,
  )
t1 = executable('sbc',
  sources: files('test/sbc.c', 'test/utils.c'),
  include_directories: inc_dir,
  link_with: lib6502,
  )
t2 = executable('adc_dec',
  sources: files('test/adc_dec.c', 'test/utils.c'),
  include_directories: inc_dir,
  link_with: lib6502,
  )
t3 = executable('sbc_dec',
  sources: files('test/sbc_dec.c', 'test/utils.c'),
  include_directories: inc_dir,
  link_with: lib6502,
  )

test('ADC instruction', t0, is_parallel : true)
test('SBC instruction', t1, is_parallel : true)
test('ADC instruction (DECIMAL mode)', t2, is_parallel : true)
test('SBC instruction (DECIMAL mode)', t3, is_parallel : true)
