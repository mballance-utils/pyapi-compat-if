/*
 * Factory.cpp
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
#include <string>
#include <string.h>
#include <unordered_set>
#include <vector>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
// THE #undefs BELOW ARE LOAD-BEARING. windows.h defines INCREF and DECREF as
// macros -- measured on the CI runner, and note it does so EVEN WITH
// WIN32_LEAN_AND_MEAN, so the defines above do not save you. IPyEval.h
// declares INCREF() and DECREF() members, and the macros rewrite those
// declarations before the compiler sees them: MSVC ends up reading
// `virtual void (PyEvalObj *obj) = 0;`, reports "missing ')' before '*'", then
// invents an `int PyEvalObj` member that poisons every later use of the type.
//
// This is the only translation unit that includes windows.h and it was the
// only one that failed. Renaming the two interface methods is the alternative
// fix; that is a public API break over a name that belongs to windows.h.
#undef INCREF
#undef DECREF
#else
#include <poll.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dlfcn.h>
#endif
#include "dmgr/impl/DebugMacros.h"
#include "pyapi-compat-if/FactoryExt.h"
#include "Factory.h"


namespace pyapi {

namespace {

// Windows has no dlopen/dlsym, and these two calls are the whole of what this
// file needs from the POSIX dynamic loader. The call sites below used them
// unguarded -- the #ifdef _WIN32 further down covers find_python_lib and
// get_python_info but never covered these -- so the library did not compile on
// Windows at all, which is why the Windows CI leg was commented out rather
// than merely failing.
void *dll_open(const char *path) {
#ifdef _WIN32
    return reinterpret_cast<void *>(LoadLibraryA(path));
#else
    // Extension libraries don't explicitly link Python, so must be loaded
    // with global symbols.
    return dlopen(path, RTLD_LAZY|RTLD_GLOBAL);
#endif
}

void *dll_sym(void *lib, const char *name) {
#ifdef _WIN32
    return reinterpret_cast<void *>(
        GetProcAddress(reinterpret_cast<HMODULE>(lib), name));
#else
    return dlsym(lib, name);
#endif
}

}


Factory::Factory() : m_dmgr(0) {

}

Factory::~Factory() {

}

void Factory::init(dmgr::IDebugMgr *dmgr) {
    m_dmgr = dmgr;
    DEBUG_INIT("pyapi::Factory", dmgr);
}

void Factory::reset() {
    DEBUG_ENTER("reset");
    if (m_pyeval) {
        DEBUG("Finalizing");
        // Drop the reference we've been holding to the extension
        m_pyeval->DECREF(m_ext_ref);
        m_pyeval->finalize();
        m_pyeval.reset();
    }
    DEBUG_LEAVE("reset");
}

void Factory::setPyEval(IPyEval *eval) {
    DEBUG_ENTER("setPyEvalBase");
    m_pyeval = IPyEvalUP(eval);
    DEBUG_LEAVE("setPyEval");
}

IPyEval *Factory::getPyEval(std::string &err) {
    DEBUG_ENTER("getPyEval");
    if (!m_pyeval) {
        std::string python_dll;
        std::string pythonpath;
        err.clear();
        void *python_dll_lib = 0;

        if (!find_python_lib(&python_dll_lib)) {
            // Didn't find the library, so search now
            if (!get_python_info(python_dll, pythonpath, err)) {
                return 0;
            }
            fprintf(stdout, "python_dll=%s\n", python_dll.c_str());
            python_dll_lib = dll_open(python_dll.c_str());
        } else {
            // Have the library
        }

        if (!python_dll_lib) {
            err = "Failed to load Python library";
            return 0;
        }

        DEBUG_ENTER("Call Py_Initialize");
        void *pyinit = dll_sym(python_dll_lib, "Py_Initialize");
        void (*pyinit_f)() = (void (*)())pyinit;
        void *(*pygetattr_f)(void *, const char *) = 
            (void *(*)(void *, const char *))dll_sym(python_dll_lib, "PyObject_GetAttrString");
        void *(*pycall_f)(void *, const char *, ...) =
            (void *(*)(void *, const char *, ...))dll_sym(python_dll_lib, "PyEval_CallFunction");

        if (!pyinit_f) {
            err = "Failed to load Py_Initialize";
            return 0;
        }

        pyinit_f();
        DEBUG_LEAVE("Call Py_Initialize");

        DEBUG_ENTER("Import pyapi package");
        void *pyimport = dll_sym(python_dll_lib, "PyImport_ImportModule");
        void *(*pyimport_f)(const char *) = (void *(*)(const char *))pyimport;

        if (!pyimport || !pyimport_f) {
            err = "Failed to find PyImport_ImportModule";
            return 0;
        }

        m_ext_ref = reinterpret_cast<PyEvalObj *>(pyimport_f("pyapi_compat_if"));
        DEBUG_LEAVE("Import pyapi package");

        void *init_o = pygetattr_f(m_ext_ref, "init");
        DEBUG_ENTER("pycall_f %p", init_o);
        pycall_f(init_o, "");
        DEBUG_LEAVE("pycall_f %p", init_o);

        if (!m_pyeval) {
            DEBUG("Python extension did not set the PyEval handle");
            err = "Python extension did not set the PyEval handle";
        } else {
//            m_pyeval->INCREF(m_ext_ref);
        }
    }

    DEBUG_LEAVE("getPyEval %p", m_pyeval.get());
    return m_pyeval.get();
}

IFactory *Factory::inst() {
    if (!m_inst) {
        m_inst = std::unique_ptr<IFactory>(new Factory());
    }
    return m_inst.get();
}

bool Factory::find_python_lib(void **python_dll) {

    *python_dll = 0;

#ifdef _WIN32
    // The Windows analogue of the /proc/self/maps scan below, and a tidier one:
    // ask the loader for the modules in this process and test each for
    // Py_Initialize. Note this does NOT take a reference the way the POSIX
    // branch's dlopen does -- the handle is borrowed from the loader, which is
    // what we want, since we are looking for a Python that someone else loaded.
    {
        HMODULE modules[1024];
        DWORD   needed = 0;

        if (EnumProcessModules(
                GetCurrentProcess(), modules, sizeof(modules), &needed)) {
            DWORD n = needed / sizeof(HMODULE);
            if (n > (sizeof(modules)/sizeof(modules[0]))) {
                n = sizeof(modules)/sizeof(modules[0]);
            }
            for (DWORD i=0; i<n; i++) {
                if (GetProcAddress(modules[i], "Py_Initialize")) {
                    *python_dll = reinterpret_cast<void *>(modules[i]);
                    break;
                }
            }
        }
    }
#else
    /**
     * Find the libraries loaded by the process
     */
    pid_t pid = getpid();
    char mapfile_path[128];

    sprintf(mapfile_path, "/proc/%d/maps", pid);
    FILE *map_fp = fopen(mapfile_path, "r");

    std::unordered_set<std::string> so_files;
    while (fgets(mapfile_path, sizeof(mapfile_path), map_fp)) {
        std::string path = mapfile_path;
        int32_t idx;

        if ((idx=path.find('/')) != std::string::npos) {
            int32_t eidx = path.size()-1;
            while (isspace(path.at(eidx))) {
                eidx--;
            }
            path = path.substr(idx, (eidx-idx+1));

            struct stat statbuf;

            if (stat(path.c_str(), &statbuf) == -1) {
                // Doesn't exist. Read another line to complete the path
                if (fgets(mapfile_path, sizeof(mapfile_path), map_fp)) {
                        path.append(mapfile_path);

                        int32_t eidx = path.size()-1;
                        while (isspace(path.at(eidx))) {
                            eidx--;
                        }

                        if (eidx < path.size()-1) {
                            path = path.substr(0, eidx+1);
                        }
                }
            }
            if (path.find(".so") != std::string::npos) {
                if (so_files.insert(path).second) {
                    void *lib = dlopen(path.c_str(), RTLD_LAZY);
                    void *py_init = dlsym(lib, "Py_Initialize");

                    if (py_init) {
                        *python_dll = lib;
                    }
                }
            }
        }

        if (*python_dll) {
            break;
        }
    }
    fclose(map_fp);
