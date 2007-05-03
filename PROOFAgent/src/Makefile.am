##************************************************************************/
##**
## * @file PROOFAgent.h
##* @brief $$File comment$$
##* @author Anar Manafov A.Manafov@gsi.de
##*/ /*
##
##  version number:    $LastChangedRevision$
##  created by:        Anar Manafov
##                        		2007-03-01
##   last changed by:   $LastChangedBy$ $LastChangedDate$
##
##   Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
## :*************************************************************************/
bin_PROGRAMS = proofagent
proofagent_SOURCES = PacketForwarder.cpp AgentImpl.cpp Main.cpp PROOFAgent.cpp

# set the include path found by configure
INCLUDES = -I@top_srcdir@/Include $(all_includes)

# the library search path.
proofagent_LDFLAGS = $(all_libraries)
proofagent_includedir=$(includedir)/proofagent
proofagent_include_HEADERS = \
	../Include/ErrorCode.h \
	../Include/IXMLPersist.h \
	../Include/XMLHelper.h \
	../Include/LogImp.h \
	../Include/Log.h \
	../Include/MiscUtils.h \
	../Include/def.h \
	../Include/Res.h \
	../Include/INet.h \
	../Include/Process.h \
	../Include/TimeoutGuard.h \
	../Include/BOOSTHelper.h \
	../Include/SysHelper.h
noinst_HEADERS = PacketForwarder.h Agent.h AgentImpl.h PROOFAgent.h PARes.h
AM_CXXFLAGS = -Wall

SUBDIRS = unit_test
