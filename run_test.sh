#! /usr/bin/env bash


clean_up()
{
 popd
 exit 1
}

trap clean_up SIGHUP SIGINT SIGTERM
echo "----------------------"
echo "pod-agent's UNIT-TESTs"
echo "----------------------"
pushd `pwd`
cd $1
echo ">>> Processing tests of ProtocoleCommands"
./pod-agent_test_ProtocolCommands || clean_up
echo
echo ">>> Processing tests of Protocole"
./pod-agent_test_Protocol || clean_up
echo
echo ">>> Processing tests of ProofStatusFile"
# prepare dummy files
ADMIN_PATH=".xproofd.22222/activesessions"
mkdir -p $ADMIN_PATH
echo "0" > $ADMIN_PATH/manafov.default.1234.status
echo "1" > $ADMIN_PATH/manafov.default.5678.status
echo "3" > $ADMIN_PATH/manafov.default.9.status
./pod-agent_test_ProofStatusFile || clean_up
popd

