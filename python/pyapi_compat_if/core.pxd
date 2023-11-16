
from libcpp cimport bool
cimport pyapi_compat_if.decl as decl

cdef class Factory(object):
    cdef decl.IFactory       *_hndl

    cpdef PyEval getPyEval(self)

    pass

cdef class PyEval(object):
    cdef decl.IPyEval        *_hndl

    @staticmethod
    cdef PyEval mk(decl.IPyEval *hndl, bool owned=*)
