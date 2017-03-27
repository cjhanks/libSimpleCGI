# vim: filetype=python
from SCons.Script import *
from os           import getenv
from os.path      import abspath, expanduser, expandvars
from platform     import system
from itertools    import chain



AddOption(
    '--install-bin-prefix',
    type=str,
    action='store',
    help='''
Location to install binary files into.
''')

AddOption(
    '--mode',
    type=str,
    action='store',
    help='''
Use the production build options optimized for the platform
''')

AddOption(
    '--define',
    action='append',
    default=[],
    help=None)

AddOption(
    '--search-path',
    action='append',
    default=[],
    help='''
Add another root search path, it will add ${SEARCH_PATH}/include to includes
and ${SEARCH_PATH}/lib to libs.
''')

AddOption(
    '--avx',
    action='store_true',
    default=False,
    help='''
Enable AVX instructions.
''')

AddOption(
    '--profile-collect',
    action='store_true',
    default=False,
    help='''
Collect the profile arcs
''')

AddOption(
    '--profile-use',
    action='store_true',
    default=False,
    help='''
Collect the profile arcs
''')

AddOption(
    '--wsgi',
    action='store_true',
    help='''
Build the WSGI application server
''')


AddOption('--verbose', action='store_true', help='use verbose output')


if GetOption('mode') == 'production':
    SConscript(
        './SConscript',
        variant_dir='build/release',
        duplicate=1,
    )
elif GetOption('mode') == 'debug':
    SConscript(
        './SConscript',
        variant_dir='build/debug',
        duplicate=1,
    )
else:
    SConscript(
        './SConscript',
        variant_dir='build/default',
        duplicate=1,
    )
