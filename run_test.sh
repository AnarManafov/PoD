#! /usr/bin/env bash


clean_up()
{
 popd
 exit 1
}

trap clean_up SIGHUP SIGINT SIGTERM

pushd `pwd`
cd $1

echo "----------------------"
echo "pod-agent UNIT-TESTs"
echo "----------------------"
echo ">>> Processing tests of ProtocoleCommands"
./pod-agent_test_ProtocolCommands || clean_up
echo
echo ">>> Processing tests of Protocole"
./pod-agent_test_Protocol || clean_up
echo
echo ">>> Processing tests of ProofStatusFile"
# prepare dummy files
# TODO: implement a function for the following
ADMIN_PATH_SRV="PoDServer/.xproofd.22222/activesessions"
ADMIN_PATH=".xproofd.22222/activesessions"
mkdir -p $ADMIN_PATH_SRV
mkdir -p $ADMIN_PATH
echo "2" > $ADMIN_PATH_SRV/manafov.default.1234.status
echo "0" > $ADMIN_PATH/manafov.default.1234.status
echo "1" > $ADMIN_PATH/manafov.default.5678.status
echo "3" > $ADMIN_PATH/manafov.default.9.status
./pod-agent_test_ProofStatusFile || clean_up

echo "----------------------"
echo "pod-ssh UNIT-TESTs"
echo "----------------------"
echo ">>> Processing tests of the ssh-plugin config file engine"
./pod-ssh_test_config

popd

