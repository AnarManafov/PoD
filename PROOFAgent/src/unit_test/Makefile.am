#************************************************************************/
#**
# * @file Makefile.am
# * @brief Automake makefile for Unit Tests (UT) of MiscCommon
# * @author Anar Manafov A.Manafov@gsi.de
# */ /*
# 
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2007-03-10
#        last changed by:   $LastChangedBy$ $LastChangedDate$
# 
#        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
#*************************************************************************/ 
AUTOMAKE_OPTIONS = gnu

INCLUDES = -I@top_srcdir@/Include $(all_includes)
# defining bin module
bin_PROGRAMS=UT_MiscUtils UT_Process

# Test_MiscUtils.cpp
UT_MiscUtils_SOURCES = Test_MiscUtils.cpp
UT_MiscUtils_LDADD = -lboost_unit_test_framework

# Test_Process.cpp
UT_Process_SOURCES = Test_Process.cpp
UT_Process_LDADD = -L@BOOST_INSTALL_PATH@/lib -lboost_unit_test_framework