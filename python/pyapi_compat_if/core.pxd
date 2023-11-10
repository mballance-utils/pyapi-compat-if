
cimport pyapi_compat_if.decl as decl

cdef class Factory(object):
    cdef decl.IFactory       *_hndl
    pass