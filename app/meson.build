src = [
    'src/log.c',
    'src/decoder.c',
    'src/device.c',
    'src/main.c',
    'src/net.c',
    'src/scrshare.c',
    'src/video_server.c',
    'src/receiver_server.c',
    'src/stream.c',
    'src/video_buffer.c'
]

if not get_option('crossbuild_windows')
    # native build
    dependencies = [
        dependency('libavformat'),
        dependency('libavcodec'),
        dependency('libavutil')
    ]
else
    # cross-compile mingw32 build (from Linux to Windows)
    cc = meson.get_compiler('c')

    prebuilt_ffmpeg_shared = meson.get_cross_property('prebuilt_ffmpeg_shared')
    prebuilt_ffmpeg_dev = meson.get_cross_property('prebuilt_ffmpeg_dev')
    ffmpeg_bin_dir = meson.current_source_dir() + '/../prebuilt-deps/' + prebuilt_ffmpeg_shared + '/bin'
    ffmpeg_include_dir = '../prebuilt-deps/' + prebuilt_ffmpeg_dev + '/include'
    ffmpeg = declare_dependency(
        dependencies: [
            cc.find_library('avcodec-58', dirs: ffmpeg_bin_dir),
            cc.find_library('avformat-58', dirs: ffmpeg_bin_dir),
            cc.find_library('avutil-56', dirs: ffmpeg_bin_dir),
        ],
        include_directories: include_directories(ffmpeg_include_dir)
    )

    dependencies = [
        ffmpeg,
        cc.find_library('mingw32')
    ]

endif

if host_machine.system() == 'windows'
    dependencies += cc.find_library('ws2_32')
endif


cc = meson.get_compiler('c')
src += [ 'src/sys/unix/net.c' ]

src_dir = include_directories('src')

if get_option('windows_noconsole')
    link_args = [ '-Wl,--subsystem,windows' ]
else
    link_args = []
endif


#librt = cc.find_library('librt', required: false)
libpthread = cc.find_library('pthread', required: false)
dependencies += [libpthread]

c_args = []
link_args = []

if libpthread.found()
    c_args += ['-pthread']
    link_args += ['-lpthread']
endif

#if librt.found()
#    link_args += ['-lrt']
#endif

executable('scrshare', src,
           dependencies: dependencies,
           include_directories: src_dir,
           install: true,
           c_args: c_args,
           link_args: link_args)
