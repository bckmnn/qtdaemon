#include "QtDaemon/qdaemon.h"
#include "QtDaemon/private/qdaemon_p.h"

#include <QCoreApplication>

QT_DAEMON_BEGIN_NAMESPACE

QDaemon::QDaemon(const QString & name)
    : QObject(qApp), d_ptr(new QDaemonPrivate(name, this))
{
    Q_ASSERT_X(qApp, Q_FUNC_INFO, "You must create the application object first.");

    d_ptr->state.load();

    QObject::connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(_q_stop()));
    QMetaObject::invokeMethod(this, "_q_start", Qt::QueuedConnection);     // Queue the daemon start up for when the application is ready
}

QT_DAEMON_END_NAMESPACE

#include "moc_qdaemon.cpp"
