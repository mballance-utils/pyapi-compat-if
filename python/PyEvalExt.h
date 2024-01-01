/**
 * PyEvalExt.h
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
#include "dmgr/IDebugMgr.h"
#include "pyapi-compat-if/IPyEval.h"
#include "pyapi-compat-if/impl/PyEvalExtBase.h"

namespace pyapi {



class PyEvalExt : 
    public virtual IPyEval,
    public virtual PyEvalExtBase {
public:
    PyEvalExt(dmgr::IDebugMgr *dmgr);

    virtual ~PyEvalExt();

    virtual int finalize() override;
    
    virtual void flush() override;

    virtual void INCREF(PyEvalObj *obj) override;

    virtual void DECREF(PyEvalObj *obj) override;

    virtual PyEvalObj *importModule(const std::string &name) override;

    virtual PyEvalObj *getAttr(PyEvalObj *obj, const std::string &name) override;

    virtual bool hasAttr(PyEvalObj *obj, const std::string &name) override;

    virtual bool isCallable(PyEvalObj *obj) override;

    virtual PyEvalObj *call(PyEvalObj *obj, PyEvalObj *args, PyEvalObj *kwargs) override;

    virtual PyEvalObj *mkTuple(int32_t sz) override;

    virtual int32_t tupleSetItem(PyEvalObj *obj, uint32_t i, PyEvalObj *val) override;

    virtual PyEvalObj *tupleGetItem(PyEvalObj *obj, uint32_t i) override;

};

}


