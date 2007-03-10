AUTOMAKE_OPTIONS = gnu


# set the include path found by configure
#AM_CPPFLAGS =\
#                $(GAW_INC)\
#                $(GLITE_INC)\
#                $(LCG_INC)

# defining bin module
bin_PROGRAMS=unit_tests
unit_tests_SOURCES = main.cpp
#gaw_test_jobsubmit_LDFLAGS = $(GAW_LIB_DIR)
#gaw_test_jobsubmit_LDADD = $(GAW_LIBS)

INCLUDES = -I@top_srcdir@/Include $(all_includes)

unit_tests_LDADD = -lboost_unit_test_framework
