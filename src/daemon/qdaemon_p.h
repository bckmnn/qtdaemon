#ifndef QDAEMON_P_H
#define QDAEMON_P_H

#include "QtDaemon/qdaemon-global.h"
#include "QtDaemon/private/qdaemonstate_p.h"
#include "QtDaemon/private/qdaemondbusinterface_p.h"

QT_DAEMON_BEGIN_NAMESPACE

class QDaemon;
class Q_DAEMON_LOCAL QDaemonPrivate
{
    Q_DECLARE_PUBLIC(QDaemon)

public:
    explicit QDaemonPrivate(const QString &, QDaemon *);

    void _q_start();
    void _q_stop();

private:
    QDaemon * q_ptr;
    QDaemonState state;
    QDaemonDBusInterfaceProvider dbus;
};

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

inline QDaemonPrivate::QDaemonPrivate(const QString & name, QDaemon * q)
    : q_ptr(q), state(name)
{
}

QT_DAEMON_END_NAMESPACE

#endif // QDAEMON_P_H
