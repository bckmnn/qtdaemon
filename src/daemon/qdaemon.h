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

#ifndef QDAEMON_H
#define QDAEMON_H

#include "QtDaemon/qdaemon_global.h"

#include <QtCore/qobject.h>

QT_DAEMON_BEGIN_NAMESPACE

class QDaemonPrivate;
class Q_DAEMON_EXPORT QDaemon : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDaemon)
    Q_DISABLE_COPY(QDaemon)

    Q_PRIVATE_SLOT(d_func(), void _q_start())
    Q_PRIVATE_SLOT(d_func(), void _q_stop())

public:
    explicit QDaemon(QObject * = nullptr);
    explicit QDaemon(DaemonScope, QObject * = nullptr);

    bool isValid() const;

    QString directoryPath() const;
    QString filePath() const;

Q_SIGNALS:
    void ready(const QStringList &);
    void error(const QString &);

private:
    QDaemonPrivate * d_ptr;
};

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

inline QDaemon::QDaemon(QObject * parent)
    : QDaemon(SystemScope, parent)
{
}

QT_DAEMON_END_NAMESPACE

#endif // QDAEMON_H
