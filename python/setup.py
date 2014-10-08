from distutils.core import setup, Extension

module1 = Extension('cmmseg',
		    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = ['/usr/local/include/mmseg'],
                    libraries = ['mmseg'],
                    library_dirs = ['/usr/local/lib'],
			
                    sources = ['pymmseg.c','mmseg_interface.cpp'])

setup (name = 'PackageName',
       version = '1.0',
       description = 'pymmseg, python wrap for libmmseg',
       ext_modules = [module1])

