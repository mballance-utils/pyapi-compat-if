/*
 * TestBase.cpp
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
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "dmgr/FactoryExt.h"
#include "pyapi-compat-if/FactoryExt.h"
#include "TestBase.h"


namespace pyapi {


TestBase::TestBase() {

}

TestBase::~TestBase() {

}

void TestBase::SetUp() {
    dmgr::IFactory *dmgr_f = dmgr_getFactory();
    m_dmgr = dmgr_f->getDebugMgr();
    m_dmgr->enable(false);
    m_factory = pyapi_compat_if_getFactory();
    m_factory->init(m_dmgr);

    std::vector<std::string> args = ::testing::internal::GetArgvs();

    char tmp[1024];

    fprintf(stdout, "argv[0]=%s\n", args.at(0).c_str());

    realpath(args.at(0).c_str(), tmp);
    fprintf(stdout, "realpath=%s\n", tmp);
    ::testing::internal::FilePath project_dir = ::testing::internal::FilePath(tmp);
    for (uint32_t i=0; i<3; i++) {
        project_dir = project_dir.RemoveFileName();
        project_dir = project_dir.RemoveTrailingPathSeparator();
    }
    fprintf(stdout, "project_dir=%s\n", project_dir.c_str());

    const char *pythonpath = getenv("PYTHONPATH");

    if (!pythonpath || !strstr(pythonpath, project_dir.c_str())) {
        fprintf(stdout, "Need to prepend\n");
        char *pythonpath_env = (char *)malloc(
            ((pythonpath)?strlen(pythonpath)+1:0)
            + project_dir.string().size()
            + strlen("/python")
            + 1);
        strcpy(pythonpath_env, project_dir.c_str());
        strcat(pythonpath_env, "/python");
        if (pythonpath && pythonpath[0]) {
            strcat(pythonpath_env, ":");
            strcat(pythonpath_env, pythonpath);
        }
        setenv("PYTHONPATH", pythonpath_env, 1);
    }

    const char *path = getenv("PATH");
    if (!path || !strstr(path, project_dir.c_str())) {
        // Add 
        fprintf(stdout, "Need to prepend\n");
        char *path_env = (char *)malloc(
            ((path)?strlen(path)+1:0)
            + project_dir.string().size()
            + strlen("/packages/python/bin")
            + 1);
        strcpy(path_env, project_dir.c_str());
        strcat(path_env, "/packages/python/bin");
        if (path && path[0]) {
            strcat(path_env, ":");
            strcat(path_env, path);
        }
        setenv("PATH", path_env, 1);
    }

}

void TestBase::TearDown() {
    fprintf(stdout, "TearDown\n");
    m_factory->reset();
}

void TestBase::enableDebug(bool en) {
    m_dmgr->enable(en);
};

}
