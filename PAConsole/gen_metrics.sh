#!/bin/bash

echo ">>> producing log from SVN... <<<"
svn log https://subversion.gsi.de/grid/D-Grid/PAConsole --xml -v > svn.log

echo ">>> setting up DISPLAY... <<<"
export DISPLAY=:0.0
xhost +localhost

echo ">>> pre-processing svn.log file... <<<"
# We need to prepare output file for StatSVN in order to get tags.
# This we need to do, since our project doesn't have dedicated repository in SVN and just is a part of "grid" repository
sed -i "s/\/D-Grid\/PAConsole\/tags\//\/tags\//g" svn.log

echo ">>> executing StatSVN... <<<"
java -jar trunk/PAConsole/sbin/statsvn.jar -css ./trunk/PAConsole/sbin/statsvn.css -output-dir ./PAConsole-metrics -tags '.*' svn.log .


echo ">>> generating SLOC count report... <<<"
sloccount --crossdups  --wide ./trunk/PAConsole/ > result.txt
./trunk/PAConsole/sbin/sloc2html.py result.txt > ./PAConsole-metrics/result.html

echo ">>> publishing metrics on the web... <<<"
chmod og+rx -R PAConsole-metrics
scp -p -r PAConsole-metrics manafov@lxial24:/misc/manafov/web-docs/D-Grid/metrics
