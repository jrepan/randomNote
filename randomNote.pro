TEMPLATE = app

QT += qml quick widgets multimedia

QMAKE_CXXFLAGS += -std=c++14
LIBS += -lfftw3f

SOURCES += main.cpp \
    musicallogic.cpp

RESOURCES += qml.qrc \
    notes.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

DISTFILES +=

HEADERS += \
    musicallogic.h
