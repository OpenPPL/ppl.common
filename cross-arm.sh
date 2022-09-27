toolchain='/opt/toolchains/linux-toolchain-aarch64'

builddir='build-cross'
mkdir ${builddir}
cd ${builddir}
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install -DPPLNN_TOOLCHAIN_DIR="${toolchain}" -DCMAKE_TOOLCHAIN_FILE=/tmp/aarch64-linux-gnu.cmake -DPPLNN_USE_AARCH64=ON
cmake --build . -j `nproc` --config Release --target install
