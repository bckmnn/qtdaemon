INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/qdaemonapplication.cpp \
    $$PWD/qdaemonlog.cpp \
    $$PWD/qdaemoncontroller.cpp \
    $$PWD/qabstractdaemonbackend.cpp \
    $$PWD/qdaemonstate.cpp \
    $$PWD/qdaemon.cpp \
    $$PWD/qdaemonwindowsctrldispatcher.cpp \
    $$PWD/qwindowsservicemanager.cpp

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
        $$PWD/daemonbackend_linux.cpp \
        $$PWD/qdaemondbusinterface.cpp \
        $$PWD/qdaemoncontroller_linux.cpp \
        $$PWD/qdaemon_linux.cpp

    PRIVATE_HEADERS += \
        $$PWD/controllerbackend_linux.h \
        $$PWD/daemonbackend_linux.h \
        $$PWD/qdaemondbusinterface_p.h

    DISTFILES += \
        resources/init \
        resources/dbus
} else: win32 {
    SOURCES += \
        $$PWD/controllerbackend_win.cpp \
        $$PWD/daemonbackend_win.cpp \
        $$PWD/qdaemoncontroller_win.cpp \
        $$PWD/qdaemon_win.cpp

    PRIVATE_HEADERS += \
        $$PWD/controllerbackend_win.h \
        $$PWD/daemonbackend_win.h \
        $$PWD/qdaemonwindowsctrldispatcher_p.h \
        $$PWD/qwindowsservicemanager_p.h

    LIBS += -luser32 -ladvapi32
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
