
print("__init__")


def init():
    print("init()")
    import pyapi_compat_if.core as core
    f = core.Factory.inst()

def reset():
    print("reset()")
    import pyapi_compat_if.core as core
    core.Factory.reset()




