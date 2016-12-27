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

#include "QtDaemon/private/qdaemonwindowsctrldispatcher_p.h"

#include <QtCore/qcoreapplication.h>

QT_DAEMON_BEGIN_NAMESPACE

static const qint32 serviceWaitHint = 1000;			// Just a service control sugar (1 second)

QDaemonWindowsCtrlDispatcher::QDaemonWindowsCtrlDispatcher()
    : QThread(Q_NULLPTR), serviceStartLock(0), serviceQuitLock(0), serviceStatusHandle(NULL), ok(true)
{
    Q_ASSERT(!instance);
    instance = this;
    ::memset(dispatchTable, 0, 2 * sizeof(SERVICE_TABLE_ENTRY));
}

void QDaemonWindowsCtrlDispatcher::run()
{
    dispatchTable[0].lpServiceName = reinterpret_cast<LPTSTR>(const_cast<ushort *>(serviceName.utf16()));	// Type safety be damned ...!
    dispatchTable[0].lpServiceProc = ServiceMain;

    // Just call the control dispatcher and hang
    if (!StartServiceCtrlDispatcher(dispatchTable))  {
        ok = false;

        qDaemonLog(QStringLiteral("Couldn't run the service control dispatcher (Error code %1).").arg(GetLastError()), QDaemonLog::ErrorEntry);
        serviceStartLock.release();		// Free the main thread
    }
}

bool QDaemonWindowsCtrlDispatcher::startService(const QString & service)
{
    serviceName = service;

    start();						// Start the control thread
    serviceStartLock.acquire();		// Wait until the service has started

    return ok;
}

void QDaemonWindowsCtrlDispatcher::stopService()
{
    if (serviceQuitLock.available() > 0)
        return;		// Already requested a stop

    serviceQuitLock.release();
    wait();
}

QDaemonWindowsCtrlDispatcher * QDaemonWindowsCtrlDispatcher::instance = Q_NULLPTR;

// --------------------------------------------------------------------------------------------------------------------------------------------------- //
// --- WinAPI callbacks ------------------------------------------------------------------------------------------------------------------------------ //
// --------------------------------------------------------------------------------------------------------------------------------------------------- //

VOID WINAPI ServiceMain(DWORD, LPTSTR *)
{
    QDaemonWindowsCtrlDispatcher * const ctrl = QDaemonWindowsCtrlDispatcher::instance;

    // Just register the handler and that's all
    ctrl->serviceStatusHandle = RegisterServiceCtrlHandlerEx(ctrl->dispatchTable[0].lpServiceName, ServiceControlHandler, ctrl);
    if (!ctrl->serviceStatusHandle)  {
        qDaemonLog(QStringLiteral("Couldn't register the service control handler (Error code %1).").arg(GetLastError()), QDaemonLog::ErrorEntry);
        ctrl->serviceStartLock.release();
        return;
    }

    // Fill up initial values
    ctrl->serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ctrl->serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
    ctrl->serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

    // Report initial status to the SCM
    ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, serviceWaitHint);

    // Nothing to initialize, so report running
    ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);

    ctrl->serviceStartLock.release();		// Free up the main thread
    ctrl->serviceQuitLock.acquire();		// Signalled (released) when we need to quit. Block for now

    ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

DWORD WINAPI ServiceControlHandler(DWORD control, DWORD, LPVOID, LPVOID context)
{
    switch (control)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, serviceWaitHint);
        QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
        return NO_ERROR;
    case SERVICE_CONTROL_INTERROGATE:
        {
            QDaemonWindowsCtrlDispatcher * const ctrl = reinterpret_cast<QDaemonWindowsCtrlDispatcher *>(context);
            ReportServiceStatus(ctrl->serviceStatus.dwCurrentState, ctrl->serviceStatus.dwWin32ExitCode, ctrl->serviceStatus.dwWaitHint);
        }
        return NO_ERROR;
    case SERVICE_CONTROL_CONTINUE:
    case SERVICE_CONTROL_PAUSE:
    default:
        return ERROR_CALL_NOT_IMPLEMENTED;
    }
}

VOID WINAPI ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    QDaemonWindowsCtrlDispatcher * const ctrl = QDaemonWindowsCtrlDispatcher::instance;

    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.
    ctrl->serviceStatus.dwCurrentState = dwCurrentState;
    ctrl->serviceStatus.dwWin32ExitCode = dwWin32ExitCode;
    ctrl->serviceStatus.dwWaitHint = dwWaitHint;
    ctrl->serviceStatus.dwControlsAccepted = dwCurrentState == SERVICE_START_PENDING ? 0 : SERVICE_ACCEPT_STOP;
    ctrl->serviceStatus.dwCheckPoint = dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED ? 0 : dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus(ctrl->serviceStatusHandle, &ctrl->serviceStatus);
}

QT_DAEMON_END_NAMESPACE
