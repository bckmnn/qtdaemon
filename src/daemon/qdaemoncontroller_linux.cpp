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

#include "qdaemonlog.h"
#include "QtDaemon/private/qdaemoncontroller_p.h"
#include "QtDaemon/private/qdaemondbusinterface_p.h"

#include <QCoreApplication>
#include <QTextStream>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QFileInfo>

QT_DAEMON_BEGIN_NAMESPACE

QDaemonControllerPrivate::QDaemonControllerPrivate(const QString & name, QDaemonController * q)
    : q_ptr(q), state(name)
{
}

bool QDaemonControllerPrivate::start()
{
    QDaemonDBusInterface dbus(state.service());
    if (dbus.open())  {
        QDBusReply<bool> reply = dbus.call<bool>(QStringLiteral("isRunning"));
        if (reply.isValid() && reply.value())
            qDaemonLog(QStringLiteral("The daemon is already running."), QDaemonLog::NoticeEntry);
        else
            qDaemonLog(QStringLiteral("The daemon is not responding."), QDaemonLog::ErrorEntry);

        return false;
    }

    // The daemon is (most probably) not running, so start it with the proper arguments
    if (!QProcess::startDetached(state.path(), state.arguments(), state.directory()))  {
        qDaemonLog(QStringLiteral("The daemon failed to start."), QDaemonLog::ErrorEntry);
        return false;
    }

    // Repeat the call to make sure the communication is ok
    dbus.setTimeout(state.dbusTimeout() * 1000);
    if (!dbus.open(QDaemonDBusInterface::AutoRetryFlag))  {
        qDaemonLog(QStringLiteral("Connection with the daemon couldn't be established. (%1)").arg(dbus.error()), QDaemonLog::ErrorEntry);
        return false;
    }

    QDBusReply<bool> reply = dbus.call<bool>(QStringLiteral("isRunning"));
    return reply.isValid() && reply.value();
}

bool QDaemonControllerPrivate::stop()
{
    QDaemonDBusInterface dbus(state.service());
    if (!dbus.open())  {
        qDaemonLog(QStringLiteral("Couldn't acquire the DBus interface. Is the daemon running? (%1)").arg(dbus.error()), QDaemonLog::ErrorEntry);
        return false;
    }

    QDBusReply<bool> reply = dbus.call<bool>(QStringLiteral("stop"));
    return reply.isValid() && reply.value();
}

