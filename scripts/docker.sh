# Script to build wheels for manylinux. This script runs inside docker.
# See build_maylinux_wheels.sh

git clone --depth 1 https://github.com/podgorskiy/anntoolkit.git
cd anntoolkit
git submodule update --init  --depth 1
cd libs/glfw
git pull origin master
cd ../..

yum install -y libX11-devel libXcursor-devel libXrandr-devel libXinerama-devel mesa-libGL-devel libXi-devel

for PYBIN in /opt/python/*/bin; do
    #"${PYBIN}/pip" install -r /io/dev-requirements.txt
    #"${PYBIN}/pip" wheel /io/ -w wheelhouse/
    "${PYBIN}/python" setup.py bdist_wheel -d /io/wheelhouse/ &
done

wait

echo "All built"

echo "Audit..."

# Bundle external shared libraries into the wheels
for whl in /io/wheelhouse/*.whl; do
    auditwheel repair "$whl" --plat $PLAT -w /io/wheelhouse/ &
done

wait

echo "All done"
