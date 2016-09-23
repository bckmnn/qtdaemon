TARGET = QtDaemon
MODULE = daemon

QT = core
unix:!macx  {
    QT += dbus
}

QMAKE_CXXFLAGS = # For some reason there's flag mixing if we don't empty this var

DEFINES += QT_NO_FOREACH

include($$PWD/daemon-lib.pri)

#load(qt_build_config)
load(qt_module)

