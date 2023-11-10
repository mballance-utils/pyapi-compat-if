
import os
import sys
import ctypes
from libc.stdint cimport intptr_t
cimport pyapi_compat_if.decl as decl
cimport debug_mgr.core as dm

cdef Factory _inst = None
cdef class Factory(object):

    @staticmethod
    def inst():
        cdef Factory factory
        cdef decl.PyEvalExt *eval
        cdef dm.Factory dm_f
        global _inst

        if _inst is None:
            ext_dir = os.path.dirname(os.path.abspath(__file__))
            core_lib = os.path.join(ext_dir, "libpyapi-compat-if.so")
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