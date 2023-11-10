# pyapi-compat-if
Implements a compatibilty interface allowing C++ code to call Python without being tied to a specific Python version

## Use Case
Python extension modules are fairly tightly bound to a specific Python 
interpreter and version. This is fine in many cases, but causes an 
issue when a native library wants to use Python without being tightly
coupled to a specific Python interp/version.

The implementation of pyapi-compat-if has two pieces:
- A C++ pure-virtual class API that roughly follows the CPython API.
  This API is independent of a Python interpreter and version. 
- A Python extension that *is* tightly bound to a specific interp/version.
  The interp/version-specific extension provides an implementation of the
  C++ API.

## Use Model
The core C++ API provides a mechanism to discover the Python interpreter
and load the pyapi-compat-if extension module into it. When the extension
is loaded, it registers the interp/version-specific version of the C++
API implementation. The C++ application can now evaluate code in the 
interpreter.


