#! /bin/bash

mkdir -p gLitePROOFpackage || exit 1

cp  --target-directory=gLitePROOFpackage -rv documentation gLitePROOF.jdl gLitePROOF.sh myselector.C myselector.h PAConsole proofagent proofagent-1.0.2.1270.tar.gz proofagent.cfg.xml proof_demo1.C READ.ME Server_gLitePROOF.sh xpd.cf || exit 1

tar  -czvf gLitePROOFpackage.2.0.1.tgz gLitePROOFpackage || exit 1

rm -rf gLitePROOFpackage || exit 1

exit 0
