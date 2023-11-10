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

void PyEvalExt::INCREF(PyEvalObj *obj) {
    Py_INCREF(reinterpret_cast<PyObject *>(obj));
}

void PyEvalExt::DECREF(PyEvalObj *obj) {
    Py_DECREF(reinterpret_cast<PyObject *>(obj));
}

PyEvalObj *PyEvalExt::ImportModule(const std::string &name) {
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

}
