#! /usr/bin/env bash

# This script helps to build a binaries for PoD workers.
# So far it is only pod-agent, who is needed
# Provide a path to a source tree of PoD as a parameter to the script.
 
 
POD_SRC=$(readlink -f $1)
LIBS_PATH_X86=/misc/manafov/PoD/forGSI/pod-agent-bin-libs/x86
LIBS_PATH_AMD64=/misc/manafov/PoD/forGSI/pod-agent-bin-libs/amd64

# bin name:
# <pakage>-<version>-<OS>-<ARCH>-<GCC_VER>.tar.gz

OS=$(uname -s 2>&1)
GCC_VER=$(gcc -dumpversion 2>/dev/null) #| sed 's/\./x/g')
if [ -z "GCC_VER" ]; then
    echo "bad gcc version"
    exit 1
fi
GCC_VER="gcc$GCC_VER"

host_arch=$( uname -m  2>&1)
case "$host_arch" in
    i[3-9]86*|x86|x86pc|k5|k6|k6_2|k6_3|k6-2|k6-3|pentium*|athlon*|i586_i686|i586-i686)
        host_arch=x86
	export PATH=/misc/manafov/cmake/cmake_32bit/cmake-2.6.4/bin:$PATH
	LIBS_PATH=$LIBS_PATH_X86
        ;;
    x86_64)
        host_arch=amd64
	export PATH=/misc/manafov/cmake/cmake_64bit/cmake/bin:$PATH
	LIBS_PATH=$LIBS_PATH_AMD64
        ;;
    *)
        logMsg "Error: unsupported architecture: $host_arch"
        exit 1
        ;;
esac

# build PoD
POD_BUILD_DIR="$POD_SRC/build_$host_arch"
mkdir -p $POD_BUILD_DIR || exit 1
mkdir -p $POD_SRC/inst_tmp || exit 1
POD_AGENT_BIN_DIR="$POD_SRC/build_agent_$host_arch/pod-agent"
mkdir -p $POD_AGENT_BIN_DIR || exit 1

POD_INST=$POD_SRC/inst_tmp

pushd `pwd`
cd $POD_BUILD_DIR
cmake -DCMAKE_INSTALL_PREFIX:PATH=$POD_INST -DCMAKE_BUILD_TYPE=Release .. || exit 1
make -j4 pod-agent || exit 1
make install || exit 1
popd

PKG_VERSION=$(cat $POD_SRC/etc/version)

# Copy pod-agent
cp -v "$POD_INST/bin/pod-agent" $POD_AGENT_BIN_DIR/ || exit 1

# Copy external libs
cp -v $LIBS_PATH/* $POD_AGENT_BIN_DIR/ || exit 1

# Tar the package
PKG_NAME="pod-agent-$PKG_VERSION-$OS-$host_arch-$GCC_VER.tar.gz"
pushd `pwd`
cd $POD_AGENT_BIN_DIR/..
$(tar -czvf $PKG_NAME pod-agent/ &>/dev/null)

# release the tarball
chmod go+xr $PKG_NAME || exit 1
scp -p $PKG_NAME manafov@lxg0527:/misc/manafov/web-docs/D-Grid/Release/Nightly || exit 1
popd

exit 0

