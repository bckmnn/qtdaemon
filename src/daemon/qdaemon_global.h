#ifndef QDAEMON_GLOBAL_H
#define QDAEMON_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtCore/qmetatype.h>

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
    UnknownStatus,
    RunningStatus,
    NotRunningStatus
};

enum DaemonScope
{
    SystemScope,
    UserScope
};

enum DaemonFlag
{
    // Windows
    UpdatePathFlag = 0x01,
};

Q_DECLARE_FLAGS(DaemonFlags, DaemonFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(DaemonFlags)

QDataStream & operator << (QDataStream & out, const DaemonFlags &);
QDataStream & operator >> (QDataStream & in, DaemonFlags &);

#ifdef QT_BUILD_DAEMON_LIB
#define QT_DAEMON_TRANSLATE(text) QCoreApplication::translate("QtDaemon", QT_TRANSLATE_NOOP("QtDaemon", text))
#endif

QT_DAEMON_END_NAMESPACE

Q_DECLARE_METATYPE(QT_DAEMON_PREPEND_NAMESPACE(DaemonStatus))

#endif // QDAEMON_GLOBAL_H
