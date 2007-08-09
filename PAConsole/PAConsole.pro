TEMPLATE = app
LANGUAGE = C++

VERSION = -0.0.4

INCLUDEPATH = \
                ./Include \
		${GAW_LOCATION}/include/glite-api-wrapper \
		${GLITE_LOCATION}/include \
		${GLITE_LOCATION}/externals/include \
		${LCG_LOCATION}/include/lfc \
		${XERCESC_LOCATION}/include

HEADERS += \
                src/MainDlg.h \
		src/JobSubmitter.h

SOURCES	+= \
                src/MainDlg.cpp \
		src/ServerInfo.cpp \
		src/main.cpp
				 	
FORMS = res/maindlg.ui

DESTDIR = Build

OBJECTS_DIR = Intermediate
UI_DIR = Intermediate
MOC_DIR = Intermediate

QT += xml

unix:LIBS += -L${GAW_LOCATION}/lib -lglite-api-wrapper
