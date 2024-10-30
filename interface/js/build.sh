#!/bin/bash

mkdir /bld
cd /bld
cmake /src -DCMAKE_TOOLCHAIN_FILE=/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DJEXPR_NO_PYTHON=ON -DJEXPR_JAVASCRIPT_MODULE=ON -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_BUILD_TYPE=Release -DJEXPR_EMBIND=ON
# cmake --build . --target main && cp main.* /src
# node main.js
cmake --build . --target jexpr && cp jexpr.* /src/interface/js
exit 0