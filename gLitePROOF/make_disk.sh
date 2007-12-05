#! /bin/bash

VERSION=2.0.2

mkdir -p gLitePROOFpackage || exit 1

cp  --target-directory=gLitePROOFpackage -rv \
	PAConsole \
	READ.ME  \
	Server_gLitePROOF.sh \
	documentation \
	dstarmb.root \
	gLitePROOF.jdl \
	gLitePROOF.sh \
	gLitePROOF_FZK.jdl \
	glite-api-wrapper.cfg.xml \
	myselector.C \
	myselector.h \
	proof_demo1.C \
	proofagent \
	proofagent.cfg.xml \
	xpd.cf \
	|| exit 1

tar  -czvf gLitePROOFpackage.$VERSION.tgz gLitePROOFpackage || exit 1

rm -rf gLitePROOFpackage || exit 1

exit 0
