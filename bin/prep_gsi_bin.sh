#! /usr/bin/env bash

POD_SRC=$1
POD_INST=$2

# Copy compiled parts of PoD
cp -v $POD_INST/bin/* $POD_SRC/bin/ || exit 1
                      rm $POD_SRC/bin/pod-info || exit 1

# Copy plug-ins
mkdir -p $POD_SRC/plugins
cp -v $POD_INST/plugins/* $POD_SRC/plugins/ || exit 1

# Copy external libs
mkdir -p $POD_SRC/lib || exit 1
cp -v /misc/manafov/PoD/forGSI/libs32b_fo_64bit/* $POD_SRC/lib/ || exit 1

# make the source distr.
cd $POD_SRC/build || exit 1
make package_source

echo "set( GSI_BUILD ON CACHE BOOL "Build a GSI specific version" FORCE )" >> $POD_SRC/BuildSetup.cmake || exit 1

exit 0

