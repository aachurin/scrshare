src = [
    'src/log.c',
    'src/decoder.c',
    'src/device.c',
    'src/main.c',
    'src/net.c',
    'src/scrshare.c',
    'src/server.c',
    'src/stream.c',
    'src/video_buffer.c'
]

# native build
dependencies = [
    dependency('libavformat'),
    dependency('libavcodec'),
    dependency('libavutil')
]

cc = meson.get_compiler('c')
src += [ 'src/sys/unix/net.c' ]

src_dir = include_directories('src')
c_args = []
link_args = []

executable('scrshare', src,
           dependencies: dependencies,
           include_directories: src_dir,
           install: true,
           c_args: c_args,
           link_args: link_args)
