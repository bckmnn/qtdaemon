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
#include "QtDaemon/private/qwindowsservicemanager_p.h"

QT_DAEMON_BEGIN_NAMESPACE

bool QDaemonControllerPrivate::start()
{
    QWindowsServiceManager manager = QWindowsServiceManager::open();
    if (!manager.isValid())
        return false;

    QWindowsService service(state.name(), manager);
    if (!service.open(SERVICE_START | SERVICE_QUERY_STATUS) || !service.start())
        return false;

    return true;
}

bool QDaemonControllerPrivate::stop()
{
    QWindowsServiceManager manager = QWindowsServiceManager::open();
    if (!manager.isValid())
        return false;

    QWindowsService service(state.name(), manager);
    if (!service.open(SERVICE_STOP | SERVICE_QUERY_STATUS) || !service.stop())
        return false;

    return true;
}

bool QDaemonControllerPrivate::install()
{
    QWindowsServiceManager manager = QWindowsServiceManager::open();
    if (!manager.isValid())
        return false;

    QWindowsService service(state.name(), manager);
    service.setDescription(state.description());

    QStringList executable = QStringList() << state.path() << state.arguments();
    service.setExecutable(executable.join(QLatin1Char(' ')));

    if (!service.create())
        return false;

    if (state.flags().testFlag(UpdatePathFlag))  {
        QWindowsSystemPath path;
        if (!path.addEntry(state.path()))
            return false;
    }

    return true;
}

bool QDaemonControllerPrivate::uninstall()
{
    QWindowsServiceManager manager = QWindowsServiceManager::open();
    if (!manager.isValid())
        return false;

    QWindowsService service(state.name(), manager);
    if (!service.open(DELETE) || !service.remove())
        return false;

    if (state.flags().testFlag(UpdatePathFlag))  {
        QWindowsSystemPath path;
        if (!path.removeEntry(state.path()))
            return false;
    }

    return true;
}

QtDaemon::DaemonStatus QDaemonControllerPrivate::status()
{
    QWindowsServiceManager manager = QWindowsServiceManager::open();
    if (!manager.isValid())
        return NotRunningStatus;

    QWindowsService service(state.name(), manager);
    if (!service.open(SERVICE_QUERY_STATUS))
        return NotRunningStatus;

    switch (service.status())
    {
    case SERVICE_RUNNING:
        return RunningStatus;
    case SERVICE_STOPPED:
    default:
        return NotRunningStatus;
    }
}

QT_DAEMON_END_NAMESPACE
