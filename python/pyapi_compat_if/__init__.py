

print("__init__")

try:
    import pyapi_compat_if.core as core
    f = core.Factory.inst()
except Exception as e:
    print("Exception: %s" % str(e))


