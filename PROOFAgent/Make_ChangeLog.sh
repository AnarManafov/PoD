#! /bin/bash

# this script is used for generating a classic GNU-style
# ChangeLog from a subversion repository log.
#

svn2cl -i --group-by-day || touch ChangeLog
