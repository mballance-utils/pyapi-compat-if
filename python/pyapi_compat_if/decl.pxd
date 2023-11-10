
cimport debug_mgr.decl as dm

ctypedef IFactory *IFactoryP

cdef extern from "pyapi-compat-if/IPyEval.h" namespace "pyapi":
    cdef cppclass IPyEval:
        pass

cdef extern from "pyapi-compat-if/IFactory.h" namespace "pyapi":
    cdef cppclass IFactory:
        void init(dm.IDebugMgr *)
        IPyEval *getPyEval()
        void setPyEval(IPyEval *)
