#!/bin/bash

OUT_DIR=./html/

xmlto -o $OUT_DIR xhtml -m config.xsl gLitePROOF.docbook
cp docbook.css $OUT_DIR


