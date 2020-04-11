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

librt = cc.find_library('librt', required: false)
libpthread = cc.find_library('pthread', required: false)
dependencies += [librt, libpthread]

c_args = []
link_args = []

if libpthread.found()
    c_args += ['-pthread']
    link_args += ['-lpthread']
endif

if librt.found()
    link_args += ['-lrt']
endif

executable('scrshare', src,
           dependencies: dependencies,
           include_directories: src_dir,
           install: true,
           c_args: c_args,
           link_args: link_args)
