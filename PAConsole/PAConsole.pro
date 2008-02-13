TEMPLATE = app
LANGUAGE = C++
VERSION = -1.0.4
INCLUDEPATH = ./Include \
    ${GAW_CPPFLAGS}/include
HEADERS += src/MainDlg.h \
    src/JobSubmitter.h \
    src/ServerDlg.h \
    src/GridDlg.h \
    src/WorkersDlg.h \
    src/TreeItemContainer.h \
    src/ServerInfo.h \
    Include/def.h \
    Include/gLiteHelper.h \
    Include/MiscUtils.h \
    Include/FindCfgFile.h \
    Include/SysHelper.h \
    Include/ErrorCode.h \
    Include/Process.h \
    Include/CustomIterator.h \
    Include/stlx.h \
    Include/INet.h \
    Include/JDLHelper.h
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

# -DWANT_NAMESPACES needed by ClassAd
QMAKE_CXXFLAGS += -DWANT_NAMESPACES
DISTFILES += build.sh \
    res/images/grid.png \
    res/images/server.png \
    res/images/workers.png
