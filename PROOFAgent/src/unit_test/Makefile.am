#************************************************************************/
#**
# * @file Makefile.am
# * @brief Automake makefile for Unit tests of MiscCommon
# * @author Anar Manafov A.Manafov@gsi.de
# */ /*
# 
#        version number:   $LastChangedRevision$
#        created by:          Anar Manafov
#                                  2007-03-10
#        last changed by:   $LastChangedBy$ $LastChangedDate$
# 
#        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
#*************************************************************************/ 
AUTOMAKE_OPTIONS = gnu

# defining bin module
bin_PROGRAMS=UnitTest_MiscUtils
UnitTest_MiscUtils_SOURCES = Test_MiscUtils.cpp

INCLUDES = -I@top_srcdir@/Include $(all_includes)

UnitTest_MiscUtils_LDADD = -lboost_unit_test_framework
