bin_PROGRAMS = proofagent
proofagent_SOURCES = PacketForwarder.cpp AgentImpl.cpp Main.cpp PROOFAgent.cpp

# set the include path found by configure
INCLUDES = -I@top_srcdir@/Include $(all_includes)

# the library search path.
proofagent_LDFLAGS = $(all_libraries) 
noinst_HEADERS = PacketForwarder.h Agent.h AgentImpl.h PROOFAgent.h
AM_CXXFLAGS = -Wall
