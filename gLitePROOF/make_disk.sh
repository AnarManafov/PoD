#! /bin/bash

VERSION=2.0.3

mkdir -p gLitePROOFpackage || exit 1

cp  --target-directory=gLitePROOFpackage -rv \
	gLitePROOF_FZK.jdl \
	myselector.C \
	proofagent.cfg.xml \
	xpd.cf.template \
	gLitePROOF.jdl \
	myselector.h \
	proof_demo1.C \
	documentation \
	gLitePROOF.sh \
	PAConsole \
	READ.ME \
	dstarmb.root \
	proofagent \
	Server_gLitePROOF.sh \
	|| exit 1


tar  -czvf gLitePROOFpackage.$VERSION.tgz gLitePROOFpackage || exit 1

rm -rf gLitePROOFpackage || exit 1

exit 0
