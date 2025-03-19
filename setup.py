#****************************************************************************
#* setup.py for pyapi-compat-if
#****************************************************************************
import os
import sys
from setuptools import Extension, find_namespace_packages

version="0.0.1"

proj_dir = os.path.dirname(os.path.abspath(__file__))

try:
    import sys
    sys.path.insert(0, os.path.join(proj_dir, "python"))
    from pyapi_compat_if.__build_num__ import BUILD_NUM
    version += ".%s" % str(BUILD_NUM)
except ImportError:
    pass

isSrcBuild = False

try:
    from ivpm.setup import setup
    isSrcBuild = os.path.isdir(os.path.join(proj_dir, "src"))
    print("pyapi-compat-if: running IVPM")
except ImportError as e:
    from setuptools import setup
    print("pyapi-compat-if: running setuptools: %s" % str(e))

if isSrcBuild:
    incdir = os.path.join(proj_dir, "src", "include")
else:
    incdir = os.path.join(proj_dir, "python/pyapi_compat_if/share/include")

pyapi_compat_if_dir = proj_dir

ext = Extension("pyapi_compat_if.core",
            sources=[
                os.path.join(pyapi_compat_if_dir, 'python', "core.pyx"), 
                os.path.join(pyapi_compat_if_dir, 'python', 'PyEvalExt.cpp'),
            ],
            language="c++",
            include_dirs=[
                os.path.join(pyapi_compat_if_dir, 'src', 'include'),
                os.path.join(pyapi_compat_if_dir, 'build/src/pyeval_base/include')
            ]
        )
ext.cython_directives={'language_level' : '3'}

setup_args = dict(
  name = "pyapi-compat-if",
  version=version,
  packages=['pyapi_compat_if'],
  package_dir = {'' : 'python'},
  author = "Matthew Ballance",
  author_email = "matt.ballance@gmail.com",
  description = ("Core Verification Stimulus and Coverage library"),
  long_description="""
  Provides a library for constrained randomization and coverage collection
  """,
  license = "Apache 2.0",
  keywords = ["SystemVerilog", "Verilog", "RTL", "Python"],
  url = "https://github.com/mballance-utils/pyapi-compat-if",
  package_data = {'pyapi_compat_if': [
      'core.pxd',
      'decl.pxd'
  ]},
  install_requires=[
      'debug-mgr'
  ],
  setup_requires=[
    'cython',
    'debug-mgr',
    'ivpm',
    'setuptools_scm',
  ],
  entry_points={
    "ivpm.pkginfo": [
        "pyapi-compat-if = pyapi_compat_if.pkginfo:PkgInfo"
    ]
  },
  ext_modules=[ ext ]
)

if isSrcBuild:
    setup_args["ivpm_extdep_pkgs"] = ["debug-mgr"]
    setup_args["ivpm_extra_data"] = {
        "pyapi_compat_if": {
            ("src/include", "share"),
            ("build/include", "share"),
            ("python/PyEvalExt.h", "share/include"),
            ("build/{libdir}/{libpref}pyapi-compat-if{dllext}", "")
        }
    }

setup(**setup_args)



