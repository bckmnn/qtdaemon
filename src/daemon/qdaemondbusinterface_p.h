/****************************************************************************
**
** Copyright (C) 2016 Konstantin Shegunov <kshegunov@gmail.com>
**
** This file is part of the QtDaemon library.
**
** The MIT License (MIT)
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDaemon API. It exists only
// as an implementation detail. This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDAEMONDBUSINTERFACE_P_H
#define QDAEMONDBUSINTERFACE_P_H

#include "QtDaemon/qdaemon_global.h"

#include <QObject>

#include <QDBusConnection>
#include <QDBusAbstractInterface>
#include <QDBusReply>

QT_BEGIN_NAMESPACE
class QDBusAbstractInterface;
QT_END_NAMESPACE

#define QT_DAEMON_DBUS_INTERFACE_KEY     "D-Bus Interface"
#define Q_DAEMON_DBUS_CONTROL_INTERFACE  "io.qt.QtDaemon.Control"

QT_DAEMON_BEGIN_NAMESPACE

class Q_DAEMON_LOCAL QDaemonDBusInterfaceProvider : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QDaemonDBusInterfaceProvider)
    Q_CLASSINFO(QT_DAEMON_DBUS_INTERFACE_KEY, Q_DAEMON_DBUS_CONTROL_INTERFACE)

public:
    QDaemonDBusInterfaceProvider(QObject * = Q_NULLPTR);
    ~QDaemonDBusInterfaceProvider() Q_DECL_OVERRIDE;

    bool create(const QString &);
    void destroy();

    Q_INVOKABLE bool isRunning() const;
    Q_INVOKABLE bool stop();

private:
    QString service;
    QDBusConnection dbus;

};

class Q_DAEMON_LOCAL QDaemonDBusInterface
{
public:
    enum OpenFlag  {
        NoRetryFlag = 0x1,
        AutoRetryFlag = 0x3
    };
    Q_DECLARE_FLAGS(OpenFlags, OpenFlag)

    QDaemonDBusInterface(const QString &);
    ~QDaemonDBusInterface();

    bool open(const OpenFlags & = NoRetryFlag);
    void close();
    bool isValid() const;

    template <class T>
    QDBusReply<T> call(const QString &);

    void setTimeout(qint32);
    qint32 timeout();

    QString error() const;

private:
    QString service;
    qint32 dbusTimeout;
    QDBusConnection dbus;
    QScopedPointer<QDBusAbstractInterface> interface;

    static const QString controlInterface, objectPath;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QDaemonDBusInterface::OpenFlags)

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

template <class T>
inline QDBusReply<T> QDaemonDBusInterface::call(const QString & method)
{
    return interface.isNull() || !interface->isValid() ? QDBusReply<T>() : interface->call(method);
}

inline void QDaemonDBusInterface::setTimeout(qint32 timeout)
{
    dbusTimeout = timeout;
}

inline qint32 QDaemonDBusInterface::timeout()
{
    return dbusTimeout;
}

QT_DAEMON_END_NAMESPACE

#endif // QDAEMONDBUSINTERFACE_P_H
