rm -Rf build

# sphinx-apidoc -o source/ ../anntoolkit
sphinx-build -M markdown source build
sphinx-build -M html source build
