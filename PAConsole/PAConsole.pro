TEMPLATE	= app
LANGUAGE = C++

INCLUDEPATH = ./Include

HEADERS	+= 	MainDlg.h \
						JobSubmitter.h

SOURCES	+= 	MainDlg.cpp \
						ServerInfo.cpp \
					 	main.cpp
					 	
FORMS		= 		maindlg.ui

QT           += xml

					
