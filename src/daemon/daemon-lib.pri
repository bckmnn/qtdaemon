INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/qdaemonapplication.cpp \
    $$PWD/qdaemoncontroller.cpp \
    $$PWD/qdaemonstate.cpp \
    $$PWD/qdaemon.cpp

PUBLIC_HEADERS += \
    $$PWD/qdaemon_global.h \
    $$PWD/qdaemonapplication.h \
    $$PWD/qdaemoncontroller.h \
    $$PWD/qdaemon.h

PRIVATE_HEADERS += \
    $$PWD/qdaemonapplication_p.h \
    $$PWD/qdaemoncontroller_p.h \
    $$PWD/qdaemon_p.h \
    $$PWD/qdaemonstate_p.h

unix:RESOURCES += qdaemon.qrc

macx {
    SOURCES += \
        $$PWD/qdaemoncontroller_osx.cpp \
        $$PWD/qdaemon_osx.cpp
} else: unix {
    SOURCES += \
        $$PWD/qdaemondbusinterface.cpp \
        $$PWD/qdaemoncontroller_linux.cpp \
        $$PWD/qdaemon_linux.cpp

    PRIVATE_HEADERS += \
        $$PWD/qdaemondbusinterface_p.h

    DISTFILES += \
        resources/init \
        resources/dbus
} else: win32 {
    SOURCES += \
        $$PWD/qdaemonwindowsctrldispatcher.cpp \
        $$PWD/qwindowsservicemanager.cpp \
        $$PWD/qdaemoncontroller_win.cpp \
        $$PWD/qdaemon_win.cpp

    PRIVATE_HEADERS += \
        $$PWD/qdaemonwindowsctrldispatcher_p.h \
        $$PWD/qwindowsservicemanager_p.h

    LIBS += -luser32 -ladvapi32
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
