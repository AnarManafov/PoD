#! /bin/bash

VERSION=2.0.4

PKG_NAME=gLitePROOFpackage


# building documentation
pushd documentation/src/
gmake || exit 1
popd

# making pkg.
mkdir -p $PKG_NAME || exit 1

cp --target-directory=$PKG_NAME -rv \
    bin \
    documentation \
    etc \
    template \
    "test" \
    install \
    || exit 1

rm -rf $PKG_NAME/bin/.svn
rm -rf $PKG_NAME/documentation/.svn
rm -rf $PKG_NAME/etc/.svn
rm -rf $PKG_NAME/template/.svn
rm -rf $PKG_NAME/test/.svn

# a revision number
REV=`svn info https://subversion.gsi.de/dgrid/gLitePROOF/trunk/gLitePROOF  | grep "Revision: " | head -1 | awk -F": " '{printf("%s", $ 2)}'`
echo "REV=$REV"
THE_NAME=$PKG_NAME.$VERSION.$REV
tar  -cvf $THE_NAME.tar $PKG_NAME || exit 1
gzip -9 $THE_NAME.tar || exit 1


rm -rf $PKG_NAME || exit 1

exit 0
