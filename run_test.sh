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
./pod-agent_test_ProofStatusFile || clean_up
popd

