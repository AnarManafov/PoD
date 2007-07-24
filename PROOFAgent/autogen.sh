#! /bin/bash

rm -fr autom4te.cache

echo ">>> Producing a ChangeLog file... <<<"
./Make_ChangeLog.sh

# Equivalent to
#       aclocal
#       autoheader
#       automake --add-missing --copy
#       autoconf
autoreconf --verbose --force --install -Wall -I config -I m4 || exit 1

echo "Now run ./configure and then make."
exit 0

