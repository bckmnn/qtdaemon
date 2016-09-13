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

#ifndef QDAEMONCONTROLLER_H
#define QDAEMONCONTROLLER_H

#include "QtDaemon/qdaemon-global.h"

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QCommandLineParser;

namespace QtDaemon
{
    enum DaemonStatus  {
        DaemonRunning,
        DaemonNotRunning
    };
}

class QDaemonControllerPrivate;
class Q_DAEMON_EXPORT QDaemonController : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDaemonController)
    Q_DISABLE_COPY(QDaemonController)

public:
    enum ControllerOption {
        // Lnux only
        InitdPrefixOption,
        DbusPrefixOption,
        // Windows only
        UpdatePathOption,
        // OSX only
        AgentOption,
        UserOption
    };

public:
    explicit QDaemonController(QObject * = 0);

    bool start();
    bool stop();
    bool install();
    bool uninstall();

    void setOption(ControllerOption, bool = true);
    void setOption(ControllerOption, const QString &);
    void setOption(ControllerOption, const QVariant &);
    QVariant option(ControllerOption) const;

    QtDaemon::DaemonStatus status();

protected:
//    virtual bool processCommandLine(const QCommandLineParser &);
    virtual QString helpText(const QCommandLineParser &) const;

private:
    QDaemonControllerPrivate * d_ptr;
};

QT_END_NAMESPACE

#endif // QDAEMONCONTROLLER_H
