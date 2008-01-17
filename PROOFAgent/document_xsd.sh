#!/bin/bash

FOLDER="documentation/PROOFAgent_config/PROOFAgent_cfg_XML_Schema"
rm -rf $FOLDER
mkdir $FOLDER
xsddoc -t "XML Schema for the PROOFAgent configuration file" -o $FOLDER -verbose documentation/PROOFAgent_config/proofagent.cfg.xsd

