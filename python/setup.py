from distutils.core import setup, Extension
import sys

define_macros = [('MAJOR_VERSION', '1'),
                 ('MINOR_VERSION', '0')]

extra_compile_args=[] 
extra_link_args=[]                  

# Ref: https://code.google.com/p/cefpython/source/browse/cefpython1/setup/setup.py

if sys.platform == "win32":
    include_dirs = ['../libcss/include']
    library_dirs = ['../libcss/lib']
    libraries = ['libcss']
    define_macros.append(('WIN32', 1))
    extra_compile_args=['/EHsc'] # '/EHsc', '/clr'
    extra_link_args=['/NODEFAULTLIB:libcmt', '/NODEFAULTLIB:msvcprt', '/ignore:4217'] # '/ignore:4217'   
else:
    include_dirs = ['/usr/local/include/mmseg']
    library_dirs = ['/usr/local/lib']
    libraries = ['mmseg']

module1 = Extension('cmmseg',
		    define_macros = define_macros,
                    include_dirs = include_dirs,
                    libraries = libraries,
                    library_dirs = library_dirs,
                    extra_compile_args = extra_compile_args,
                    extra_link_args = extra_link_args,
                    sources = ['pymmseg.c','mmseg_interface.cpp'])

setup (name = 'mmseg4py',
       version = '1.0',
       description = 'pymmseg, python wrap for libmmseg',
       ext_modules = [module1])

