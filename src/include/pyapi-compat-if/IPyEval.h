/**
 * IPyEval.h
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
#pragma once
#include <memory>

namespace pyapi {

// Opaque type used in place of PyObject
struct PyEvalObj;

class IPyEval;
using IPyEvalUP=std::unique_ptr<IPyEval>;
class IPyEval {
public:

    virtual ~IPyEval() { }

    virtual void INCREF(PyEvalObj *obj) = 0;

    virtual void DECREF(PyEvalObj *obj) = 0;

    virtual PyEvalObj *ImportModule(const std::string &name) = 0;

    virtual PyEvalObj *getAttr(PyEvalObj *obj, const std::string &name) = 0;

    virtual bool hasAttr(PyEvalObj *obj, const std::string &name) = 0;

};

} /* namespace pyapi */


