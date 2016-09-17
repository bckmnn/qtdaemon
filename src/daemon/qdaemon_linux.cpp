#include "qdaemon_p.h"
#include "qdaemon.h"
#include "qdaemondbusinterface_p.h"

#include <QCoreApplication>

QT_DAEMON_BEGIN_NAMESPACE

void QDaemonPrivate::_q_start()
{
    // Create the DBus service
    if (!state.load() || !dbus.create(state.service()))
        qApp->quit();

    // Just emit the ready signal, nothing more to do here
    QMetaObject::invokeMethod(q_func(), "ready", Qt::QueuedConnection, Q_ARG(const QStringList &, qApp->arguments()));
}

void QDaemonPrivate::_q_stop()
{
    dbus.destroy();
}

QT_DAEMON_END_NAMESPACE
