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

#include <QCoreApplication>

QT_DAEMON_BEGIN_NAMESPACE

QDaemon::QDaemon(const QString & name)
    : QObject(qApp), d_ptr(new QDaemonPrivate(name, this))
{
    Q_ASSERT_X(qApp, Q_FUNC_INFO, "You must create the application object first.");

    d_ptr->state.load();

    QObject::connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(_q_stop()));
    QMetaObject::invokeMethod(this, "_q_start", Qt::QueuedConnection);     // Queue the daemon start up for when the application is ready
}

QString QDaemon::directoryPath() const
{
    Q_D(const QDaemon);
    return d->state.directory();
}

QString QDaemon::filePath() const
{
    Q_D(const QDaemon);
    return d->state.path();
}

QT_DAEMON_END_NAMESPACE

#include "moc_qdaemon.cpp"
