#!/bin/bash

mkdir ocl_aarch64-build
cd ocl_aarch64-build

cmd="cmake .. \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=install \
      -DCMAKE_TOOLCHAIN_FILE='/opt/toolchains/android-ndk-r24/build/cmake/android.toolchain.cmake' \
      -DANDROID_ABI=arm64-v8a \
      -DPPLCOMMON_USE_OPENCL=ON \
      -DPPLCOMMON_OPENCL_INCLUDE_DIRS='/opt/toolchains/android-toolchain-aarch64/include/OpenCL' \
      -DPPLCOMMON_OPENCL_LIBRARIES='/opt/toolchains/android-toolchain-aarch64/lib64/OpenCL/qualcomm/libOpenCL.so' \
      -DCL_TARGET_OPENCL_VERSION=200"
echo "cmd -> $cmd"
eval $cmd

cmake --build . -j 8 --config Release --target install

      # -DPPLCOMMON_OPENCL_LIBRARIES='/opt/toolchains/android-toolchain-aarch64/lib64/OpenCL/mali/t860/libGLES_mali.so' \
      # -DPPLCOMMON_OPENCL_LIBRARIES='/opt/toolchains/android-toolchain-aarch64/lib64/OpenCL/mali/libOpenCL.so' \