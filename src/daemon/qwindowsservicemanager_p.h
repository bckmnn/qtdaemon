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

#ifndef QWINDOWSSERVICEMANAGER_P_H
#define QWINDOWSSERVICEMANAGER_P_H

#include "QtDaemon/qdaemon_global.h"

#include <QtCore/qthread.h>
#include <QtCore/qsemaphore.h>

#include <Windows.h>

QT_DAEMON_BEGIN_NAMESPACE

class QWindowsService;
class Q_DAEMON_LOCAL QWindowsServiceManager
{
    friend class QWindowsService;

private:
    QWindowsServiceManager(SC_HANDLE manager);

public:
    QWindowsServiceManager(const QWindowsServiceManager &);
    ~QWindowsServiceManager();

    static QWindowsServiceManager open();
    void close();

    bool isValid() const;
    QString lastError() const;

private:
    SC_HANDLE handle;
    QString error;
};

inline QWindowsServiceManager::~QWindowsServiceManager()
{
    close();
}

inline bool QWindowsServiceManager::isValid() const
{
    return handle;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

class Q_DAEMON_LOCAL QWindowsService
{
public:
    explicit QWindowsService(QWindowsServiceManager & sm);
    QWindowsService(const QString & name, QWindowsServiceManager & sm);
    ~QWindowsService();

    void setName(const QString & name);
    QString name() const;

    void setDescription(const QString & description);
    QString description() const;

    void setExecutable(const QString & executable);
    QString executable() const;

    bool isOpen();
    bool open(int access);
    void close();

    bool create();
    bool remove();
    bool start();
    bool stop();
    DWORD status();

    QString lastError() const;

private:
    bool waitForStatus(DWORD requestedStatus);
    void setError(DWORD);
    void setError(const QString &);

private:
    QWindowsServiceManager & manager;
    SC_HANDLE handle;
    QString serviceName;
    QString serviceDescription;
    QString serviceExecutable;
    QString error;
};

inline QWindowsService::QWindowsService(const QString & name, QWindowsServiceManager & sm)
    : manager(sm), handle(Q_NULLPTR), serviceName(name)
{
}

inline QWindowsService::QWindowsService(QWindowsServiceManager & sm)
    : manager(sm), handle(Q_NULLPTR)
{
}

inline QWindowsService::~QWindowsService()
{
    close();
}

inline void QWindowsService::setName(const QString & name)
{
    serviceName = name;
}

inline QString QWindowsService::name() const
{
    return serviceName;
}

inline void QWindowsService::setDescription(const QString & description)
{
    serviceDescription = description;
}

inline QString QWindowsService::description() const
{
    return serviceDescription;
}

inline void QWindowsService::setExecutable(const QString & executable)
{
    serviceExecutable = executable;
}

inline QString QWindowsService::executable() const
{
    return serviceExecutable;
}

inline bool QWindowsService::isOpen()
{
    return handle;
}

inline QString QWindowsService::lastError() const
{
    return error;
}

inline void QWindowsService::setError(const QString & errorText)
{
    error = errorText;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

class Q_DAEMON_LOCAL QWindowsSystemPath
{
public:
    QWindowsSystemPath();
    ~QWindowsSystemPath();

    bool addEntry(const QString &);
    bool removeEntry(const QString & dir);

private:
    bool load();
    bool save();

private:
    HKEY registryKey;
    QStringList entries;

    QString error;
    static LPCTSTR pathRegistryKey;
};

QT_DAEMON_END_NAMESPACE

#endif // QWINDOWSSERVICEMANAGER_P_H
