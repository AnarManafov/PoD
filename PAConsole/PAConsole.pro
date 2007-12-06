TEMPLATE = app
LANGUAGE = C++
VERSION = -1.0.3
INCLUDEPATH = ./Include \
    ${GAW_LOCATION}/include \
    ${GLITE_LOCATION}/include \
    ${GLITE_LOCATION}/externals/include \
    ${LCG_LOCATION}/include/lfc \
    ${XERCESC_LOCATION}/include
HEADERS += src/MainDlg.h \
    src/JobSubmitter.h \
    src/ServerDlg.h \
    src/GridDlg.h \
    src/WorkersDlg.h \
    src/TreeItemContainer.h
SOURCES += src/MainDlg.cpp \
    src/ServerInfo.cpp \
    src/ServerDlg.cpp \
    src/GridDlg.cpp \
    src/WorkersDlg.cpp \
    src/main.cpp
FORMS = res/maindlg.ui \
    res/wgServer.ui \
    res/wgGrid.ui \
    res/wgWorkers.ui
RESOURCES += res/paconsole.qrc
DESTDIR = Build
OBJECTS_DIR = Intermediate
UI_DIR = Intermediate
MOC_DIR = Intermediate
QT += xml
unix:LIBS += -L${GAW_LOCATION}/lib \
    -lglite-api-wrapper
