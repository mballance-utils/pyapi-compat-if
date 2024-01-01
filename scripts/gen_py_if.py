#!/usr/bin/env python3
#*
#* gen_py_if.py
#*
#* Copyright 2023 Matthew Ballance and Contributors
#*
#* Licensed under the Apache License, Version 2.0 (the "License"); you may 
#* not use this file except in compliance with the License.  
#* You may obtain a copy of the License at:
#*
#*   http://www.apache.org/licenses/LICENSE-2.0
#*
#* Unless required by applicable law or agreed to in writing, software 
#* distributed under the License is distributed on an "AS IS" BASIS, 
#* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
#* See the License for the specific language governing permissions and 
#* limitations under the License.
#*
#* Created on:
#*     Author: 
#*
import argparse
import io
import os
import pcpp
import sys
from cxxheaderparser.simple import parse_string
import cxxheaderparser.types as cxxt
import sysconfig

file_header = """
/**
 * %s
 *
 * Copyright 2023 Matthew Ballance and Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may 
 * not use this file except in compliance with the License.  
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 * Created on:
 *     Author: 
 */
"""

class Preprocessor(pcpp.Preprocessor):

    def on_include_not_found(self, is_malformed, is_system_include, curdir, includepath):
        raise pcpp.parser.OutputDirective(pcpp.parser.Action.IgnoreAndPassThrough)
        pass

def gen_type(t):
    ts = t.format()
    if isinstance(t, cxxt.Type):
        return t.format()
    else:
        return t.format()
#        return "<unknown %s>" % str(t)
    pass

def generate_base_if(out, functions):
    ind = ""

    out.write(file_header % "IPyEvalBase.h")
    out.write("#pragma once\n")
    out.write("#include <stdint.h>\n")
    out.write("#include <stdio.h>\n")
    out.write("#include <stdarg.h>\n")
    out.write("#include <sys/types.h>\n")
    out.write("\n")
    out.write("typedef struct _object PyObject;\n")
    out.write("typedef struct _typeobject PyTypeObject;\n")
    out.write("typedef ssize_t Py_ssize_t;\n")
    out.write("\n")
    out.write("namespace pyapi {\n")
    out.write("\n")
    out.write("\n")
    out.write("class IPyEvalBase {\n")
    out.write("public:\n")
    out.write("\n")
    ind += "    "
    out.write("%svirtual ~IPyEvalBase() { }\n" % ind)
    out.write("\n")

    for i,f in enumerate(functions):
        if i:
            out.write("\n")

        out.write("%svirtual " % ind)
        if f.return_type is not None:
            out.write("%s " % gen_type(f.return_type))
        else:
            out.write("void ")
        out.write("%s(" % f.name.segments[0].name)
        for i,p in enumerate(f.parameters):
            if i > 0:
                out.write(", ")
            if p.name is not None:
                pname = p.name
            else:
                pname = "p%d" % i
            out.write("%s %s" % (gen_type(p.type), pname))

        if f.vararg:
            out.write(" ...")
        out.write(") = 0;\n")
    ind = ind[4:]
    out.write("};\n")
    out.write("\n")
    out.write("}\n")

def generate_ext_base(fp_h, functions):
    ind_h = ""

    fp_h.write(file_header % "PyEvalExtBase.h")

    fp_h.write("#pragma once\n")
    fp_h.write("#include \"pyapi-compat-if/IPyEvalBase.h\"\n")
    fp_h.write("#include \"Python.h\"\n")
    fp_h.write("\n")
    fp_h.write("namespace pyapi {\n")
    fp_h.write("\n")
    fp_h.write("class PyEvalExtBase : public virtual IPyEvalBase {\n")
    fp_h.write("public:\n")
    ind_h += "    "
    fp_h.write("%sPyEvalExtBase();\n" % ind_h)
    fp_h.write("\n")
    fp_h.write("%svirtual ~PyEvalExtBase() { }\n" % ind_h)

    for i,f in enumerate(functions):
        # Override prototype
        if i:
            fp_h.write("\n")
        fp_h.write("%svirtual " % ind_h)
        if f.return_type is not None:
            fp_h.write("%s " % gen_type(f.return_type))
        else:
            fp_h.write("void ")
        fp_h.write("%s(" % f.name.segments[0].name)
        for i,p in enumerate(f.parameters):
            if i > 0:
                fp_h.write(", ")
            if p.name is not None:
                pname = p.name
            else:
                pname = "p%d" % i
            fp_h.write("%s %s" % (gen_type(p.type), pname))

        if f.vararg:
            fp_h.write(" ...")
        fp_h.write(") override {\n")
        ind_h + "    "

        if f.return_type is not None:
            fp_h.write("%sreturn " % ind_h)
        else:
            fp_h.write("%s" % ind_h)
        fp_h.write("::%s(" % f.name.segments[0].name)
        for i,p in enumerate(f.parameters):
            if i > 0:
                fp_h.write(", ")
            if p.name is not None:
                pname = p.name
            else:
                pname = "p%d" % i
            fp_h.write("%s" % pname)
        fp_h.write(");\n")
        ind_h = ind_h[4:]
        fp_h.write("%s}\n" % ind_h)

    fp_h.write("};\n")
    fp_h.write("\n")
    fp_h.write("} // namespace pyapi\n")

