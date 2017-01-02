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

#ifndef QDAEMONWINDOWSCTRLDISPATCHER_P_H
#define QDAEMONWINDOWSCTRLDISPATCHER_P_H

#include "QtDaemon/qdaemon_global.h"

#include <QtCore/qthread.h>
#include <QtCore/qsemaphore.h>

#include <Windows.h>

QT_DAEMON_BEGIN_NAMESPACE

VOID WINAPI ServiceMain(DWORD, LPTSTR *);
DWORD WINAPI ServiceControlHandler(DWORD, DWORD, LPVOID, LPVOID);
VOID WINAPI ReportServiceStatus(DWORD, DWORD, DWORD);

class Q_DAEMON_LOCAL QDaemonWindowsCtrlDispatcher : public QThread
{
    friend VOID WINAPI ServiceMain(DWORD, LPTSTR *);
    friend DWORD WINAPI ServiceControlHandler(DWORD, DWORD, LPVOID, LPVOID);
    friend VOID WINAPI ReportServiceStatus(DWORD, DWORD, DWORD);

public:
    QDaemonWindowsCtrlDispatcher();

    void run() Q_DECL_OVERRIDE;

    bool startService(const QString &);
    void stopService();

private:
    QString serviceName;
    QSemaphore serviceStartLock;
    QSemaphore serviceQuitLock;
    SERVICE_TABLE_ENTRY dispatchTable[2];
    SERVICE_STATUS_HANDLE serviceStatusHandle;
    SERVICE_STATUS serviceStatus;
    bool ok;
    QString lastError;

private:
    static QDaemonWindowsCtrlDispatcher * instance;
};

QT_DAEMON_END_NAMESPACE

#endif // QDAEMONWINDOWSCTRLDISPATCHER_P_H
