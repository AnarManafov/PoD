#!/bin/bash


# changing the name of the proxy file
mv x509up_u500 /tmp/x509up_u$UID
chmod 600 /tmp/x509up_u$UID

# ROOT
export ROOTSYS=/usr/ROOT/5.11.06
export PATH=$ROOTSYS/bin:$PATH
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH

# PROOFD
proofd -A -d 3 -p 5151

# Sleeping for a while
sleep 600

# killing PROOFD
pkill proofd

# removing proxy file
rm /tmp/x509up_u$UID
