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

#include "QtDaemon/qdaemoncontroller.h"
#include "QtDaemon/private/qdaemoncontroller_p.h"

#include <QtCore/qcoreapplication.h>

QT_DAEMON_BEGIN_NAMESPACE

/*!
    Starts the daemon
*/
QDaemonController::QDaemonController(const QString & name, QObject * parent)
    : QObject(parent), d_ptr(new QDaemonControllerPrivate(name, this))
{
    Q_ASSERT_X(qApp, Q_FUNC_INFO, "You must create the application object first.");

    if (!d_ptr->state.load())
        d_ptr->lastError = QT_DAEMON_TRANSLATE("Couldn't load the daemon configuration.");
}

/*!
    Starts the daemon with the command line arguments that were specified when it was installed.
*/
bool QDaemonController::start()
{
    Q_D(QDaemonController);
    if (!d->state.isLoaded())
        return false;

    return d->start();
}

/*!
    Starts the daemon with command line arguments specified by \a arguments.
*/
bool QDaemonController::start(const QStringList & arguments)
{
    Q_D(QDaemonController);
    if (!d->state.isLoaded())
        return false;

    QStringList oldArguments = d->state.arguments();

    d->state.setArguments(arguments);
    if (!d->state.save())  {
        d->lastError = QT_DAEMON_TRANSLATE("Couldn't save the daemon configuration.");
        return false;
    }

    bool ok = d->start();

    d->state.setArguments(oldArguments);
    if (!d->state.save())  {
        d->lastError = QT_DAEMON_TRANSLATE("Couldn't save the daemon configuration.");
        return false;
    }

    return ok;
}

/*!
    Stops the daemon.
*/
bool QDaemonController::stop()
{
    Q_D(QDaemonController);
    if (!d->state.isLoaded())
        return false;

    return d->stop();
}

/*!
    Install the executable located at \a path as a daemon with command line arguments
    given by \a arguments.
*/
bool QDaemonController::install(const QString & path, const QStringList & arguments)
{
    Q_D(QDaemonController);
    if (d->state.isLoaded())  {
        d->lastError = QT_DAEMON_TRANSLATE("The daemon is already installed.");
        return false;
    }

    if (!d->state.initialize(path, arguments))  {
        d->lastError = QT_DAEMON_TRANSLATE("Couldn't initialize the daemon configuration.");
        return false;
    }

    if (!d->state.save())  {
        d->lastError = QT_DAEMON_TRANSLATE("Couldn't save the daemon configuration.");
        return false;
    }

    if (!d->install())  {
        d->state.clear();
        return false;
    }

    return true;
}

/*!
    Uninstall the daemon.
*/
bool QDaemonController::uninstall()
{
    Q_D(QDaemonController);
    if (!d->state.isLoaded())  {
        d->lastError = QT_DAEMON_TRANSLATE("The daemon is not installed.");
        return false;
    }

    bool ok = d->uninstall();
    d->state.clear();
    return ok;
}

/*!
    Returns the daemon status.
*/
QtDaemon::DaemonStatus QDaemonController::status()
{
    Q_D(QDaemonController);
    return d->status();
}

QString QDaemonController::lastError() const
{
    Q_D(const QDaemonController);
    return d->lastError;
}

/*!
    Daemon Description
*/
void QDaemonController::setDescription(const QString & description)
{
    Q_D(QDaemonController);
    d->state.setDescription(description);
}

QString QDaemonController::description() const
{
    Q_D(const QDaemonController);
    return d->state.description();
}

/*!
    Daemon flags
*/
void QDaemonController::setFlags(const QDaemonFlags & flags)
{
    Q_D(QDaemonController);
    d->state.setFlags(flags);
}

QDaemonFlags QDaemonController::flags() const
{
    Q_D(const QDaemonController);
    return d->state.flags();
}

/*!
    initScriptPrefix property
*/
void QDaemonController::setInitScriptPrefix(const QString & prefix)
{
    Q_D(QDaemonController);
    d->state.setInitDPrefix(prefix);
}

QString QDaemonController::initScriptPrefix() const
{
    Q_D(const QDaemonController);
    return d->state.initdPrefix();
}

/*!
    dbusConfigurationPrefix property
*/
void QDaemonController::setDBusConfigurationPrefix(const QString & prefix)
{
    Q_D(QDaemonController);
    d->state.setDBusPrefix(prefix);
}

QString QDaemonController::dbusConfigurationPrefix() const
{
    Q_D(const QDaemonController);
    return d->state.dbusPrefix();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

/*!
*/
QDaemonControllerPrivate::QDaemonControllerPrivate(const QString & name, QDaemonController * q)
    : q_ptr(q), state(name)
{
}

QT_DAEMON_END_NAMESPACE
