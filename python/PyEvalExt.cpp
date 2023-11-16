/*
 * PyEvalExt.cpp
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
#include "PyEvalExt.h"
#include "Python.h"


namespace pyapi {


PyEvalExt::PyEvalExt(dmgr::IDebugMgr *dmgr) {

}

PyEvalExt::~PyEvalExt() {

}

int PyEvalExt::finalize() {
    PyObject *ext = PyImport_ImportModule("pyapi_compat_if");
    PyObject *del = PyObject_GetAttrString(ext, "reset");
    PyEval_CallFunction(del, "");

    // For testing, at least, finalizing really isn't a good idea
//    return Py_FinalizeEx();
    return 0;
}

void PyEvalExt::flush() {
    PyObject *stream = PySys_GetObject("stdout");
    PyObject *obj = PyObject_GetAttrString(stream, "flush");
    /*PyObject *res =*/ PyObject_Call(obj, PyTuple_New(0), 0);
//    fprintf(stdout, "stream=%p obj=%p res=%p\n", stream, obj, res);
    fflush(stdout);
}

void PyEvalExt::INCREF(PyEvalObj *obj) {
    Py_INCREF(reinterpret_cast<PyObject *>(obj));
}

void PyEvalExt::DECREF(PyEvalObj *obj) {
    Py_DECREF(reinterpret_cast<PyObject *>(obj));
}

PyEvalObj *PyEvalExt::importModule(const std::string &name) {
    return reinterpret_cast<PyEvalObj *>(PyImport_ImportModule(name.c_str()));
}

PyEvalObj *PyEvalExt::getAttr(PyEvalObj *obj, const std::string &name) {
    return reinterpret_cast<PyEvalObj *>(PyObject_GetAttrString(
        reinterpret_cast<PyObject *>(obj),
        name.c_str()));
}

bool PyEvalExt::hasAttr(PyEvalObj *obj, const std::string &name) {
    return PyObject_HasAttrString(
        reinterpret_cast<PyObject *>(obj),
        name.c_str());
}

bool PyEvalExt::isCallable(PyEvalObj *obj) {
    return PyCallable_Check(reinterpret_cast<PyObject *>(obj));
}

PyEvalObj *PyEvalExt::call(PyEvalObj *obj, PyEvalObj *args, PyEvalObj *kwargs) {
    return reinterpret_cast<PyEvalObj *>(PyObject_Call(
        reinterpret_cast<PyObject *>(obj), 
        reinterpret_cast<PyObject *>(args), 
        reinterpret_cast<PyObject *>(kwargs)));
}

PyEvalObj *PyEvalExt::mkTuple(int32_t sz) {
    return reinterpret_cast<PyEvalObj *>(PyTuple_New(sz));
}

int32_t PyEvalExt::tupleSetItem(PyEvalObj *obj, uint32_t i, PyEvalObj *val) {
    return PyTuple_SetItem(
        reinterpret_cast<PyObject *>(obj), 
        i, 
        reinterpret_cast<PyObject *>(val));
}

PyEvalObj *PyEvalExt::tupleGetItem(PyEvalObj *obj, uint32_t i) {
    return reinterpret_cast<PyEvalObj *>(PyTuple_GetItem(
        reinterpret_cast<PyObject *>(obj),
        i));
}

}
