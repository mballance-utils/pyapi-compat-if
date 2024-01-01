
from libcpp.string cimport string as cpp_string
cimport debug_mgr.decl as dm

ctypedef IFactory *IFactoryP
ctypedef PyEvalObj *PyEvalObjP

cdef extern from "pyapi-compat-if/IPyEval.h" namespace "pyapi":
    cdef struct PyEvalObj:
        pass
    cdef cppclass IPyEval:
        pass

cdef extern from "pyapi-compat-if/IPyEvalBase.h" namespace "pyapi":
    cdef cppclass IPyEvalBase:
        pass

cdef extern from "PyEvalExt.h" namespace "pyapi":
    cdef cppclass PyEvalExt(IPyEval):
        PyEvalExt(dm.IDebugMgr *)

cdef extern from "pyapi-compat-if/IFactory.h" namespace "pyapi":
    cdef cppclass IFactory:
        void init(dm.IDebugMgr *)
        IPyEval *getPyEval(cpp_string &)
        void setPyEval(IPyEval *)
