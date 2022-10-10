project(
    'beast-async', 'cpp',
    version: '0.0.1',
    default_options: [
        'warning_level=3',
        'cpp_std=c++20',
    ]
)

cpp = meson.get_compiler('cpp')

boost_dep = dependency('boost', version: '>=1.74.0')
fmt_dep = dependency('fmt', version: '>=8.1.1')
openssl_dep = dependency('openssl', version: '>=1.1.1') # Prebuilt packages in alpine is 1.1.1, but 3.0.2 recommended

subdir('src')
