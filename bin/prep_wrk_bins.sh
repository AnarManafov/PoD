#! /usr/bin/env bash

# This script helps to build a binaries for PoD workers.
# So far it is only pod-agent and pod-user-defaults, who is needed
# Provide a path to a source tree of PoD as a parameter to the script.

#=============================================================================
# ***** a portable version of "readlink -f" *****
# needed because MacOSX doesn't support readlink -f
#=============================================================================
my_readlink()
{
   TARGET_FILE=$1

   cd `dirname $TARGET_FILE`
   TARGET_FILE=`basename $TARGET_FILE`

   # Iterate down a (possible) chain of symlinks
   while [ -L "$TARGET_FILE" ]
   do
      TARGET_FILE=`readlink $TARGET_FILE`
      cd `dirname $TARGET_FILE`
      TARGET_FILE=`basename $TARGET_FILE`
   done

   # Compute the canonicalized name by finding the physical path 
   # for the directory we're in and appending the target file.
   PHYS_DIR=`pwd -P`
   RESULT=$PHYS_DIR/$TARGET_FILE
   echo $RESULT
}

#=============================================================================
POD_SRC=$(my_readlink $1)
LIBS_PATH_X86=/misc/manafov/PoD/forGSI/pod-agent-bin-libs/x86
LIBS_PATH_AMD64=/misc/manafov/PoD/forGSI/pod-agent-bin-libs/amd64
LIBS_PATH_DARWIN_AMD64=/Users/anar/PoD_For_WN_PKG

SSH_ARGS="-o BatchMode=yes -o StrictHostKeyChecking=no -o PasswordAuthentication=no -q"

# bin name:
# <pakage>-<version>-<OS>-<ARCH>.tar.gz
BASE_NAME="pod-wrk-bin"

OS=$(uname -s 2>&1)
host_arch=$( uname -m  2>&1)


case "$OS" in
   "Linux")
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
   ;;

   "Darwin")
      LIBS_PATH=$LIBS_PATH_DARWIN_AMD64
   ;;

   *)
      logMsg "Error: unsupported OS: $OS"
      exit 1
   ;;
esac

# build PoD
POD_BUILD_DIR="$POD_SRC/build_$host_arch"
mkdir -p $POD_BUILD_DIR || exit 1
mkdir -p $POD_SRC/inst_tmp || exit 1

POD_AGENT_BIN_DIR="$POD_SRC/build_agent_$host_arch/$BASE_NAME"
mkdir -p $POD_AGENT_BIN_DIR || exit 1

POD_INST=$POD_SRC/inst_tmp

pushd `pwd`
cd $POD_BUILD_DIR
cmake -DCMAKE_INSTALL_PREFIX:PATH=$POD_INST -DCMAKE_BUILD_TYPE=Release .. || exit 1
make -j4 pod-agent || exit 1
make -j4 pod-user-defaults || exit 1
make install || exit 1
popd

PKG_VERSION=$(cat $POD_SRC/etc/version)

# Copy the binaries
cp -v "$POD_INST/bin/pod-agent" $POD_AGENT_BIN_DIR/ || exit 1
cp -v "$POD_INST/bin/pod-user-defaults" $POD_AGENT_BIN_DIR/ || exit 1
# Copy needed PoD libs
case "$OS" in
   "Linux")
      cp -v "$POD_INST/lib/libpod_protocol.so" $POD_AGENT_BIN_DIR/ || exit 1
      cp -v "$POD_INST/lib/libproof_status_file.so" $POD_AGENT_BIN_DIR/ || exit 1
   ;;

   "Darwin")
      cp -v "$POD_INST/lib/libpod_protocol.dylib" $POD_AGENT_BIN_DIR/ || exit 1
      cp -v "$POD_INST/lib/libproof_status_file.dylib" $POD_AGENT_BIN_DIR/ || exit 1
   ;;
esac

# Copy external libs
cp -v $LIBS_PATH/* $POD_AGENT_BIN_DIR/ || exit 1

# Tar the package
PKG_NAME="$BASE_NAME-$PKG_VERSION-$OS-$host_arch.tar.gz"
pushd `pwd`
cd $POD_AGENT_BIN_DIR/..
tar -czvf $PKG_NAME $BASE_NAME &>/dev/null

# release the tarball
chmod go+xr $PKG_NAME || exit 1
REPO_LOCATION="/u/podwww/web-docs/releases/add/$PKG_VERSION"
ssh $SSH_ARGS podwww@lxi001 "mkdir -p $REPO_LOCATION && chmod go+xr $REPO_LOCATION"
scp $SSH_ARGS -p $PKG_NAME podwww@lxi001:$REPO_LOCATION || exit 1
popd

exit 0

