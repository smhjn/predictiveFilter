from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
import numpy as np
import os

os.environ["CC"] = "g++" 
os.environ["CXX"] = "g++"

ext_modules=[
    Extension(name = "PyPredictiveFilter",
              sources=["PyPredictiveFilter.pyx"],
              include_dirs = ["."]+ [np.get_include()],
              libraries=["predictivefilter"],
	          library_dirs =["."],
	          language="c++") # Unix-like specific
]

setup(
    cmdclass = {'build_ext': build_ext},
    ext_modules = ext_modules
)
