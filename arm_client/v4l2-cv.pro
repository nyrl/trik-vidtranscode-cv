VERSION = "0.0.1-trunk"

TEMPLATE = app

QT     -= gui
CONFIG += debug_and_release \
          warn_on
CONFIG -= warn_off
debug:CONFIG += console

TARGET  = v4l2-cv

INCLUDEPATH += include
SOURCES += src/main.cpp
HEADERS += include/internal/main.h


unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += "'libcodecengine-client >= 0.0.1'"

    target.path = $$[INSTALL_ROOT]/bin
    INSTALLS += target

    QMAKE_CXXFLAGS += -std=c++0x -Wno-unused-parameter
}


