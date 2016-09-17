#ifndef QDAEMON_H
#define QDAEMON_H

#include "QtDaemon/qdaemon-global.h"

#include <QObject>

QT_DAEMON_BEGIN_NAMESPACE

class QDaemonPrivate;
class QDaemon : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDaemon)
    Q_DISABLE_COPY(QDaemon)

    Q_PRIVATE_SLOT(d_func(), void _q_start())
    Q_PRIVATE_SLOT(d_func(), void _q_stop())

public:
    explicit QDaemon(const QString &);

signals:
    void ready(const QStringList &);

private:
    QDaemonPrivate * d_ptr;
};

QT_DAEMON_END_NAMESPACE

#endif // QDAEMON_H
