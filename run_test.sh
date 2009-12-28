#! /usr/bin/env bash
echo "----------------------"
echo "pod-agent's UNIT-TESTs"
echo "----------------------"
echo ">>> Processing tests of ProtocoleCommands"
$1/pod-agent_test_ProtocolCommands || exit 1
echo
echo ">>> Processing tests of Protocole"
$1/pod-agent_test_Protocol || exit 1
