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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDaemon API. It exists only
// as an implementation detail. This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDAEMONCONTROLLER_P_H
#define QDAEMONCONTROLLER_P_H

/*!
    \internal
    \class QDaemonController

    Realizes the daemon controller functionality. Platform specific options can be set and retrieved with setOption() and option().

    \note The following methods don't have a standard implementation and thus should be provided in a separate platform specific source file:
    QDaemonControllerPrivate::start()
    QDaemonControllerPrivate::stop()
    QDaemonControllerPrivate::install()
    QDaemonControllerPrivate::uninstall()
    QtDaemon::DaemonStatus QDaemonControllerPrivate::status()

    \fn QDaemonController::QDaemonController(const QString & name, const QString & description, QDaemonController * q);

    Creates the private object for the daemon controller public object pointed by \a q with name and description given by \a name \a description respectively.
*/

#include "QtDaemon/qdaemon_global.h"
#include "QtDaemon/private/qdaemonstate_p.h"

QT_DAEMON_BEGIN_NAMESPACE

class QDaemonController;
class Q_DAEMON_EXPORT QDaemonControllerPrivate
{
    Q_DECLARE_PUBLIC(QDaemonController)

public:
    QDaemonControllerPrivate(const QString &, QDaemonController *);

    bool start();
    bool stop();
    bool install();
    bool uninstall();

    QtDaemon::DaemonStatus status();

private:
    QDaemonController * q_ptr;
    QDaemonState state;
};

QT_DAEMON_END_NAMESPACE

#endif // QDAEMONCONTROLLER_P_H
