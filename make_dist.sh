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
VERSION=2.0.7

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

# a revision number
REV=`svn info https://subversion.gsi.de/dgrid/PoD/trunk/PoD  | grep "Revision: " | head -1 | awk -F": " '{printf("%s", $ 2)}'`
echo "REV=$REV"
THE_NAME=$PKG_NAME.$VERSION.$REV
rm -f $THE_NAME.tar.* 
tar  -cvf $THE_NAME.tar $PKG_NAME || exit 1
gzip -9 $THE_NAME.tar || exit 1


rm -rf $PKG_NAME || exit 1

exit 0