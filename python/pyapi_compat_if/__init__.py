
import os

def get_deps():
    return []

def get_libs():
    return ["pyapi-compat-if"]

def get_libdirs():
    pkg_dir = os.path.dirname(os.path.abspath(__file__))
    return [pkg_dir]

def get_incdirs():
    pkg_dir = os.path.dirname(os.path.abspath(__file__))
    if os.path.isdir(os.path.join(pkg_dir, "include")):
        return [os.path.join(pkg_dir, "include")]
    else:
        root_dir = os.path.abspath(os.path.join(pkg_dir, "../.."))
        ret = [
            os.path.join(root_dir, "src", "include"),
            os.path.join(root_dir, "build/src/pyeval_base/include")
        ]
        print("ret: %s" % str(ret))
        return ret


def init():
    print("init()")
    import pyapi_compat_if.core as core
    f = core.Factory.inst()

def reset():
    print("reset()")
    import pyapi_compat_if.core as core
    core.Factory.reset()




