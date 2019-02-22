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

#include "QtDaemon/private/qdaemonstate_p.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qsettings.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <QtCore/qdatastream.h>

QT_DAEMON_BEGIN_NAMESPACE

static const QString defaultInitDPath = QStringLiteral("/etc/init.d");
static const QString defaultDBusPath = QStringLiteral("/etc/dbus-1/system.d");
static const qint32 defaultDBusTimeout = 30;

QDaemonState::Data::Data(const QDaemonState & state)
    : path(QStringLiteral("Path"), state),
      description(QStringLiteral("Description"), state),
#ifdef Q_OS_LINUX
      initdPrefix(QStringLiteral("InitDPrefix"), defaultInitDPath, state),
      dbusPrefix(QStringLiteral("DBusPrefix"), defaultDBusPath, state),
      dbusTimeout(QStringLiteral("DBusTimeout"), defaultDBusTimeout, state),
#endif
      arguments(QStringLiteral("Arguments"), state),
      flags(QStringLiteral("Flags"), state),
      scope(DaemonScope::SystemScope)
{
    qRegisterMetaType<DaemonFlags>();
    qRegisterMetaTypeStreamOperators<DaemonFlags>();

    // Update the directory and executable file name from the path
    path.observe([this] (const QString & exe) -> void  {
        QFileInfo info(exe);
        if (!info.isFile() || !info.isExecutable())
            return;   // Not a file, not an executable

        executable = info.fileName();
        directory = info.dir().absolutePath();
    });

    name = QCoreApplication::applicationName();
    QObject::connect(qApp, &QCoreApplication::applicationNameChanged, [this] () -> void {
        name = QCoreApplication::applicationName();
    });
}

QDaemonState::QDaemonState()
    : d(*this)
{
}

bool QDaemonState::create(const QString & exe, const QStringList & arguments)
{
    QFileInfo info(exe);
    if (!info.isFile() || !info.isExecutable())
        return false;   // Not a file, not an executable

    d.path = info.absoluteFilePath();
    d.arguments = arguments;
    d.service = generateServiceName();

    return true;
}

bool QDaemonState::isKnown() const
{
    return !QString(d.path).isEmpty() && !QString(d.executable).isEmpty() && !QString(d.directory).isEmpty() && !d.service.isEmpty();
}


QString QDaemonState::dbusConfigPath() const
{
    QDir dbusPrefix(d.dbusPrefix);
    if (!dbusPrefix.makeAbsolute() || !dbusPrefix.exists() || d.service.isEmpty())
        return QString();

    return dbusPrefix.filePath(d.service + QStringLiteral(".conf"));
}

QString QDaemonState::initdScriptPath() const
{
    QDir initdPrefix(d.initdPrefix);
    if (!initdPrefix.makeAbsolute() || !initdPrefix.exists() || d.executable.isEmpty())
        return QString();

    return initdPrefix.filePath(d.executable);
}

QString QDaemonState::plistPath() const
{
    QDir prefix(plistPrefix());
    if (!prefix.exists() || d.executable.isEmpty())
        return QString();

    return prefix.filePath(d.service);
}