#endif

    return *python_dll;
}

bool Factory::get_python_info(
        std::string         &python_dll,
        std::string         &pythonpath,
        std::string         &err) {
       // Need to extract:
        // - Python native-library path
        // - Python path elements

        std::vector<std::string> cmdline;

        cmdline.push_back("python3");
        cmdline.push_back("-m");
        cmdline.push_back("sysconfig");
        std::string ldlibrary;
        std::string libdir;

#ifdef _WIN32
    // Do NOT try to reconstruct the DLL path from sysconfig the way the POSIX
    // branch does. LDLIBRARY and LIBDIR are POSIX-only config vars --
    // sysconfig._init_non_posix sets only LIBDEST, BINLIBDEST, INCLUDEPY,
    // EXT_SUFFIX, EXE, VERSION, BINDIR and TZPATH -- so the parse below would
    // find nothing on Windows however it was spelled.
    //
    // Nor is guessing base_prefix + "python{ver}.dll" good enough: the layout
    // differs between the python.org installer, the Microsoft Store package,
    // the embeddable package and a venv, and the name carries a 't' suffix for
    // free-threaded builds and '_d' for debug ones. The docs do not specify a
    // location to rely on.
    //
    // So ask the interpreter where its own DLL is. sys.dllhandle is documented
    // as "Integer specifying the handle of the Python DLL. Availability:
    // Windows", and GetModuleFileNameW turns that into the path the loader
    // actually used. That is correct for every layout above, by construction.
    //
    // The -c body deliberately contains no quotes of its own, so the whole
    // thing survives cmd.exe with one level of quoting.
    {
        static const char *PY_DLL_QUERY =
            " -c \"import sys,ctypes;"
            "b=ctypes.create_unicode_buffer(32768);"
            "ctypes.windll.kernel32.GetModuleFileNameW("
            "ctypes.c_void_p(sys.dllhandle),b,len(b));"
            "print(b.value)\"";

        // python3 is usually NOT on PATH on Windows; python.exe is, and the
        // py launcher is the fallback when several versions are installed.
        const char *launchers[] = {"python", "py -3"};

        for (size_t i=0; i<sizeof(launchers)/sizeof(launchers[0]); i++) {
            std::string cmd = std::string(launchers[i]) + PY_DLL_QUERY;

            // _popen rather than the CreateProcess/pipe plumbing the POSIX
            // branch needs. Caveat: it requires a console subsystem, so a GUI
            // host would need the longer road.
            FILE *fp = _popen(cmd.c_str(), "r");
            if (!fp) {
                continue;
            }

            char buf[4096];
            std::string out;
            while (fgets(buf, sizeof(buf), fp)) {
                out += buf;
            }
            int rc = _pclose(fp);

            while (out.size() && (out.back() == '\n' || out.back() == '\r')) {
                out.pop_back();
            }

            DEBUG("launcher=%s rc=%d dll=%s", launchers[i], rc, out.c_str());

            if (rc == 0 && out.size()) {
                python_dll = out;
                return true;
            }
        }

        // The POSIX branch leaves err untouched on failure, which surfaces to
        // the caller as an empty message. Say something useful instead.
        err = "could not locate the Python DLL: neither `python` nor `py -3` "
              "answered a sys.dllhandle query";
        return false;
    }
#else
        {
                int cout_pipe[2];

                pipe(cout_pipe);

                posix_spawn_file_actions_t action;
                posix_spawn_file_actions_init(&action);
                posix_spawn_file_actions_addclose(&action, cout_pipe[0]);
                posix_spawn_file_actions_adddup2(&action, cout_pipe[1], 1);
                posix_spawn_file_actions_addclose(&action, cout_pipe[1]);

                char **args = new char *[cmdline.size()+1];

                for (int32_t i=0; i<cmdline.size(); i++) {
                        args[i] = strdup(cmdline.at(i).c_str());
                }
                args[cmdline.size()] = 0;

                pid_t pid;
                if (posix_spawnp(&pid, args[0], &action, 0, &args[0], 0) != 0) {
                        err = "failed to start python";
                        return false;
                }

                close(cout_pipe[1]);

                // Now, read data from the pipe
                std::string out;
                char buf[1024];
                int sz;

                while ((sz=read(cout_pipe[0], buf, sizeof(buf))) > 0) {
                        buf[sz] = 0;
                        out += buf;
                }

                /*
                fprintf(stdout, "Output:\n");
                fputs(out.c_str(), stdout);
                 */
                size_t idx = 0;
                while (idx != std::string::npos) {
                    size_t start = idx;
                    idx = out.find('\n', start);

                    std::string line;
                    if (idx == std::string::npos) {
                        line = out.substr(start);
                    } else {
                        line = out.substr(start, (idx-start));
                        idx++;
                    }

                    size_t find_i;

                    if ((find_i = line.find("LDLIBRARY = ")) != std::string::npos) {
                        ldlibrary = line.substr(find_i+strlen("LDLIBRARY = ")+1);
                        ldlibrary = ldlibrary.substr(0, ldlibrary.size()-1);
                    } else if ((find_i = line.find("LIBDIR = ")) != std::string::npos) {
                        libdir = line.substr(find_i+strlen("LIBDIR = ")+1);
                        libdir = libdir.substr(0, libdir.size()-1);
                    }

//                    DEBUG("line=%s", line.c_str());
                }

                DEBUG("ldlibrary=%s", ldlibrary.c_str());
                DEBUG("libdir=%s", libdir.c_str());

                int32_t exit_code;
                waitpid(pid, &exit_code, 0);

                DEBUG("Exit code: %d", exit_code);
        }
#endif

    if (ldlibrary != "" && libdir != "") {
        python_dll = libdir + "/" + ldlibrary;
        return true;
    } else {
        return false;
    }
}

std::unique_ptr<IFactory> Factory::m_inst;
dmgr::IDebug *Factory::m_dbg = 0;

}

pyapi::IFactory *pyapi_compat_if_getFactory() {
    return pyapi::Factory::inst();
}
