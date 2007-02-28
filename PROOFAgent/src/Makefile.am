bin_PROGRAMS = proofagent
proofagent_SOURCES = proofagent.cpp PacketForwarder.cpp main.cpp

# set the include path found by configure
INCLUDES = -I@top_srcdir@/Include $(all_includes)

# the library search path.
proofagent_LDFLAGS = $(all_libraries) 
noinst_HEADERS = PacketForwarder.h agent.h AgentImpl.h proofagent.h
