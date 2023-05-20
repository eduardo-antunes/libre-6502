project('alt-libre-nes', 'c')
inc_dir = include_directories('include')
sources = files(
  'src/main.c'      ,
  'src/decoder.c'   ,
  'src/emulator.c'  ,
  'src/processor.c' ,
)

executable('libre-nes', sources, include_directories: inc_dir)
