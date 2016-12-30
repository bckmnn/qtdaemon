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

#include "QtDaemon/qdaemon.h"
#include "QtDaemon/private/qdaemon_p.h"

#include <QtCore/qcoreapplication.h>

QT_DAEMON_BEGIN_NAMESPACE

void QDaemonPrivate::_q_start()
{
    if (!ctrl.startService(state.name()))  {
        qApp->quit();
        return;
    }

    // Just emit the ready signal, nothing more to do here
    QStringList arguments;
    arguments << state.path() << state.arguments();

    QMetaObject::invokeMethod(q_func(), "ready", Qt::QueuedConnection, Q_ARG(const QStringList &, arguments));
}

void QDaemonPrivate::_q_stop()
{
    ctrl.stopService();
}

QT_DAEMON_END_NAMESPACE
