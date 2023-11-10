/**
 * Factory.h
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
#include "pyapi-compat-if/IFactory.h"

namespace pyapi {



class Factory : public virtual IFactory {
public:
    Factory();

    virtual ~Factory();

    virtual void init(dmgr::IDebugMgr *dmgr) override;

    virtual void setPyEval(IPyEval *eval) override;

    virtual IPyEval *getPyEval(std::string &err) override;

    static IFactory *inst();

private:
    bool find_python_lib(
        void                **python_dll
    );

    bool get_python_info(
        std::string         &python_dll,
        std::string         &pythonpath,
        std::string         &err);

private:
    static std::unique_ptr<IFactory>        m_inst;
    static dmgr::IDebug                     *m_dbg;
    dmgr::IDebugMgr                         *m_dmgr;
    IPyEvalUP                               m_pyeval;

};

}


