
# generate a default PoD configuration file

# setting the lib path variable is important for GSI specific builds
# we set it for all builds so-far, it doesn't break anything
set( ENV{LD_LIBRARY_PATH} "${CMAKE_INSTALL_PREFIX}/lib:$ENV{LD_LIBRARY_PATH}" )
# generate a default PoD configuration file
execute_process(COMMAND ${CMAKE_INSTALL_PREFIX}/bin/pod-user-defaults -c ${CMAKE_INSTALL_PREFIX}/etc/PoD.cfg -d )

