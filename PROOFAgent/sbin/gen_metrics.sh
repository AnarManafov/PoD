#!/bin/bash

echo ">>> producing log from SVN... <<<"

echo '<?xml version="1.0"?>' > svn.log
echo "<log>" >> svn.log
svn log https://subversion.gsi.de/grid/D-Grid/Include --incremental --xml -v >> svn.log
svn log https://subversion.gsi.de/grid/D-Grid/PROOFAgent --incremental --xml -v >> svn.log
echo "</log>" >> svn.log

echo ">>> setting up DISPLAY... <<<"
export DISPLAY=:0.0
xhost +localhost

echo ">>> pre-processing svn.log file... <<<"
# We need to prepare output file for StatSVN in order to get tags.
# This we need to do, since our project doesn't have dedicated repository in SVN and just is a part of "grid" repository
sed -i "s/\/D-Grid\/PROOFAgent\/tags\//\/tags\//g" svn.log
# adding external "Include" dir to the metrics
sed -i "s/\/D-Grid\/Include/\/D-Grid\/PROOFAgent\/trunk\/PROOFAgent\/Include/g" svn.log

echo ">>> executing StatSVN... <<<"
java -jar trunk/PROOFAgent/sbin/statsvn.jar -include Include/** -css ./trunk/PROOFAgent/sbin/statsvn.css -output-dir ./PROOFAgent-metrics -tags '.*' svn.log .


echo ">>> generating SLOC count report... <<<"
sloccount --crossdups  --wide ./trunk/PROOFAgent/ > result.txt
./trunk/PROOFAgent/sbin/sloc2html.py result.txt > ./PROOFAgent-metrics/result.html

echo ">>> publishing metrics on the web... <<<"
chmod og+rx -R PROOFAgent-metrics
scp -p -r PROOFAgent-metrics manafov@lxial24:/misc/manafov/web-docs/D-Grid/metrics

