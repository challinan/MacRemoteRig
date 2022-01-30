# Adding statemachine For debug only
QT       += core gui serialport statemachine

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    frequencynudger.cpp \
    frequencypoller.cpp \
    genericdialog.cpp \
    gstreamerlistener.cpp \
    hamlibconnector.cpp \
    main.cpp \
    config_object.cpp \
    mainwindow.cpp \
    spot_delayworker.cpp \
    transmitwindow.cpp \
    tune_dialog.cpp

HEADERS += \
    frequencynudger.h \
    frequencypoller.h \
    genericdialog.h \
    gstreamerlistener.h \
    hamlibconnector.h \
    config_object.h \
    icon_defines.h \
    mainwindow.h \
    spot_delayworker.h \
    transmitwindow.h \
    tune_dialog.h

FORMS += \
    configdialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# QMAKE_MACOSX_DEPLOYMENT_TARGET = "11.0"
QMAKE_MACOSX_DEPLOYMENT_TARGET = "10.15"

# message($$PWD)
# Support for Hamlib library
# macx: LIBS += -L$$PWD/../../../../usr/local/lib/ -lhamlib.2
# INCLUDEPATH += $$PWD/../../../../usr/local/include
# DEPENDPATH += $$PWD/../../../../usr/local/include

# Support for Hamlib Library in our workspace
macx: LIBS += -L$$PWD/../../sandbox/Hamlib/install-dir-temp/lib/ -lhamlib.4
INCLUDEPATH += $$PWD/../../sandbox/Hamlib/install-dir-temp/include
DEPENDPATH += $$PWD/../../sandbox/Hamlib/install-dir-temp/include

# GStreamer support
macx: LIBS += -L/Library/Frameworks/GStreamer.framework/Libraries -lgstreamer-1.0.0 -lglib-2.0.0
INCLUDEPATH += /Library/Frameworks/GStreamer.framework/Headers
