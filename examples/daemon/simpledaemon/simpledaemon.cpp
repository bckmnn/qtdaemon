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

#include "simpledaemon.h"

#include <QStringList>

SimpleDaemon::SimpleDaemon(QObject * parent)
    : QObject(parent), logFile("simpledaemon.log"), out(stdout)
{
    if (logFile.open(QFile::WriteOnly | QFile::Append | QFile::Text))
        out.setDevice(&logFile);
}

void SimpleDaemon::onDaemonReady(const QStringList & arguments)
{
    out << QStringLiteral("The daemon is ready. Arguments: %1").arg(arguments.join(' ')) << endl;
}

void SimpleDaemon::onStarted()
{
    out << QStringLiteral("The daemon was started.") << endl;
}

void SimpleDaemon::onStopped()
{
    out << QStringLiteral("The daemon was stopped.") << endl;
}

void SimpleDaemon::onInstalled()
{
    out << QStringLiteral("The daemon was installed.") << endl;
}

void SimpleDaemon::onUninstalled()
{
    out << QStringLiteral("The daemon was uninstalled.") << endl;
}

void SimpleDaemon::onStatus(QtDaemon::DaemonStatus status)
{
    out << QStringLiteral("The daemon is %1").arg(status == QtDaemon::RunningStatus ? QStringLiteral("running") : QStringLiteral("not running")) << endl;
}

void SimpleDaemon::onError(const QString & error)
{
    out << QStringLiteral("An error occured: %1").arg(error) << endl;
}
