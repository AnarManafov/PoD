#! /usr/bin/env bash

# This script helps to build a GSI specific version of PoD and
# bins packages for PoD workers

export LD_LIBRARY_PATH=/misc/manafov/Qt/4.4.2_etch32/lib:$LD_LIBRARY_PATH
export PATH=/misc/manafov/cmake/cmake_32bit/cmake-2.6.4/bin:$PATH
export PATH=/misc/manafov/Soft/git/bin:$PATH

POD_SRC="/misc/manafov/PoD/BinBuilds"


if [ -z "$1" ]; then
    # executing locally
    cat ./prep_bins.sh | ssh manafov@lxetch32 "cat > /tmp/prep_bins.sh; chmod 755 /tmp/prep_bins.sh; /tmp/prep_bins.sh remote"
else
    # executing remotly
    # prep repo
    pushd `pwd`
    cd $POD_SRC
    rm -rf PoD
    git clone ssh://anar@depc218.gsi.de//home/anar/GitRepository/PROOFonDemand/PoD
    cd PoD
    git submodule update --init --recursive || exit 1
    popd
    
    # GSI bin
    ssh lxetch32 "$POD_SRC/PoD/bin/prep_gsi_bin.sh $POD_SRC/PoD" || exit 1
    
    # Workers bins
    ssh lxetch64 "$POD_SRC/PoD/bin/prep_wrk_bins.sh $POD_SRC/PoD" || exit 1
    ssh lxetch32 "$POD_SRC/PoD/bin/prep_wrk_bins.sh $POD_SRC/PoD" || exit 1
fi

exit 0