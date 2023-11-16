
import os
import sys
import ctypes
from libc.stdint cimport intptr_t
from libcpp.string cimport string as cpp_string
cimport pyapi_compat_if.decl as decl
cimport debug_mgr.core as dm

cdef Factory _inst = None
cdef class Factory(object):

    def __del__(self):
        print("Factory.__del__");

    cpdef PyEval getPyEval(self):
        cdef decl.IPyEval *pyeval
        cdef cpp_string err

        pyeval = self._hndl.getPyEval(err)

        if pyeval == NULL:
            raise Exception("Failed to get interpreter: %s" % err.decode())

        return PyEval.mk(pyeval, False)

    @staticmethod
    def reset():
        global _inst
        _inst = None

    @staticmethod
    def inst():
        cdef Factory factory
        cdef decl.PyEvalExt *eval
        cdef dm.Factory dm_f
        global _inst

        if _inst is None:
            ext_dir = os.path.dirname(os.path.abspath(__file__))
            build_dir = os.path.abspath(os.path.join(ext_dir, "../../build"))

            # First, look in the build directory
            core_lib = None
            libname = "libpyapi-compat-if.so"
            for libdir in ("lib", "lib64"):
                if os.path.isfile(os.path.join(build_dir, libdir, libname)):
                    core_lib = os.path.join(build_dir, libdir, libname)
                    break
            if core_lib is None:
                core_lib = os.path.join(ext_dir, libname)

            if not os.path.isfile(core_lib):
                raise Exception("Extension library core \"%s\" doesn't exist" % core_lib)
            so = ctypes.cdll.LoadLibrary(core_lib)
            func = so.pyapi_compat_if_getFactory
            func.restype = ctypes.c_void_p

            hndl = <decl.IFactoryP>(<intptr_t>(func()))

            dm_f = dm.Factory.inst()

            eval = new decl.PyEvalExt(dm_f.getDebugMgr()._hndl)
            hndl.setPyEval(eval)

            factory = Factory()
            factory._hndl = hndl
            _inst = factory

        return _inst
    pass

def __del__():
    print("core.pyx::__del__")

cdef class PyEval(object):

    @staticmethod
    cdef PyEval mk(decl.IPyEval *hndl, bool owned=True):
        ret = PyEval()
        ret._hndl = hndl
        return ret

