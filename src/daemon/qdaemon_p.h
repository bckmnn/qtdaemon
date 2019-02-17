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

#ifndef QDAEMON_P_H
#define QDAEMON_P_H

#include "QtDaemon/qdaemon_global.h"
#include "QtDaemon/private/qdaemonstate_p.h"

#if defined(Q_OS_LINUX)
#include "QtDaemon/private/qdaemondbusinterface_p.h"
#elif defined(Q_OS_WIN)
#include "QtDaemon/private/qdaemonwindowsctrldispatcher_p.h"
#endif

QT_DAEMON_BEGIN_NAMESPACE

class QDaemon;
class Q_DAEMON_LOCAL QDaemonPrivate
{
    Q_DECLARE_PUBLIC(QDaemon)

public:
    explicit QDaemonPrivate(const QString &, DaemonScope scope, QDaemon *);

    void _q_start();
    void _q_stop();

private:
    QDaemon * q_ptr;
    QDaemonState state;
#if defined(Q_OS_LINUX)
    QDaemonDBusInterfaceProvider dbus;
#elif defined(Q_OS_WIN)
    QDaemonWindowsCtrlDispatcher ctrl;
#endif
};

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

inline QDaemonPrivate::QDaemonPrivate(const QString & name, DaemonScope scope, QDaemon * q)
    : q_ptr(q), state(name, scope)
{
}

QT_DAEMON_END_NAMESPACE

#endif // QDAEMON_P_H
