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

QDaemonState::Data::Data(const QString & daemonName)
    : name(daemonName), initdPrefix(defaultInitDPath), dbusPrefix(defaultDBusPath), dbusTimeout(defaultDBusTimeout)
{
    qRegisterMetaType<QDaemonFlags>();
    qRegisterMetaTypeStreamOperators<QDaemonFlags>();
}

QDaemonState::QDaemonState(const QString & name)
    : loaded(false), d(name)
{
}

bool QDaemonState::initialize(const QString & exe, const QStringList & arguments)
{
    QFileInfo info(exe);
    if (!info.isFile() || !info.isExecutable())
        return false;   // Not a file, not an executable

    setPath(exe);
    setArguments(arguments);

    // Construct the service name
    QByteArray path = d.path.toUtf8();
    quint16 checksum = qChecksum(path.data(), path.size());

    QString serviceBase, organizationDomain = QCoreApplication::organizationDomain();
    if (!organizationDomain.isEmpty())  {
        QStringList elements = organizationDomain.split(QLatin1Char('.'));
        std::reverse(elements.begin(), elements.end());

        serviceBase = elements.join(QLatin1Char('.'));
    }
    else
        serviceBase = QStringLiteral("io.qt.QtDaemon");

    d.service = QStringLiteral("%1.%2-%3").arg(serviceBase).arg(info.fileName()).arg(checksum);

    return true;
}

void QDaemonState::setPath(const QString & path)
{
    QFileInfo info(path);
    d.path = info.absoluteFilePath();
    d.executable = info.fileName();
    d.directory = info.dir().absolutePath();
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

void QDaemonState::generatePListPath()
{
    if (d.flags.testFlag(AgentFlag))  {
        if (d.flags.testFlag(UserAgentFlag))
            d.plistPath = QStringLiteral("%1/Library/LaunchAgents").arg(QDir::homePath());
        else
            d.plistPath = QStringLiteral("/Library/LaunchAgents");
    }
    else
        d.plistPath = QStringLiteral("/Library/LaunchDaemons");
}

bool QDaemonState::load()
{
    QString organizationName = QCoreApplication::organizationName();
    if (organizationName.isEmpty())
        return false;

    QSettings settings(QSettings::SystemScope, organizationName, d.name);
    settings.setFallbacksEnabled(false);

    if (settings.allKeys().size() <= 0)
        return false;

    // Deserialize the settings
    d.path = settings.value(QStringLiteral("Path")).value<QString>();
    d.executable = settings.value(QStringLiteral("Executable")).value<QString>();
    d.directory = settings.value(QStringLiteral("Directory")).value<QString>();
    d.description = settings.value(QStringLiteral("Description")).value<QString>();
    d.service = settings.value(QStringLiteral("Service")).value<QString>();
    d.arguments = settings.value(QStringLiteral("Arguments")).value<QStringList>();
    d.flags = settings.value(QStringLiteral("Flags")).value<QDaemonFlags>();

#if defined(Q_OS_LINUX)
    d.initdPrefix = settings.value(QStringLiteral("InitDPrefix")).value<QString>();
    d.dbusPrefix = settings.value(QStringLiteral("DBusPrefix")).value<QString>();
    d.dbusTimeout = settings.value(QStringLiteral("DBusTimeout")).value<qint32>();
#elif defined(Q_OS_OSX)
    d.plistPath = settings.value(QStringLiteral("PListFilePath")).value<QString>();
#endif

    loaded = !d.path.isEmpty() && !d.executable.isEmpty() && !d.directory.isEmpty() && !d.service.isEmpty();
    return loaded;
}

bool QDaemonState::save() const
{
    if (d.path.isEmpty() || d.executable.isEmpty() || d.directory.isEmpty() || d.service.isEmpty())
        return false;

    QSettings settings(QSettings::SystemScope, QCoreApplication::organizationName(), d.name);
    settings.setFallbacksEnabled(false);

    // Serialize the settings
    settings.setValue(QStringLiteral("Path"), d.path);
    settings.setValue(QStringLiteral("Executable"), d.executable);
    settings.setValue(QStringLiteral("Directory"), d.directory);
    settings.setValue(QStringLiteral("Description"), d.description);
    settings.setValue(QStringLiteral("Service"), d.service);
    settings.setValue(QStringLiteral("Arguments"), d.arguments);
    settings.setValue(QStringLiteral("Flags"), QVariant::fromValue(d.flags));

#if defined (Q_OS_LINUX)
    settings.setValue(QStringLiteral("InitDPrefix"), d.initdPrefix);
    settings.setValue(QStringLiteral("DBusPrefix"), d.dbusPrefix);
    settings.setValue(QStringLiteral("DBusTimeout"), d.dbusTimeout);
#elif defined(Q_OS_OSX)
    settings.setValue(QStringLiteral("PListFilePath"), d.plistPath);
#endif

    return true;
}

void QDaemonState::clear()
{
    QString organizationName = QCoreApplication::organizationName();
    if (organizationName.isEmpty())
        return;

    QSettings settings(QSettings::SystemScope, organizationName, d.name);
    settings.setFallbacksEnabled(false);
    settings.clear();
}

QDataStream & operator << (QDataStream & out, const QDaemonFlags & flags)
{
    out << static_cast<QDaemonFlags::Int>(flags);
    return out;
}

QDataStream & operator >> (QDataStream & in, QDaemonFlags & flags)
{
    QDaemonFlags::Int value;
    in >> value;

    flags = QDaemonFlags(value);
    return in;
}

QT_DAEMON_END_NAMESPACE
