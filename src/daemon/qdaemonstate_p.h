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

#ifndef QDAEMONSTATE_P_H
#define QDAEMONSTATE_P_H

#include "qdaemon_global.h"

#include <QHash>
#include <QVariant>

QT_DAEMON_BEGIN_NAMESPACE

class Q_DAEMON_LOCAL QDaemonState
{
public:
    QDaemonState(const QString &);

    // Init and (de)serialization
    bool initialize(const QString &, const QStringList &);
    bool load();
    bool isLoaded() const;
    bool save() const;
    void clear();

    // Common
    void setPath(const QString &);
    void setDescription(const QString &);
    void setArguments(const QStringList &);
    void setFlags(const QDaemonFlags &);
    // Linux
    void setInitDPrefix(const QString &);
    void setDBusPrefix(const QString &);
    void setDBusTimeout(qint32);

    // Common
    QString name() const;
    QString path() const;
    QString executable() const;
    QString directory() const;
    QString description() const;
    QString service() const;
    QStringList arguments() const;
    QDaemonFlags flags() const;
    // Linux
    QString initdPrefix() const;
    QString dbusPrefix() const;
    QString initdScriptPath() const;
    QString dbusConfigPath() const;
    qint32 dbusTimeout() const;

private:
    bool loaded;
    struct Data
    {
        Data(const QString &);

        QString name, path, executable, directory, description, service, initdPrefix, dbusPrefix;
        QStringList arguments;
        qint32 dbusTimeout;
        QDaemonFlags flags;
    } d;
};

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

inline bool QDaemonState::isLoaded() const
{
    return loaded;
}

inline void QDaemonState::setDescription(const QString & description)
{
    d.description = description;
}

inline void QDaemonState::setArguments(const QStringList & arguments)
{
    d.arguments = arguments;
}

inline void QDaemonState::setFlags(const QDaemonFlags & flags)
{
    d.flags = flags;
}

inline void QDaemonState::setInitDPrefix(const QString & prefix)
{
    d.initdPrefix = prefix;
}

inline void QDaemonState::setDBusPrefix(const QString & prefix)
{
    d.dbusPrefix = prefix;
}

inline void QDaemonState::setDBusTimeout(qint32 timeout)
{
    d.dbusTimeout = timeout;
}

inline QString QDaemonState::name() const
{
    return d.name;
}

inline QString QDaemonState::path() const
{
    return d.path;
}

inline QString QDaemonState::executable() const
{
    return d.executable;
}

inline QString QDaemonState::directory() const
{
    return d.directory;
}

inline QString QDaemonState::description() const
{
    return d.description;
}

inline QString QDaemonState::service() const
{
    return d.service;
}

inline QStringList QDaemonState::arguments() const
{
    return d.arguments;
}

inline QDaemonFlags QDaemonState::flags() const
{
    return d.flags;
}

inline QString QDaemonState::initdPrefix() const
{
    return d.initdPrefix;
}

inline QString QDaemonState::dbusPrefix() const
{
    return d.dbusPrefix;
}

inline qint32 QDaemonState::dbusTimeout() const
{
    return d.dbusTimeout;
}

QT_DAEMON_END_NAMESPACE

Q_DECLARE_METATYPE(QtDaemon::QDaemonFlags)

#endif // QDAEMONSTATE_P_H
