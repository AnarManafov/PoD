TEMPLATE	= app
LANGUAGE = C++

CONFIG = qt

VERSION = 0.0.2

INCLUDEPATH = ./Include \
						${GAW_LOCATION}/include/glite-api-wrapper \
						${GLITE_LOCATION}/include \
						${GLITE_LOCATION}/externals/include \
						${LCG_LOCATION}/include/lfc 

HEADERS	+= 	MainDlg.h \
						JobSubmitter.h

SOURCES	+= 	MainDlg.cpp \
						ServerInfo.cpp \
					 	main.cpp
					 	
FORMS		= 		maindlg.ui

QT           += xml

unix:LIBS += -L${GAW_LOCATION}/lib -lglite-api-wrapper

					
