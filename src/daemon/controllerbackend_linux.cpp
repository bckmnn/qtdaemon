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

#include "controllerbackend_linux.h"
#include "daemonbackend_linux.h"
#include "qdaemonapplication.h"
#include "qdaemonlog.h"

#include <QtCore/qmetaobject.h>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcommandlineoption.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qprocess.h>
#include <QtCore/qthread.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qelapsedtimer.h>

#include <QtDBus/qdbusconnection.h>
#include <QtDBus/qdbuserror.h>
#include <QtDBus/qdbusinterface.h>
#include <QtDBus/qdbusreply.h>

QT_DAEMON_BEGIN_NAMESPACE

const QString ControllerBackendLinux::initdPrefix = QStringLiteral("initd-prefix");
const QString ControllerBackendLinux::dbusPrefix = QStringLiteral("dbus-prefix");

ControllerBackendLinux::ControllerBackendLinux(QCommandLineParser & parser, bool autoQuit)
    : QAbstractControllerBackend(parser, autoQuit),
      dbusPrefixOption(dbusPrefix, QCoreApplication::translate("main", "Sets the path for the installed dbus configuration file"), QStringLiteral("path"), "defaultDBusPath"),
      initdPrefixOption(initdPrefix, QCoreApplication::translate("main", "Sets the path for the installed init.d script"), QStringLiteral("path"), "defaultInitPath")
{
    parser.addOption(dbusPrefixOption);
    parser.addOption(initdPrefixOption);
}

bool ControllerBackendLinux::start()
{
    QMetaObject::invokeMethod(qApp, "started", Qt::QueuedConnection);
    return true;
}

bool ControllerBackendLinux::stop()
{


    QMetaObject::invokeMethod(qApp, "stopped", Qt::QueuedConnection);
    return true;
}

bool ControllerBackendLinux::install()
{


    QMetaObject::invokeMethod(qApp, "installed", Qt::QueuedConnection);
    return true;
}

bool ControllerBackendLinux::uninstall()
{


    QMetaObject::invokeMethod(qApp, "uninstalled", Qt::QueuedConnection);
    return true;
}

DaemonStatus ControllerBackendLinux::status()
{
    return NotRunningStatus;
}

QT_DAEMON_END_NAMESPACE