QString QDaemonState::generateServiceName() const
{
    // Construct the service name
    QString serviceBase, organizationDomain = QCoreApplication::organizationDomain();
    if (!organizationDomain.isEmpty())  {
        QStringList elements = organizationDomain.split(QLatin1Char('.'));
        std::reverse(elements.begin(), elements.end());

        serviceBase = elements.join(QLatin1Char('.'));
    }
    else
        serviceBase = QStringLiteral("QtDaemon");

    return QStringLiteral("%1.%2").arg(serviceBase, d.name);
}
/*
bool QDaemonState::load()
{
    QString name = QCoreApplication::applicationName();
    if (name.isEmpty() || name == QFileInfo(QCoreApplication::applicationFilePath()).baseName())  {
        qWarning("QCoreApplication::applicationName() must be set");
        return false;
    }

    QString organization = QCoreApplication::organizationName();
    if (organization.isEmpty())  {
        qWarning("QCoreApplication::organizationName() must not return an empty string.");
        return false;
    }

    // IMPORTANT:
    // We are going to get speculative loads (on construction)
    // Try to load the settings into the state


    if (settings.allKeys().size() <= 0)
        return false;

    // Deserialize the settings
    d.path = settings.value(QStringLiteral("Path")).value<QString>();
    d.executable = settings.value(QStringLiteral("Executable")).value<QString>();
    d.directory = settings.value(QStringLiteral("Directory")).value<QString>();
    d.description = settings.value(QStringLiteral("Description")).value<QString>();
    d.service = settings.value(QStringLiteral("Service")).value<QString>();
    d.arguments = settings.value(QStringLiteral("Arguments")).value<QStringList>();
    d.flags = settings.value(QStringLiteral("Flags")).value<DaemonFlags>();

#if defined(Q_OS_LINUX)
    d.initdPrefix = settings.value(QStringLiteral("InitDPrefix")).value<QString>();
    d.dbusPrefix = settings.value(QStringLiteral("DBusPrefix")).value<QString>();
    d.dbusTimeout = settings.value(QStringLiteral("DBusTimeout")).value<qint32>();
#elif defined(Q_OS_OSX)
    d.plistPrefix = settings.value(QStringLiteral("PListPrefix")).value<QString>();
#endif

    d.loaded = !d.path.isEmpty() && !d.executable.isEmpty() && !d.directory.isEmpty() && !d.service.isEmpty();
    return d.loaded;
}

bool QDaemonState::save() const
{
#if defined(Q_OS_WIN)
    if (QCoreApplication::organizationName().isEmpty())  {
        qWarning("You should provide an organization name! QCoreApplication::organizationName() must not return an empty string.");
        return false;
    }
#endif

    if (d.path.isEmpty() || d.executable.isEmpty() || d.directory.isEmpty() || d.service.isEmpty())
        return false;

    QSettings settings(d.scope == QtDaemon::UserScope ? QSettings::UserScope : QSettings::SystemScope,
            QCoreApplication::organizationName(), d.name);
    settings.setFallbacksEnabled(false);

    // Serialize the settings
    settings.setValue(QStringLiteral("Path"), d.path);
    settings.setValue(QStringLiteral("Description"), d.description);
    settings.setValue(QStringLiteral("Arguments"), d.arguments);
    settings.setValue(QStringLiteral("Flags"), QVariant::fromValue(d.flags));

#if defined(Q_OS_LINUX)
    settings.setValue(QStringLiteral("InitDPrefix"), d.initdPrefix);
    settings.setValue(QStringLiteral("DBusPrefix"), d.dbusPrefix);
    settings.setValue(QStringLiteral("DBusTimeout"), d.dbusTimeout);
#endif

    return true;
}
*/
void QDaemonState::clear()
{
//    QDaemonSettings settings(d.scope, d.name);
//    settings.clear();
}

void QDaemonState::invalidate()
{
    d.path.invalidate();
    d.description.invalidate();
#ifdef Q_OS_LINUX
    d.initdPrefix.invalidate();
    d.dbusPrefix.invalidate();
    d.dbusTimeout.invalidate();
#endif
    d.arguments.invalidate();
    d.flags.invalidate();
}


QString QDaemonState::plistPrefix() const
{
#ifndef Q_OS_OSX
    qWarning("QDaemonState::plistPrefix() is meaningfull only on OSX");
#endif

    switch (d.scope)
    {
    case QtDaemon::UserScope:
        return QStringLiteral("%1/Library/LaunchAgents").arg(QDir::homePath());
    case QtDaemon::SystemScope:
        return QStringLiteral("/Library/LaunchAgents");
    }

    Q_UNREACHABLE();
}

QDataStream & operator << (QDataStream & out, const DaemonFlags & flags)
{
    out << static_cast<DaemonFlags::Int>(flags);
    return out;
}

QDataStream & operator >> (QDataStream & in, DaemonFlags & flags)
{
    DaemonFlags::Int value;
    in >> value;

    flags = DaemonFlags(value);
    return in;
}

QT_DAEMON_END_NAMESPACE
