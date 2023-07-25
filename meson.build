project('libre-nes', 'c')
inc_dir = include_directories('include')
sources = files(
  'src/cpu/addressing.c' ,
  'src/cpu/decoder.c'    ,
  'src/cpu/processor.c'  ,
  'src/cartridge.c'      ,
  'src/emulator.c'       ,
  'src/main.c'           ,
  'src/reader.c'         ,
)
executable('libre-nes', sources, include_directories: inc_dir)
