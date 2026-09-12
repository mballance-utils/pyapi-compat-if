#!/bin/sh -ex

# -e matters. Without it this script ran every remaining line after a failure
# and exited on whatever the last command happened to return: the cp38 legs
# reported "rm: cannot remove dist/*.whl", four commands downstream of the real
# error, which was that the interpreter did not exist.

git config --global --add safe.directory /io

# BUILD_NUM is the PEP 440 suffix the workflow computed, e.g.
# "dev33020374597+gh.g7775f37". It is EMPTY on a tag build, and an empty file
# is not the same as an absent one -- setup.py keys off the import failing.
if [ -n "${BUILD_NUM}" ]; then
    echo "BUILD_NUM=\"${BUILD_NUM}\"" > python/pyapi_compat_if/__build_num__.py
else
    rm -f python/pyapi_compat_if/__build_num__.py
fi

yum install -y ninja-build

${IVPM_PYTHON} -m pip install ivpm cython setuptools --pre
# -d default, explicitly: without it ivpm resolves default-dev on a source
# checkout and drags the C++ test dependencies into the release build path.
${IVPM_PYTHON} -m ivpm update -a -d default --py-prerls-packages --py-pip

PYTHON=./packages/python/bin/python
${PYTHON} -m pip install twine auditwheel ninja wheel cython
${PYTHON} setup.py bdist_wheel

for whl in dist/*.whl; do
    ${PYTHON} -m auditwheel repair --only-plat $whl
    rm $whl
done

# Import the extension out of the REPAIRED wheel, not the build tree: this is
# what catches a wheel that compiled but cannot resolve libpyapi-compat-if or
# libdebug-mgr at load time.
${PYTHON} -m pip install wheelhouse/*.whl
${PYTHON} -c "import pyapi_compat_if, pyapi_compat_if.core as core; print('import OK:', core.__file__)"
