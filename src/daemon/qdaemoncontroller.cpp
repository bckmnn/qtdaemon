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

#include "qdaemoncontroller.h"
#include "qdaemoncontroller_p.h"
#include "qdaemonlog_p.h"

#include <QCoreApplication>

QT_DAEMON_BEGIN_NAMESPACE

/*!
    Starts the daemon
*/
QDaemonController::QDaemonController(const QString & name)
    : QObject(qApp), d_ptr(new QDaemonControllerPrivate(name, this))
{
}

/*!
    Starts the daemon with the command line arguments that were specified when it was installed.
*/
bool QDaemonController::start()
{
    Q_D(QDaemonController);
    return d->start();
}

/*!
    Starts the daemon with command line arguments specified by \a arguments.
*/
bool QDaemonController::start(const QStringList & arguments)
{
    Q_D(QDaemonController);
    QDaemonState & state = d->state;

    QStringList oldArguments = state.arguments();
    state.setArguments(arguments);

    bool status = d->start();

    state.setArguments(oldArguments);
    return status;
}

/*!
    Stops the daemon.
*/
bool QDaemonController::stop()
{
    Q_D(QDaemonController);
    return d->stop();
}

/*!
    Install the executable located at \a path as a daemon with command line arguments
    given by \a arguments.
*/
bool QDaemonController::install(const QString & path, const QStringList & arguments)
{
    Q_D(QDaemonController);
    return d->install(path, arguments);
}

/*!
    Uninstall the daemon.
*/
bool QDaemonController::uninstall()
{
    Q_D(QDaemonController);
    return d->uninstall();
}

/*!
    Returns the daemon status.
*/
QtDaemon::DaemonStatus QDaemonController::status()
{
    Q_D(QDaemonController);
    return d->status();
}

/*!
    Daemon Description
*/
void QDaemonController::setDescription(const QString & description)
{
    d_func()->state.setDescription(description);
}

QString QDaemonController::description() const
{
    return d_func()->state.description();
}

/*!
    Daemon flags
*/
void QDaemonController::setFlags(const QDaemonFlags & flags)
{
    d_func()->state.setFlags(flags);
}

QDaemonFlags QDaemonController::flags() const
{
    return d_func()->state.flags();
}

QT_DAEMON_END_NAMESPACE