def generate_delegator(fp_h, functions):
    ind_h = ""

    fp_h.write(file_header % "PyEvalBaseDelegator.h")

    fp_h.write("#pragma once\n")
    fp_h.write("#include \"pyapi-compat-if/IPyEvalBase.h\"\n")
    fp_h.write("\n")
    fp_h.write("namespace pyapi {\n")
    fp_h.write("\n")
    fp_h.write("class PyEvalBaseDelegator : public virtual IPyEvalBase {\n")
    fp_h.write("public:\n")
    ind_h += "    "
    fp_h.write("%sPyEvalBaseDelegator(IPyEvalBase *core) : m_core(core) { }\n" % ind_h)
    fp_h.write("\n")
    fp_h.write("%svirtual ~PyEvalBaseDelegator() { }\n" % ind_h)
    fp_h.write("\n")

    for i,f in enumerate(functions):
        # Override prototype
        if i:
            fp_h.write("\n")
        fp_h.write("%svirtual " % ind_h)
        if f.return_type is not None:
            fp_h.write("%s " % gen_type(f.return_type))
        else:
            fp_h.write("void ")
        fp_h.write("%s(" % f.name.segments[0].name)
        for i,p in enumerate(f.parameters):
            if i > 0:
                fp_h.write(", ")
            if p.name is not None:
                pname = p.name
            else:
                pname = "p%d" % i
            fp_h.write("%s %s" % (gen_type(p.type), pname))

        if f.vararg:
            fp_h.write(" ...")
        fp_h.write(") override {\n")

        ind_h += "    "

        # Implementation
        if f.return_type is not None:
            fp_h.write("%sreturn " % ind_h)
        else:
            fp_h.write("%s" % ind_h)
        fp_h.write("m_core->%s(" % f.name.segments[0].name)

        for i,p in enumerate(f.parameters):
            if i > 0:
                fp_h.write(", ")
            if p.name is not None:
                pname = p.name
            else:
                pname = "p%d" % i
            fp_h.write("%s" % pname)

        fp_h.write(");\n")
        ind_h = ind_h[4:]
        fp_h.write("%s}\n" % ind_h)
    
    fp_h.write("%sprotected:\n" % ind_h)
    ind_h += "    "
    fp_h.write("%sIPyEvalBase *m_core;\n" % ind_h);
    ind_h = ind_h[4:]

    fp_h.write("%s};\n" % ind_h)
    fp_h.write("\n")
    fp_h.write("%s}\n" % ind_h)
    pass

def generate_cmake(fp_cm):
    fp_cm.write("cmake_minimum_required(VERSION 3.10)\n")
    fp_cm.write("project(pyeval_base)\n")

