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
#include "private/qdaemoncontroller_p.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qcommandlineparser.h>

QT_BEGIN_NAMESPACE

using namespace QtDaemon;

QDaemonController::QDaemonController(QCoreApplication & app)
    : QObject(&app), d_ptr(new QDaemonControllerPrivate(this))
{
}

bool QDaemonController::start()
{
    Q_D(QDaemonController);
    return d->start();
}

bool QDaemonController::stop()
{
    Q_D(QDaemonController);
    return d->stop();
}

bool QDaemonController::install()
{
    Q_D(QDaemonController);
    return d->install();
}

bool QDaemonController::uninstall()
{
    Q_D(QDaemonController);
    return d->uninstall();
}

QtDaemon::DaemonStatus QDaemonController::status()
{
    Q_D(QDaemonController);
    return d->status();
}

void QDaemonController::setOption(ControllerOption opt, const QVariant & value)
{
    Q_D(QDaemonController);
    d->options.insert(opt, value);
}

QVariant QDaemonController::option(ControllerOption opt) const
{
    Q_D(const QDaemonController);
    return d->options.value(opt);
}

QT_END_NAMESPACE
