#! /usr/bin/env bash

# This script helps to build a GSI specific version of PoD and
# bins packages for PoD workers
# I will remove it as soon as I get a hardware for buildbot slaves. So I can
# have different virtual machines with different architecture to test on and produce binary packages.

export LD_LIBRARY_PATH=/misc/manafov/Qt/4.4.2_etch32/lib:$LD_LIBRARY_PATH
export PATH=/misc/manafov/cmake/cmake_32bit/cmake-2.6.4/bin:$PATH

POD_SRC="/misc/manafov/PoD/BinBuilds"

# hard-codded because of key changing problem (local GSI issue)
HOST64=lxi020.gsi.de
HOST32=lxi009.gsi.de

if [ -z "$1" ]; then
    # executing locally
    echo ">>> sending script to a remote host..."
    SCRIPT="/misc/manafov/tmp/prep_bins.sh"
    
    cat ./prep_bins.sh | ssh manafov@$HOST32 "cat > $SCRIPT; chmod 755 $SCRIPT; $SCRIPT get_repo"
    ssh manafov@$HOST32 "$SCRIPT wrk_bin"
    ssh manafov@$HOST64 "$SCRIPT wrk_bin"
    # gsi build needs to be the last, since it modifies bootstrap, so other builds would need to rewrite it
    ssh manafov@$HOST32 "$SCRIPT gsi_bin"
else
    echo ">>> executing on the host: "$(hostname -f)
    case "$1" in
	"get_repo")
	       export PATH=/misc/manafov/Soft/git/bin:$PATH
	       echo ">>> Preparing PoD repository..."
	       pushd `pwd`
	       cd $POD_SRC
	       rm -rf PoD
	       git clone ssh://anar@depc218.gsi.de//home/anar/GitRepository/PROOFonDemand/PoD
	       cd PoD
	       git submodule update --init --recursive || exit 1
	       popd
	       ;;
	"gsi_bin")
	       echo ">>> Building GSI pkg..."
	       $POD_SRC/PoD/bin/prep_gsi_bin.sh $POD_SRC/PoD || exit 1
	       ;;
	"wrk_bin") 
	    echo ">>> Building wrk. pkg..."
	    $POD_SRC/PoD/bin/prep_wrk_bins.sh $POD_SRC/PoD || exit 1
	    ;;
    esac
fi

exit 0