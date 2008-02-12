#! /bin/bash

VERSION=2.0.4

PKG_NAME=gLitePROOFpackage

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

tar  -czvf $PKG_NAME.$VERSION.tgz $PKG_NAME || exit 1

rm -rf $PKG_NAME || exit 1

exit 0