bool QDaemonControllerPrivate::install()
{
    QString dbusFilePath = state.dbusConfigPath(), initdFilePath = state.initdScriptPath();
    if (dbusFilePath.isEmpty() || initdFilePath.isEmpty())  {
        qDaemonLog(QStringLiteral("The provided D-Bus path and/or init.d is invalid"), QDaemonLog::ErrorEntry);
        return false;
    }

    QFile dbusConf(dbusFilePath), initdFile(initdFilePath);
    if (dbusConf.exists())  {
        qDaemonLog(QStringLiteral("The provided D-Bus configuration directory already contains a configuration for this service. Uninstall first"), QDaemonLog::ErrorEntry);
        return false;
    }
    if (initdFile.exists())  {
        qDaemonLog(QStringLiteral("The provided init.d directory already contains a script for this service. Uninstall first"), QDaemonLog::ErrorEntry);
        return false;
    }

    if (!dbusConf.open(QFile::WriteOnly | QFile::Text))  {
        qDaemonLog(QStringLiteral("Couldn't open the D-Bus configuration file for writing (%1).").arg(dbusFilePath), QDaemonLog::ErrorEntry);
        return false;
    }

    if (!initdFile.open(QFile::WriteOnly | QFile::Text))  {
        qDaemonLog(QStringLiteral("Couldn't open the init.d script for writing (%1).").arg(initdFilePath), QDaemonLog::ErrorEntry);
        dbusConf.remove();		// Remove the created dbus configuration
        return false;
    }

    // We have opened both files, read the templates
    QFile dbusTemplate(QStringLiteral(":/resources/dbus")), initdTemplate(QStringLiteral(":/resources/init"));

    // We don't expect resources to be inaccessible, but who knows ...
    if (!dbusTemplate.open(QFile::ReadOnly | QFile::Text) || !initdTemplate.open(QFile::ReadOnly | QFile::Text))  {
        qDaemonLog(QStringLiteral("Couldn't read the daemon's resources!"), QDaemonLog::ErrorEntry);
        return false;
    }

    // Read the dbus configuration, do the substitution and write to disk
    QTextStream fin(&dbusTemplate), fout(&dbusConf);
    QString data = fin.readAll();
    data.replace(QStringLiteral("%%SERVICE_NAME%%"), state.service());
    fout << data;

    if (fout.status() != QTextStream::Ok)  {
        qDaemonLog(QStringLiteral("An error occured while writing the D-Bus configuration. Installation may be broken."), QDaemonLog::WarningEntry);
        fout.resetStatus();
    }

    // Set the permissions for the dbus configuration
    if (!dbusConf.setPermissions(QFile::WriteOwner | QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther))
        qDaemonLog(QStringLiteral("An error occured while setting the permissions for the D-Bus configuration. Installation may be broken"), QDaemonLog::WarningEntry);

    // Switch IO devices
    fin.setDevice(&initdTemplate);
    fout.setDevice(&initdFile);

    // Read the init.d script, do the substitution and write to disk
    /*QStringList arguments = parser.positionalArguments();
    if (arguments.size() > 0)
        arguments.prepend(QStringLiteral("--"));*/

    data = fin.readAll();
    data.replace(QStringLiteral("%%CONTROLLER%%"), QCoreApplication::applicationFilePath())
        .replace(QStringLiteral("%%DAEMON%%"), state.executable())
        .replace(QStringLiteral("%%NAME%%"), state.name())
        .replace(QStringLiteral("%%DESCRIPTION%%"), state.description());
    fout << data;

    if (fout.status() != QTextStream::Ok)
        qDaemonLog(QStringLiteral("An error occured while writing the init.d script. Installation may be broken."), QDaemonLog::WarningEntry);

    // Set the permissions for the init.d script
    if (!initdFile.setPermissions(QFile::WriteOwner | QFile::ExeOwner | QFile::ReadOwner | QFile::ReadGroup | QFile::ExeGroup | QFile::ReadOther | QFile::ExeOther))
        qDaemonLog(QStringLiteral("An error occured while setting the permissions for the init.d script. Installation may be broken."), QDaemonLog::WarningEntry);

    return state.save();
}

bool QDaemonControllerPrivate::uninstall()
{
    QString dbusFilePath = state.dbusConfigPath(), initdFilePath = state.initdScriptPath();

    QFile dbusConf(dbusFilePath), initdFile(initdFilePath);
    if (dbusFilePath.isEmpty() || (dbusConf.exists() && !dbusConf.remove()))  {
        qDaemonLog(QStringLiteral("Couldn't remove the D-Bus configuration file for this service (%1).").arg(dbusFilePath), QDaemonLog::ErrorEntry);
        return false;
    }
    if (initdFilePath.isEmpty() || (initdFile.exists() && !initdFile.remove()))  {
        qDaemonLog(QStringLiteral("Couldn't remove the init.d script for this service (%1).").arg(initdFilePath), QDaemonLog::ErrorEntry);
        return false;
    }

    state.clear();
    return true;
}

QtDaemon::DaemonStatus QDaemonControllerPrivate::status()
{
    QDaemonDBusInterface dbus(state.service());
    if (!dbus.open())
        return RunningStatus;

    QDBusReply<bool> reply = dbus.call<bool>(QStringLiteral("isRunning"));
    return reply.isValid() && reply.value() ? RunningStatus : NotRunningStatus;
}

QT_DAEMON_END_NAMESPACE
