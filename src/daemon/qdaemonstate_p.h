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

#include "QtDaemon/qdaemon_global.h"

#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>
#include <QtCore/qsettings.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qvariant.h>

#include <functional>

QT_DAEMON_BEGIN_NAMESPACE

class QDaemonState;

template <typename Type>
class QDaemonStateField
{
public:
    typedef std::function<void(const Type &)> Observer;

    QDaemonStateField(const QString &, const QDaemonState &);
    QDaemonStateField(const QString &, const Type &, const QDaemonState &);

    operator Type () const;
    QDaemonStateField<Type> & operator = (const Type &);

    void observe(const Observer &);

    void save();
    void invalidate();

private:
    bool needsLoad() const;

    const QDaemonState & state;
    const QString key;
    mutable Type value;
    mutable bool loaded;
    bool dirty;
    Observer callback;
};

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

class Q_DAEMON_LOCAL QDaemonState
{
public:
    QDaemonState();

    // Init and (de)serialization
    bool create(const QString &, const QStringList &);
    bool isKnown() const;
    void clear();
    void invalidate();

    // Common
    void setPath(const QString &);
    void setDescription(const QString &);
    void setArguments(const QStringList &);
    void setFlags(const DaemonFlags &);
    void setScope(DaemonScope);

    // Linux
    void setInitDPrefix(const QString &);
    void setDBusPrefix(const QString &);
    void setDBusTimeout(qint32);
    // macOS

    // Common
    QString name() const;
    QString path() const;
    QString executable() const;
    QString directory() const;
    QString description() const;
    QString service() const;
    QStringList arguments() const;
    DaemonFlags flags() const;
    DaemonScope scope() const;

    // Linux
    QString initdPrefix() const;
    QString dbusPrefix() const;
    QString initdScriptPath() const;
    QString dbusConfigPath() const;
    qint32 dbusTimeout() const;
    // macOS
    QString plistPrefix() const;
    QString plistPath() const;

private:
    QString generateServiceName() const;

    struct Data
    {
        Data(const QDaemonState &);

        QDaemonStateField<QString> path, description;
#ifdef Q_OS_LINUX
        QDaemonStateField<QString> initdPrefix, dbusPrefix;
        QDaemonStateField<qint32> dbusTimeout;
#endif
        QDaemonStateField<QStringList> arguments;
        QDaemonStateField<DaemonFlags> flags;
        QString executable, directory, service, name;

        DaemonScope scope;
    } d;
};

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

inline void QDaemonState::setDescription(const QString & description)
{
    d.description = description;
}

inline void QDaemonState::setArguments(const QStringList & arguments)
{
    d.arguments = arguments;
}

inline void QDaemonState::setFlags(const DaemonFlags & flags)
{
    d.flags = flags;
}

inline void QDaemonState::setScope(DaemonScope scope)
{
    if (d.scope == scope)
        return;

    d.scope = scope;
    invalidate();

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
    return QCoreApplication::applicationName();
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

inline DaemonFlags QDaemonState::flags() const
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

inline DaemonScope QDaemonState::scope() const
{
    return d.scope;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

template <typename Type>
inline QDaemonStateField<Type>::QDaemonStateField(const QString & key, const QDaemonState & state)
    : QDaemonStateField(key, Type(), state)
{
}

template <typename Type>
inline QDaemonStateField<Type>::QDaemonStateField(const QString & settingsKey, const Type & defaultValue, const QDaemonState & daemonState)
    : state(daemonState), key(settingsKey), value(defaultValue), loaded(false), dirty(false), callback(nullptr)
{
}

template <typename Type>
inline QDaemonStateField<Type>::operator Type () const
{
    // Load from the settings if it was neither loaded nor overwritten
    if (needsLoad())  {
        QSettings::Scope scope = (state.scope() == QtDaemon::UserScope ? QSettings::UserScope : QSettings::SystemScope);
        QSettings settings(scope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
        settings.setFallbacksEnabled(false);

        if (!settings.contains(key))
            return value;

        QVariant data = settings.value(key);
        value = data.value<Type>();
        loaded = true;
    }

    return value;
}

template <typename Type>
inline QDaemonStateField<Type> & QDaemonStateField<Type>::operator = (const Type & newValue)
{
    if (value == newValue)
        return *this;

    value = newValue;
    dirty = true;
    return *this;
}

template <typename Type>
inline bool QDaemonStateField<Type>::needsLoad() const
{
    return !loaded && !dirty;
}

template <typename Type>
inline void QDaemonStateField<Type>::save()
{
    if (!dirty)
        return;

    QSettings::Scope scope = (state.scope() == QtDaemon::UserScope ? QSettings::UserScope : QSettings::SystemScope);
    QSettings settings(scope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setFallbacksEnabled(false);

    settings.setValue(key, QVariant::fromValue<Type>(value));

    loaded = true;
    dirty = false;
}

template <typename Type>
inline void QDaemonStateField<Type>::invalidate()
{
    loaded = false;
}

template <typename Type>
inline void QDaemonStateField<Type>::observe(const Observer & observer)
{
    callback = observer;
}

QT_DAEMON_END_NAMESPACE

Q_DECLARE_METATYPE(QtDaemon::DaemonFlags)

#endif // QDAEMONSTATE_P_H
