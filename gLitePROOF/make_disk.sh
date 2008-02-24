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

tar  -cvf $PKG_NAME.$VERSION.tar $PKG_NAME || exit 1
gzip -9 $PKG_NAME.$VERSION.tar || exit 1


rm -rf $PKG_NAME || exit 1

exit 0
