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

#include "QtDaemon/private/qdaemondbusinterface_p.h"
#include "QtDaemon/qdaemonlog.h"

#include <QString>
#include <QThread>
#include <QElapsedTimer>

#include <QDBusError>

QT_DAEMON_BEGIN_NAMESPACE

static const qint32 dbusPollTime = 1000;				// Poll each second on start

const QString QDaemonDBusInterface::controlInterface = QStringLiteral("io.qt.QtDaemon.Control");
const QString QDaemonDBusInterface::objectPath = QStringLiteral("/");

QDaemonDBusInterface::QDaemonDBusInterface(const QString & serviceName)
    : service(serviceName), dbusTimeout(0), dbus(QDBusConnection::systemBus())
{
    if (!dbus.isConnected())
        qDaemonLog(QStringLiteral("Can't connect to the DBus system bus (%1)").arg(dbus.lastError().message()), QDaemonLog::ErrorEntry);
}

QDaemonDBusInterface::~QDaemonDBusInterface()
{
    close();
}

bool QDaemonDBusInterface::open(const OpenFlags & flags)
{
    if (!dbus.isConnected())
        return false;
    if (!interface.isNull())
        return true;

    // Autoretry loop
    if (flags.testFlag(AutoRetryFlag))  {
        OpenFlags newFlags = flags & ~AutoRetryFlag | NoRetryFlag;

        // Repeat the call to make sure the communication is ok
        QElapsedTimer dbusTimeoutTimer;
        dbusTimeoutTimer.start();

        // Give the daemon some seconds to start its DBus service
        while (!dbusTimeoutTimer.hasExpired(dbusTimeout) && !open(newFlags))
            QThread::msleep(dbusPollTime);  // Wait some time before retrying

        return isValid();
    }

    interface.reset(new QDBusInterface(service, objectPath, controlInterface, dbus));
    if (!interface->isValid())  {
        interface.reset();
        return false;
    }

    return true;
}

void QDaemonDBusInterface::close()
{
    interface.reset();
}

bool QDaemonDBusInterface::isValid() const
{
    return dbus.isConnected() && !interface.isNull();
}

QString QDaemonDBusInterface::error() const
{
    return dbus.lastError().message();
}

QT_DAEMON_END_NAMESPACE
