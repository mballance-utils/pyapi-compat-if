/*
 * TestLoadPyEval.cpp
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
#include "TestLoadPyEval.h"


namespace pyapi {


TestLoadPyEval::TestLoadPyEval() {

}

TestLoadPyEval::~TestLoadPyEval() {

}

TEST_F(TestLoadPyEval, smoke) {
    std::string err;

    enableDebug(false);

    IPyEval *eval = m_factory->getPyEval(err);
    ASSERT_TRUE(eval);

    PyEvalObj *os_m = eval->importModule("os");
    ASSERT_TRUE(os_m);
    eval->DECREF(os_m);

    ASSERT_TRUE(eval->hasAttr(os_m, "environ"));

    enableDebug(true);
}

TEST_F(TestLoadPyEval, smoke2) {
    std::string err;

    enableDebug(true);

    IPyEval *eval = m_factory->getPyEval(err);
    ASSERT_TRUE(eval);

    PyEvalObj *os_m = eval->importModule("os");
    ASSERT_TRUE(os_m);
    eval->DECREF(os_m);

    ASSERT_TRUE(eval->hasAttr(os_m, "environ"));

}

}
