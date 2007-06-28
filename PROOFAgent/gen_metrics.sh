#!/bin/bash

echo ">>> producing log from SVN... <<<"
svn log https://subversion.gsi.de/grid/D-Grid/PROOFAgent --xml -v > svn.log

echo ">>> setting up DISPLAY... <<<"
export DISPLAY=:0.0
xhost +localhost

echo ">>> pre-processing svn.log file... <<<"
# We need to prepare output file for StatSVN in order to get tags.
# This we need to do, since our project doesn't have dedicated repository in SVN and just is a part of "grid" repository
sed -i "s/\/D-Grid\/PROOFAgent\/tags\//\/tags\//g" svn.log

echo ">>> executing StatSVN... <<<"
java -jar ./PROOFAgent/trunk/PROOFAgent/sbin/statsvn.jar -css ./PROOFAgent/trunk/PROOFAgent/sbin/statsvn.css -output-dir ./PROOFAgent-metrics -tags '.*' svn.log ./PROOFAgent/


echo ">>> generating SLOC count report... <<<"
sloccount --crossdups  --wide ./PROOFAgent/trunk/PROOFAgent/ > result.txt
./PROOFAgent/trunk/PROOFAgent/sbin/sloc2html.py result.txt > ./PROOFAgent-metrics/result.html

echo ">>> publishing metrics on the web... <<<"
chmod og+rx -R PROOFAgent-metrics
scp -p -r PROOFAgent-metrics manafov@lxial24:/misc/manafov/web-docs/D-Grid/metrics

