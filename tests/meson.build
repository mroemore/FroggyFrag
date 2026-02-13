# Test build configuration

unity_inc = include_directories('unity/src')
unity_src = files('unity/src/unity.c')

test_quality_src = files('test_animation_quality.c')

# Include paths for source files
src_inc = include_directories('../src')

# Test executable
test_quality_exe = executable('test_quality',
    test_quality_src + unity_src + ['../src/animation.c', '../src/ease.c'],
    include_directories: [unity_inc, src_inc],
    c_args: [
        '-Wall',
        '-Wextra',
        '-Wpedantic',
        '-Werror',
        '-std=c99'
    ],
    link_args: ['-lm']
)

test('quality_tests', test_quality_exe)
