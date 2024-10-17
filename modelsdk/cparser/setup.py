from setuptools import setup, Extension
from Cython.Build import cythonize

# 定义 Cython 扩展

extensions = [
    Extension("call_parser", sources=["call_parser.pyx", "parser.cpp","aes.cpp","xcb_auth_backend.cpp"],
              language="c++",
              extra_compile_args=["-std=c++11"]),
]

setup(
    name="call_parser",
    ext_modules=cythonize(extensions),
)
