#! /bin/bash

# get a pid of our xrd. We get any xrd running by $UID
XRD_PID=`ps -w -u$UID -o pid,args | awk '{print $1" "$2}' | grep -v grep | grep xrootd| awk '{print $1}'`

if [ -n "$XRD_PID" ]
then
    echo "XRD is running under PID: "$XRD_PID
else
    echo "XRD is NOT running"
    exit 1
fi

# getting an array of XRD LISTEN ports
# oreder: the lowerst port goes first
XRD_PORTS=(`lsof -p $XRD_PID -P  | grep LISTEN | awk '{print $9}' | awk -F":" '{print $2}' |sort -b -n -u`)

echo "XRD port:"${XRD_PORTS[0]}
echo "XPROOF port:"${XRD_PORTS[1]}

exit 0