def main():
    parser = argparse.ArgumentParser(prog="gen_py_if")
    parser.add_argument("outdir", help="Specifies the output directory")

    args = parser.parse_args()

    include_pref = {"Py_", "PyLong_"}
    exclude_pref = {
        "_", "PyAsyncGen_", "PyBuffer_", "PyCapsule_", "PyCode_", "PyComplex_", 
        "PyConfig_", "PyCFunction_", "PyCMethod_", "PyCoro_",
        "PyFile_", "PyFrame_", "PyFunction_", "PyGen_", "PyHash_", 
        "PyMem_", "PyMember_", "PyMemoryView_", "PyModule_", "PyModuleDef_", 
        "PyPickleBuffer_",
        "PyPreConfig_", "PyRun_", "PyState_", "PyStatus_", "PySys_",
        "PyThread_", "PyTraceBack_",
        "PyUnicode_", "PyUnstable_", "PyWideStringList_",
        "PyGILState_", "PyInterpreterState_", "PyThreadState_", "PyStructSequence_",
        "PyWeakref_",
        "PyDescr_" }
    exclude = {
        "Py_INCREF", "Py_DECREF", "Py_Exit", "PyInit__imp",
        "PyModule_AddType",
        "PyFunction_SetVectorcall", "PyVectorcall_Function", 
        "PyVectorcall_NARGS",
        "PyBytes_AS_STRING", "PyBytes_GET_SIZE",
        "PyByteArray_AS_STRING", "PyByteArray_GET_SIZE",
        "PyCell_GET", "PyCell_SET",
        "PyDict_AddWatcher", "PyDict_GET_SIZE",
        "PyEval_AcquireLock", # Deprecated
        "PyEval_CallObjectWithKeywords", # Deprecated
        "PyEval_InitThreads", # Deprecated
        "PyEval_ReleaseLock", # Deprecated
        "PyEval_ThreadsInitialized", # Deprecated
        "PyEval_CallFunction", # Deprecated
        "PyEval_SetProfile", "PyEval_SetProfileAllThreads", 
        "PyEval_SetTrace", "PyEval_SetTraceAllThreads",
        "PyEval_MergeCompilerFlags",
        "Py_NewInterpreter", "Py_EndInterpreter", "Py_NewInterpreterFromConfig",
        "PyEval_GetFrame", "PyEval_EvalFrame", "PyEval_EvalFrameEx",
        "PyEval_SaveThread", "PyEval_RestoreThread", "PyEval_AcquireThread",
        "PyEval_ReleaseThread",
        "PyErr_BadInternalCall",
        "PyFloat_AS_DOUBLE",
        "PyImport_AddAuditHook", "PyImport_AppendInittab", "PyImport_ExtendInittab",
        "PyInstanceMethod_GET_FUNCTION",
        "PyIter_Send",
        "PyList_GET_SIZE", "PyList_SET_ITEM",
        "PyMapping_Length",
        "PyMethod_GET_FUNCTION", "PyMethod_GET_SELF",
        "PyObject_GetBuffer", "Py_SET_SIZE", 
        "PySequence_In", "PySequence_Length",
        "PySet_GET_SIZE",
        "PySlice_GetIndicesEx",
        "PyTuple_GET_SIZE", "PyTuple_SET_ITEM",
        "PyType_Check", "PyType_CheckExact",
        "PyType_FromSpec", "PyType_FromSpecWithBases",
        "PyType_FromModuleAndSpec", "PyType_FromMetaclass", "PyType_GetModuleByDef",
        "PyType_AddWatcher",
        "PyObject_AsCharBuffer", # Deprecated
        "PyObject_AsReadBuffer", # Deprecated
        "PyObject_AsWriteBuffer", # Deprecated
        "PyObject_CheckReadBuffer", # Deprecated
        "PyObject_Hash", "PyObject_HashNotImplemented", "PyObject_InitVar",
        "PyObject_GetArenaAllocator", "PyObject_SetArenaAllocator", "PyObject_Length",
        "PyObject_TypeCheck",
        "Py_CompileStringExFlags", "Py_CompileStringObject", 
        "Py_AddPendingCall",
        "Py_AtExit", 
        "Py_SetPath", # Deprecated
        "Py_SetProgramName", # Deprecated
        "Py_SetPythonHome", # Deprecated
        "Py_SetStandardStreamEncoding", # Deprecated
        "Py_CompileString", "Py_ExitStatusException", "Py_FatalError", "Py_IS_TYPE", "Py_Is",
        "Py_IsFalse", "Py_IsTrue", "Py_IsNone", "Py_NewRef", "Py_REFCNT", "Py_SET_REFCNT",
        "Py_SET_TYPE", "Py_SIZE", "Py_TYPE", "Py_XDECREF", "Py_XINCREF", "Py_XNewRef",
        "PyOS_AfterFork", # Deprecated
        "PyOS_getsig", "PyOS_setsig", "Py_PreInitialize",
        "Py_PreInitializeFromBytesArgs", "Py_PreInitializeFromArgs",
        "Py_InitializeFromConfig", "PyExitStatusException",
        "Py_UNICODE_IS_SURROGATE", "Py_UNICODE_IS_HIGH_SURROGATE", "Py_UNICODE_IS_LOW_SURROGATE",
        "Py_UNICODE_JOIN_SURROGATES", "Py_UNICODE_HIGH_SURROGATE", "Py_UNICODE_LOW_SURROGATE",
        "Py_UNICODE_ISSPACE", "Py_UNICODE_ISALNUM"}
    paths = sysconfig.get_paths()
    py_incdir = paths["include"]
    pp_out = io.StringIO()
    pp = Preprocessor()
    pp.add_path(py_incdir)
    pp.define("UCHAR_MAX 255")
    with open(os.path.join(py_incdir, "Python.h"), "r") as fp:
        pp.parse(fp)

    pp.write(pp_out)

    data = parse_string(pp_out.getvalue())

    print("Namespace: %s" % data.namespace.name)

    functions = []
    for f in data.namespace.functions:
        name = f.name.segments[0].name
        first_under = name.find("_")
        prefix = name[:first_under+1]
#        print("name: %s" % name)
        include = prefix not in exclude_pref and name not in exclude
        include &= not f.vararg

        if include:
#            print("function: %s" % name)
            functions.append(f)

    functions.sort(key=lambda f: f.name.segments[0].name)

    if not os.path.isdir(args.outdir):
        os.makedirs(args.outdir)
    if not os.path.isdir(os.path.join(args.outdir, "include/pyapi-compat-if/impl")):
        os.makedirs(os.path.join(args.outdir, "include/pyapi-compat-if/impl"))

    with open(os.path.join(args.outdir, "include/pyapi-compat-if/IPyEvalBase.h"), "w") as fp:
        generate_base_if(fp, functions)

    with open(os.path.join(args.outdir, "include/pyapi-compat-if/impl/PyEvalExtBase.h"), "w") as fp_h:
        generate_ext_base(fp_h, functions)

    fp_h = open(os.path.join(args.outdir, "include/pyapi-compat-if/impl/PyEvalBaseDelegator.h"), "w")
    generate_delegator(fp_h, functions)
    fp_h.close()

    with open(os.path.join(args.outdir, "CMakeLists.txt"), "w") as fp_cm:
        generate_cmake(fp_cm)

#    print("Output:\n%s" % pp_out.getvalue())
    pass


if __name__ == "__main__":
    main()
