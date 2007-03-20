#! /bin/bash

echo ">>> Producing a ChangeLog file... <<<"
./Make_ChangeLog.sh
autoreconf -v --force --install -I config -I m4

