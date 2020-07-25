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

    # prebuilt_sdl2 = meson.get_cross_property('prebuilt_sdl2')
    # sdl2_bin_dir = meson.current_source_dir() + '/../prebuilt-deps/' + prebuilt_sdl2 + '/bin'
    # sdl2_lib_dir = meson.current_source_dir() + '/../prebuilt-deps/' + prebuilt_sdl2 + '/lib'
    # sdl2_include_dir = '../prebuilt-deps/' + prebuilt_sdl2 + '/include'

    # sdl2 = declare_dependency(
    #     dependencies: [
    #         cc.find_library('SDL2', dirs: sdl2_bin_dir),
    #         cc.find_library('SDL2main', dirs: sdl2_lib_dir),
    #     ],
    #     include_directories: include_directories(sdl2_include_dir)
    # )

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
        # sdl2,
        cc.find_library('mingw32')
    ]

endif

if host_machine.system() == 'windows'
    dependencies += cc.find_library('ws2_32')
endif


cc = meson.get_compiler('c')
src_dir = include_directories('src')

if get_option('windows_noconsole')
    link_args = [ '-Wl,--subsystem,windows' ]
else
    link_args = []
endif


executable('scrshare', src,
           dependencies: dependencies,
           include_directories: src_dir,
           install: true,
           c_args: [],
           link_args: link_args)
