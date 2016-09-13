#ifndef QDAEMON_GLOBAL_H
#define QDAEMON_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_DAEMON_STATICLIB)
    #if defined(QT_BUILD_DAEMON_LIB)
        #define Q_DAEMON_EXPORT Q_DECL_EXPORT
        #define Q_DAEMON_LOCAL Q_DECL_HIDDEN
    #else
        #define Q_DAEMON_EXPORT Q_DECL_IMPORT
    #endif
#else
    #define Q_DAEMON_EXPORT
    #define Q_DAEMON_LOCAL
#endif

namespace QtDaemon
{
    enum DaemonStatus
    {
        DaemonRunning,
        DaemonNotRunning
    };

    enum ControllerOption
    {
        // Lnux only
        InitDPrefixOption,
        DBusPrefixOption,
        // Windows only
        UpdatePathOption,
        // macOS only
        AgentOption,
        UserOption
    };
}

QT_END_NAMESPACE

#endif // QDAEMON_GLOBAL_H
