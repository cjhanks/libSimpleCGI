# vim: filetype=python

from SCons.Script import *
from os.path      import abspath, basename, join, exists
from os           import getenv
from platform     import system
from functools    import partial
from itertools    import chain


# -- CONFIGURE AESTHETICS OF BUILD ------------------------------------ #
Env = Environment(ENV={'PATH': getenv('PATH')},
                  tools=['default'])
Env['BUILDROOT'] = Dir('.')

if getenv('CXX'):
    Env['CXX'] = getenv('CXX')

COLORS = {
    'cyan'  : '\033[96m',
    'purple': '\033[95m',
    'blue'  : '\033[94m',
    'green' : '\033[92m',
    'yellow': '\033[93m',
    'red'   : '\033[91m',
    'end'   : '\033[0m'
}

compile_str = \
        '{0}Compile:{1} $SOURCE'.format(COLORS['green'], COLORS['end'])
linking_str = \
        '{0}Linking:{1} $SOURCE'.format(COLORS['yellow'], COLORS['end'])
generating_str = \
        '{0}Chewing:{1} $SOURCE'.format(COLORS['cyan'], COLORS['end'])
install_str = \
        '{0}Install:{1} $SOURCE'.format(COLORS['red'], COLORS['end'])

if not GetOption('verbose'):
    Env.Append(
        CXXCOMSTR=compile_str,
        CCCOMSTR=compile_str,
        SHCXXCOMSTR=compile_str,
        SHCCCOMSTR=compile_str,
        ARCOMSTR=linking_str,
        RANLIBCOMSTR=linking_str,
        SHLINKCOMSTR=linking_str,
        LINKCOMSTR=linking_str,
        AVROCOMSTR=generating_str,
        PROTOCOMSTR=generating_str,
        INSTALLSTR=install_str,
        INSTALLCOMSTR=install_str,
        )


# -- CONFIGURE PATHS FOR LIBRARIES AND INCLUDES ----------------------- #
all_search_paths = [
    '/usr/',
    '/usr/local/',
]
for path in GetOption('search_path'):
    all_search_paths.append(abspath(path))


# {
inc_paths_root = map(lambda x: join(x, 'include'), all_search_paths)
inc_postfixes = [
]
inc_paths = inc_paths_root[:]
for postfix in inc_postfixes:
    inc_paths.extend(map(lambda x: join(x, postfix), inc_paths_root))

inc_paths.extend([
    Dir('#$BUILDROOT'),
    Dir('#'),
])
Env.Append(CPPPATH=inc_paths)
# }

# {
lib_paths = map(lambda x: join(x, 'lib'), all_search_paths)
lib_postfixes = [
    'lib',
    'lib64',
]
for postfix in lib_postfixes:
    lib_paths.extend(map(lambda x: join(x, postfix), lib_paths))

lib_paths = [l for l in lib_paths if exists(l)]
Env.Append(LIBPATH=lib_paths)
# }

# -- CONFIGURE OPTIMIZATION FLAGS ------------------------------------- #
Env.Append(CPPDEFINES=GetOption('define'))
Env.Append(CXXFLAGS=[
    '-Wall',
    '-Wextra',
    '-std=c++11',

])

Env.Append(CPPFLAGS=[
    '-pthread',
    '-pipe',
])

Env.Append(LINKFLAGS=[
    '-pthread'
])

if GetOption('mode') == 'production':
    Env.Append(CXXFLAGS=[
        '-O3',
    ])

    if GetOption('avx'):
        Env.Append(CXXFLAGS=[
            '-march=corei7-avx',
            '-mtune=corei7-avx',
        ])
    else:
        Env.Append(CXXFLAGS=[
            '-march=corei7',
            '-mtune=corei7',
        ])

    Env.Append(LINKFLAGS=[
        '-flto',
    ])
    Env.Append(CPPDEFINES=[
        'NDEBUG',
    ])

elif GetOption('mode') == 'debug':
    #Env.Append(CPPDEFINES=[
    #    'D_GLIBCXX_DEBUG=',
    #])
    sanitization = [
        'address',
        'undefined',
    ]
    Env.Append(CPPFLAGS=[
        '-fsanitize={}'.format(f) for f in sanitization])
    Env.Append(CPPFLAGS=[
        '-fsanitize-recover',
        '-fstack-protector',
    ])
    Env.Append(LIBS=[
        'asan',
        'ubsan',
    ])

    Env.Append(CXXFLAGS=[
        '-g',
        '-mtune=generic',
        '-O1',
    ])

else:
    Env.Append(CXXFLAGS=[
        '-g',
        '-O2',
        '-mtune=corei7'
    ])

# -- CONFIGURE LINKER FLAGS ------------------------------------------- #
libs = [
]
Env.Append(LIBS=libs)

# some pkg-config things...
pkg_configs = [
]

for pkg in pkg_configs:
    Env.ParseConfig('pkg-config --cflags --libs {}'.format(pkg))

if GetOption('wsgi'):
    Env.Append(CPPDEFINES=[
        'WITH_WSGI=1'
    ])
    Env.Append(LIBS=[
        'boost_program_options',
    ])
    Env.ParseConfig('python3-config --cflags --ldflags')


# -- BUILD STATIC LIBRARY --------------------------------------------- #
Env = Env.Clone()
BinOutput = Dir('#bin').abspath

libSimpleCGI = \
        Env.SConscript('SimpleCGI/SConscript', exports={'Env': Env})

Env = Env.Clone()
Env.Prepend(LIBS=[libSimpleCGI])

# -- BUILD OPTIONAL APPLICATIONS -------------------------------------- #

Env.SConscript('application/SConscript', exports={'Env':Env})
