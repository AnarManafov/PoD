#! /bin/bash
##/************************************************************************/
##/*! \file make_dist.sh
##  *//*
##
##         version number:     $LastChangedRevision$
##         created by:         Anar Manafov
##                             2008-02-06
##         last changed by:    $LastChangedBy$ $LastChangedDate$
##
##         Copyright (c) 2008-2009 GSI GridTeam. All rights reserved.
##*************************************************************************/
VERSION=2.0.10

PKG_NAME=PoD_Package


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
    ReleaseNotes \
    || exit 1

if [[ -d $PKG_NAME  ]] ; then
	rm -rf $PKG_NAME/bin/.svn
	rm -rf $PKG_NAME/documentation/.svn
	rm -rf $PKG_NAME/documentation/src/img/.svn
	rm -rf $PKG_NAME/documentation/src/.svn
	rm -rf $PKG_NAME/etc/.svn
	rm -rf $PKG_NAME/template/.svn
	rm -rf $PKG_NAME/test/.svn
fi

THE_NAME=$PKG_NAME.$VERSION
rm -f $THE_NAME.tar.* 
tar  -cvf $THE_NAME.tar $PKG_NAME || exit 1
gzip -9 $THE_NAME.tar || exit 1


rm -rf $PKG_NAME || exit 1

exit 0
