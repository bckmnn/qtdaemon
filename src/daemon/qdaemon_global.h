#ifndef QDAEMON_GLOBAL_H
#define QDAEMON_GLOBAL_H

#include <QtCore/qglobal.h>

// Namespace utilities
#define QT_DAEMON_NAMESPACE QtDaemon
#define QT_DAEMON_BEGIN_NAMESPACE \
    QT_BEGIN_NAMESPACE \
    namespace QT_DAEMON_NAMESPACE {
#define QT_DAEMON_END_NAMESPACE \
    } \
    QT_END_NAMESPACE
#define QT_DAEMON_PREPEND_NAMESPACE(name) \
    QT_PREPEND_NAMESPACE(QT_DAEMON_NAMESPACE)::name

// Import/export
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

// Enums and globals
QT_DAEMON_BEGIN_NAMESPACE

enum DaemonStatus
{
    DaemonRunning,
    DaemonNotRunning
};

enum DaemonFlag
{
    // Windows
    UpdatePathFlag = 0x01,
    // macOS
    AgentFlag = 0x81,
    UserAgentFlag = 0x83        // Overlap AgentFlag if it's not set
};
Q_DECLARE_FLAGS(QDaemonFlags, DaemonFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDaemonFlags)

QT_DAEMON_END_NAMESPACE

#endif // QDAEMON_GLOBAL_H
