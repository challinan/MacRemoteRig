QT       += core serialport widgets

CONFIG += c++17

QMAKE_MACOSX_DEPLOYMENT_TARGET = "13.0"

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

macx: QT_CONFIG -= no-pkg-config

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
    morse_table.h \
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

# Support for Hamlib Library in our workspace
macx: LIBS += -L/usr/local/lib -lhamlib
win32: LIBS += -L$$PWD/../../Hamlib/install-dir-temp/lib -lhamlib -lws2_32
INCLUDEPATH += $$PWD/../../Hamlib/include
DEPENDPATH += $$PWD/../../Hamlib/include

# GStreamer support
INCLUDEPATH += /opt/homebrew/Cellar/gstreamer/1.22.5/include/gstreamer-1.0 /opt/homebrew/Cellar/glib/2.76.4/include \
/opt/homebrew/Cellar/glib/2.76.4/include/glib-2.0 /opt/homebrew/Cellar/glib/2.76.4/lib/glib-2.0/include \
/opt/homebrew/opt/gettext/include /opt/homebrew/Cellar/pcre2/10.42/include \
/Library/Developer/CommandLineTools/SDKs/MacOSX13.sdk/usr/include/ffi

LIBS += -L/opt/homebrew/Cellar/gstreamer/1.22.5/lib -L/opt/homebrew/Cellar/glib/2.76.4/lib \
-L/opt/homebrew/opt/gettext/lib -lgstreamer-1.0 -Wl,-rpath,/opt/homebrew/Cellar/gstreamer/1.22.5/lib \
-lgobject-2.0 -lglib-2.0 -lintl

# RTSP SERVER SUPPORT
LIBS += -lgstrtspserver-1.0

# message(CONFIG = $$CONFIG)
message(QMAKE_MAC_SDK = $$QMAKE_MAC_SDK)
message(QMAKE_MAC_SDK_PATH = $$QMAKE_MAC_SDK_PATH)
message(QMAKE_MAC_SDK_PLATFORM_PATH = $$QMAKE_MAC_SDK_PLATFORM_PATH)
message(QMAKE_MAC_SDK_VERSION = $$QMAKE_MAC_SDK_VERSION)
