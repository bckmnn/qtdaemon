INCLUDEPATH += $$PWD

QT = core

unix:!macx  {
    QT += dbus
}

SOURCES += \
    $$PWD/qdaemonapplication.cpp \
    $$PWD/qdaemonlog.cpp \
    $$PWD/qdaemoncontroller.cpp \
    $$PWD/qabstractdaemonbackend.cpp \
    $$PWD/qdaemonstate.cpp \
    $$PWD/qdaemoncontroller_linux.cpp \
    $$PWD/qdaemondbusinterface.cpp \
    $$PWD/qdaemon.cpp \
    $$PWD/qdaemon_linux.cpp

PUBLIC_HEADERS += \
    $$PWD/qdaemon_global.h \
    $$PWD/qdaemonapplication.h \
    $$PWD/qdaemonlog.h \
    $$PWD/qdaemoncontroller.h \
    $$PWD/qdaemon.h

PRIVATE_HEADERS += \
    $$PWD/qabstractdaemonbackend.h \
    $$PWD/qdaemonapplication_p.h \
    $$PWD/qdaemonlog_p.h \
    $$PWD/qdaemoncontroller_p.h \
    $$PWD/qdaemonstate_p.h \
    $$PWD/qdaemondbusinterface_p.h \
    $$PWD/qdaemon_p.h

unix:RESOURCES += qdaemon.qrc

macx {
    SOURCES += \
        $$PWD/controllerbackend_osx.cpp \
        $$PWD/daemonbackend_osx.cpp

    PRIVATE_HEADERS += \
        $$PWD/controllerbackend_osx.h \
        $$PWD/daemonbackend_osx.h
} else: unix {
    SOURCES += \
        $$PWD/controllerbackend_linux.cpp \
        $$PWD/daemonbackend_linux.cpp

    PRIVATE_HEADERS += \
        $$PWD/controllerbackend_linux.h \
        $$PWD/daemonbackend_linux.h


    target.path = /usr/lib
    INSTALLS += target

    DISTFILES += \
        resources/init \
        resources/dbus
} else: win32 {
    SOURCES += \
        $$PWD/controllerbackend_win.cpp \
        $$PWD/daemonbackend_win.cpp

    PRIVATE_HEADERS += \
        $$PWD/controllerbackend_win.h \
        $$PWD/daemonbackend_win.h

    LIBS += -luser32 -ladvapi32
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
