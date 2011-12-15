from distutils.core import setup, Extension

module1 = Extension('demo',
		    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = ['/usr/local/include/mmseg'],
                    libraries = ['mmseg'],
                    library_dirs = ['/usr/local/lib'],
			
                    sources = ['pymmseg.c','mmseg_interface.cpp'])

setup (name = 'PackageName',
       version = '1.0',
       description = 'This is a demo package',
       ext_modules = [module1])

