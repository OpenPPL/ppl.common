#!/bin/bash

mkdir ocl_x86-build
cd ocl_x86-build

cmd="cmake .. \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=install \
      -DPPLCOMMON_USE_OPENCL=ON \
      -DPPLCOMMON_OPENCL_INCLUDE_DIRS='/usr/include' \
      -DPPLCOMMON_OPENCL_LIBRARIES='/usr/local/cuda-10.0/lib64/libOpenCL.so' \
      -DCL_TARGET_OPENCL_VERSION=200"
echo "cmd -> $cmd"
eval $cmd

cmake --build . -j 8 --config Release --target install
