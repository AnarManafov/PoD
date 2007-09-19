#! /bin/bash

rm -fr autom4te.cache

# Checking that ChangeLog is exist
if [ ! -e "ChangeLog" ]; then
 ./Make_ChangeLog.sh || touch "ChangeLog"
fi

# Equivalent to
#       aclocal
#       autoheader
#       automake --add-missing --copy
#       autoconf
autoreconf --verbose --force --install -I config -I m4 || exit 1

echo "Now run ./configure and then make."
exit 0

