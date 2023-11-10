/**
 * IFactory.h
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
#include <string>
#include "dmgr/IDebugMgr.h"
#include "pyapi-compat-if/IPyEval.h"

namespace pyapi {



class IFactory {
public:

    virtual ~IFactory() { }

    virtual void init(dmgr::IDebugMgr *dmgr) = 0;

    virtual void reset() = 0;

    virtual void setPyEval(IPyEval *eval) = 0;

    virtual IPyEval *getPyEval(std::string &err) = 0;


};

} /* namespace pyapi */


