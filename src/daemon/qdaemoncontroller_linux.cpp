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

#include "QtDaemon/private/qdaemoncontroller_p.h"
#include "QtDaemon/private/qdaemondbusinterface_p.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qprocess.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>

QT_DAEMON_BEGIN_NAMESPACE

bool QDaemonControllerPrivate::start()
{
    QDaemonDBusInterface dbus(state.service());
    if (dbus.open())  {
        QDBusReply<bool> reply = dbus.call<bool>(QStringLiteral("isRunning"));
        if (reply.isValid() && reply.value())  {
            lastError = QT_DAEMON_TRANSLATE("The daemon is already running.");
            return true;
        }

        lastError = QT_DAEMON_TRANSLATE("The daemon is not responding.");
        return false;
    }

    // The daemon is (most probably) not running, so start it with the proper arguments
    if (!QProcess::startDetached(state.path(), QStringList(), state.directory()))  {
        lastError = QT_DAEMON_TRANSLATE("The daemon failed to start.");
        return false;
    }

    // Repeat the call to make sure the communication is ok
    dbus.setTimeout(state.dbusTimeout() * 1000);
    if (!dbus.open(QDaemonDBusInterface::AutoRetryFlag))  {
        lastError = QT_DAEMON_TRANSLATE("Connection with the daemon couldn't be established. (DBus error: %1)").arg(dbus.error());
        return false;
    }

    QDBusReply<bool> reply = dbus.call<bool>(QStringLiteral("isRunning"));
    return reply.isValid() && reply.value();
}

bool QDaemonControllerPrivate::stop()
{
    QDaemonDBusInterface dbus(state.service());
    if (!dbus.open())  {
        lastError = QT_DAEMON_TRANSLATE("Couldn't acquire the D-Bus interface. Is the daemon running? (DBus error: %1)").arg(dbus.error());
        return false;
    }

    QDBusReply<bool> reply = dbus.call<bool>(QStringLiteral("stop"));
    return reply.isValid() && reply.value();
}

bool QDaemonControllerPrivate::install()
{
    QString dbusFilePath = state.dbusConfigPath(), initdFilePath = state.initdScriptPath();
    if (dbusFilePath.isEmpty())  {
        lastError = QT_DAEMON_TRANSLATE("The provided D-Bus configuration path %1 is invalid").arg(dbusFilePath);
        return false;
    }
    if (initdFilePath.isEmpty())  {
        lastError = QT_DAEMON_TRANSLATE("The provided init.d configuration path %1 is invalid").arg(initdFilePath);
        return false;
    }

    QFile dbusConf(dbusFilePath), initdFile(initdFilePath);
    if (dbusConf.exists())  {
        lastError = QT_DAEMON_TRANSLATE("The D-Bus configuration directory %1 already contains a configuration for this service.").arg(dbusFilePath);
        return false;
    }
    if (initdFile.exists())  {
        lastError = QT_DAEMON_TRANSLATE("The provided init.d directory %1 already contains a script for this service.").arg(initdFilePath);
        return false;
    }

    if (!dbusConf.open(QFile::WriteOnly | QFile::Text))  {
        lastError = QT_DAEMON_TRANSLATE("Couldn't open the D-Bus configuration file %1 for writing.").arg(dbusFilePath);
        return false;
    }

    if (!initdFile.open(QFile::WriteOnly | QFile::Text))  {
        lastError = QT_DAEMON_TRANSLATE("Couldn't open the init.d script %1 for writing.").arg(initdFilePath);
        dbusConf.remove();		// Remove the created dbus configuration
        return false;
    }

    // We have opened both files, read the templates
    QFile dbusTemplate(QStringLiteral(":/resources/dbus")), initdTemplate(QStringLiteral(":/resources/init"));

    // We don't expect resources to be inaccessible, but who knows ...
    if (!dbusTemplate.open(QFile::ReadOnly | QFile::Text) || !initdTemplate.open(QFile::ReadOnly | QFile::Text))  {
        lastError = QT_DAEMON_TRANSLATE("Couldn't read the daemon's resources!");
        return false;
    }

    // Read the dbus configuration, do the substitution and write to disk
    QTextStream fin(&dbusTemplate), fout(&dbusConf);
    QString data = fin.readAll();
    data.replace(QStringLiteral("%%SERVICE_NAME%%"), state.service());
    fout << data;

    if (fout.status() != QTextStream::Ok)  {
        lastError = QT_DAEMON_TRANSLATE("An error occured while writing the D-Bus configuration. Installation may be broken.");
        fout.resetStatus();
    }

    // Set the permissions for the dbus configuration
    if (!dbusConf.setPermissions(QFile::WriteOwner | QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther))
        lastError = QT_DAEMON_TRANSLATE("An error occured while setting the permissions for the D-Bus configuration. Installation may be broken");

    // Switch IO devices
    fin.setDevice(&initdTemplate);
    fout.setDevice(&initdFile);

    // Read the init.d script, do the substitution and write to disk
    data = fin.readAll();
    data.replace(QStringLiteral("%%CONTROLLER%%"), QCoreApplication::applicationFilePath())
        .replace(QStringLiteral("%%DAEMON%%"), state.executable())
        .replace(QStringLiteral("%%NAME%%"), state.name())
        .replace(QStringLiteral("%%DESCRIPTION%%"), state.description());
    fout << data;

    if (fout.status() != QTextStream::Ok)
        lastError = QT_DAEMON_TRANSLATE("An error occured while writing the init.d script. Installation may be broken.");

    // Set the permissions for the init.d script
    if (!initdFile.setPermissions(QFile::WriteOwner | QFile::ExeOwner | QFile::ReadOwner | QFile::ReadGroup | QFile::ExeGroup | QFile::ReadOther | QFile::ExeOther))
        lastError = QT_DAEMON_TRANSLATE("An error occured while setting the permissions for the init.d script. Installation may be broken.");

    return state.save();
}

bool QDaemonControllerPrivate::uninstall()
{
    QString dbusFilePath = state.dbusConfigPath(), initdFilePath = state.initdScriptPath();

    QFile dbusConf(dbusFilePath), initdFile(initdFilePath);
    if (dbusFilePath.isEmpty() || (dbusConf.exists() && !dbusConf.remove()))  {
        lastError = QT_DAEMON_TRANSLATE("Couldn't remove the D-Bus configuration file %1 for this service.").arg(dbusFilePath);
        return false;
    }
    if (initdFilePath.isEmpty() || (initdFile.exists() && !initdFile.remove()))  {
        lastError = QT_DAEMON_TRANSLATE("Couldn't remove the init.d script %1 for this service.").arg(initdFilePath);
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
