#! /usr/bin/env bash

# This script helps to build a GSI specific version of PoD.
#
# Provide a path to a source tree of PoD as a parameter to the script.
 

POD_SRC=$(readlink -f $1)
LIBS_PATH=/misc/manafov/PoD/forGSI/libs32b_fo_64bit/

# build PoD
mkdir $POD_SRC/build
mkdir $POD_SRC/inst_tmp
# set a local install prefix
export POD_LOCATION=$POD_SRC/inst_tmp

pushd `pwd`
cd $POD_SRC/build
cmake -C ../BuildSetup.cmake ..
make install
popd

POD_INST=$POD_LOCATION

# Copy compiled parts of PoD
cp -v $POD_INST/bin/* $POD_SRC/bin/ || exit 1
                      rm $POD_SRC/bin/pod-info || exit 1

# Copy plug-ins
mkdir -p $POD_SRC/plugins
cp -v $POD_INST/plugins/* $POD_SRC/plugins/ || exit 1

# Copy external libs
mkdir -p $POD_SRC/lib || exit 1
cp -v $LIBS_PATH/* $POD_SRC/lib/ || exit 1

echo "set( GSI_BUILD ON CACHE BOOL \"Build a GSI specific version\" FORCE )" >> $POD_SRC/BuildSetup.cmake || exit 1

# build a GSI specific distr.
rm -rf $POD_SRC/build/*
pushd `pwd`
cd $POD_SRC/build
cmake -C ../BuildSetup.cmake ..
make package_source
popd

# release the tarball
$(chmod go+xr $POD_SRC/build/*.tar.gz) || exit 1
$(scp -p $POD_SRC/build/*.tar.gz manafov@lxg0527:/misc/manafov/web-docs/D-Grid/Release/Nightly) || exit 1

exit 0

