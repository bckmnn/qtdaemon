load(qt_build_config)

TARGET = QtDaemon
MODULE = daemon

QT = core
unix:!macx  {
    QT += dbus

    QMAKE_CXXFLAGS_DEBUG += -std=c++03 -pedantic-errors # Just make sure we're not using *anything* from c++11
}

DEFINES += QT_NO_FOREACH

include($$PWD/daemon-lib.pri)

load(qt_module)
