project('libre-nes', 'c')
inc_dir = include_directories('include')
sources = files(
  'src/cartridge.c'         ,
  'src/cpu/addressing.c'    ,
  'src/cpu/decoder.c'       ,
  'src/cpu/processor.c'     ,
  'src/emulator.c'          ,
  'src/main.c'              ,
  'src/picture/ppu.c'       ,
  'src/picture/bus.c'       ,
  'src/picture/registers.c' ,
  'src/reader.c'            ,
)
#sdl2_dep = dependency('sdl2')
executable('libre-nes', sources,
  include_directories: inc_dir,
  #dependencies: sdl2_dep,
)
