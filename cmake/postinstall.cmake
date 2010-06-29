
# generate a default PoD configuration file

# setting the lib path is important for GSI specific builds
if(GSI_BUILD)
	set( ENV{LD_LIBRARY_PATH} "${CMAKE_INSTALL_PREFIX}/lib:$ENV{LD_LIBRARY_PATH}" )
endif(GSI_BUILD)
# generate a default PoD configuration file
execute_process(COMMAND ${CMAKE_INSTALL_PREFIX}/bin/pod-user-defaults -c ${CMAKE_INSTALL_PREFIX}/etc/PoD.cfg -d )

