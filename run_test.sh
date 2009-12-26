#! /usr/bin/env bash

echo "*** pod-agent's UNIT-TESTs"
echo "*** Processing tests of ProtocoleCommands"
$1/pod-agent_test_ProtocolCommands || exit 1
