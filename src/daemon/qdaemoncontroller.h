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

#include "QtDaemon/qdaemon_global.h"

#include <QtCore/qobject.h>

QT_DAEMON_BEGIN_NAMESPACE

class QDaemonControllerPrivate;
class Q_DAEMON_EXPORT QDaemonController : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDaemonController)
    Q_DISABLE_COPY(QDaemonController)

public:
    // TODO: Finish up the properties
    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(QDaemonFlags flags READ flags WRITE setFlags)
    Q_PROPERTY(QString initScriptPrefix READ initScriptPrefix WRITE setInitScriptPrefix)
    Q_PROPERTY(QString dbusConfigurationPrefix READ dbusConfigurationPrefix WRITE setDBusConfigurationPrefix)

    explicit QDaemonController(const QString &, DaemonScope scope, QObject * = Q_NULLPTR);

    bool start();
    bool start(const QStringList &);
    bool stop();
    bool install(const QString &, const QStringList & = QStringList());
    bool uninstall();

    QtDaemon::DaemonStatus status();

    QString lastError() const;

    void setDescription(const QString &);
    QString description() const;

    void setFlags(const QDaemonFlags &);
    QDaemonFlags flags() const;

    // Lunux only:
    void setInitScriptPrefix(const QString &);
    QString initScriptPrefix() const;

    void setDBusConfigurationPrefix(const QString &);
    QString dbusConfigurationPrefix() const;

private:
    QDaemonControllerPrivate * d_ptr;
};

QT_DAEMON_END_NAMESPACE

#endif // QDAEMONCONTROLLER_H
