project('FroggyFrag', 'c',
    version: '0.1.0',
    default_options: [
        'c_std=c99',
        'warning_level=3',
        'werror=true'
    ]
)

# Find raylib dependency
cc = meson.get_compiler('c')
raylib_dep = dependency('raylib', required: false)
if not raylib_dep.found()
    raylib_lib = cc.find_library('raylib', required: false)
endif

# Include directories
inc = include_directories('src')

# Source files
sources = files(
    'src/main.c',
    'src/conf.c',
    'src/gui.c',
    'src/animation.c',
    'src/ease.c'
)

# Build the executable
if raylib_dep.found()
    executable('FroggyFrag',
        sources,
        include_directories: inc,
        dependencies: [raylib_dep],
        install: true
    )
else
    message('raylib not found - skipping main build')
endif

# Tests
subdir('tests')
