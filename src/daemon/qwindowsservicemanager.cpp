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

#include "QtDaemon/private/qwindowsservicemanager_p.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qdir.h>

QT_DAEMON_BEGIN_NAMESPACE

static SC_HANDLE serviceManagerHandle = NULL;
static qint32 serviceManagerHandleReferenceCount = 0;

QWindowsServiceManager::QWindowsServiceManager(SC_HANDLE manager)
    : handle(manager)
{
    if (handle)
        serviceManagerHandleReferenceCount++;
}

QWindowsServiceManager::QWindowsServiceManager(const QWindowsServiceManager & other)
{
    handle = other.handle;
    error = other.error;

    if (handle)
        serviceManagerHandleReferenceCount++;
}

QWindowsServiceManager QWindowsServiceManager::open()
{
    // Try to open the service manager (if not already)
    if (!serviceManagerHandle)
        serviceManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    // Set up the object
    QWindowsServiceManager manager(serviceManagerHandle);
    if (!serviceManagerHandle)
        manager.error = QT_DAEMON_TRANSLATE("Couldn't open a handle to the service manager (Error code %1)").arg(GetLastError());

    return manager;
}

void QWindowsServiceManager::close()
{
    serviceManagerHandleReferenceCount--;
    if (serviceManagerHandleReferenceCount > 0)
        return;

    if (handle && !CloseServiceHandle(handle))
        error = QT_DAEMON_TRANSLATE("Error while closing the service manager (Error code %1).").arg(GetLastError());

    serviceManagerHandle = handle = NULL;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

static const qint32 serviceNotifyTimeout = 30000;			// Up to 30 seconds

bool QWindowsService::open(int access)
{
    LPCTSTR lpName = reinterpret_cast<LPCTSTR>(serviceName.utf16());

    // Open the service
    handle = OpenService(manager.handle, lpName, access);
    if (!handle)
        setError(GetLastError());

    return handle;
}

void QWindowsService::close()
{
    if (handle && !CloseServiceHandle(handle))
        error = QT_DAEMON_TRANSLATE("Error while closing the service handle (Error code %1).").arg(GetLastError());

    handle = NULL;
}

bool QWindowsService::create()
{
    LPCTSTR lpName = reinterpret_cast<LPCTSTR>(serviceName.utf16());
    LPCTSTR lpPath = reinterpret_cast<LPCTSTR>(serviceExecutable.utf16());

    // Create the service
    handle = CreateService(manager.handle, lpName, lpName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, lpPath, NULL, NULL, NULL, NULL, NULL);
    if (!handle)  {
        setError(GetLastError());
        return false;
    }

    // Set the description if provided
    if (!serviceDescription.isEmpty())  {
        LPTSTR lpDescription = reinterpret_cast<LPTSTR>(const_cast<ushort *>(serviceDescription.utf16()));		// MS doesn't care about constness

        SERVICE_DESCRIPTION serviceDescription;
        serviceDescription.lpDescription = lpDescription;
        if (!ChangeServiceConfig2(handle, SERVICE_CONFIG_DESCRIPTION, &serviceDescription))
            setError(GetLastError());
    }

    return true;
}

bool QWindowsService::remove()
{
    if (!handle)  {
        qWarning("Service must be opened before calling remove().");
        return false;
    }

    // Delete the service
    if (!DeleteService(handle))
        setError(GetLastError());

    return true;
}

bool QWindowsService::start()
{
    if (!handle)  {
        qWarning("Service must be opened before calling start().");
        return false;
    }

    // Start the service
    if (!StartService(handle, 0, NULL))  {
        setError(GetLastError());
        return false;
    }

    return waitForStatus(SERVICE_RUNNING);
}

bool QWindowsService::stop()
{
    if (!handle)  {
        qWarning("Service must be opened before calling stop().");
        return false;
    }

    // Stop the service
    SERVICE_STATUS serviceStatus;
    if (!ControlService(handle, SERVICE_CONTROL_STOP, &serviceStatus))  {
        setError(GetLastError());
        return false;
    }

    return waitForStatus(SERVICE_STOPPED);
}

DWORD QWindowsService::status()
{
    if (!handle)  {
        qWarning("Service must be opened before calling status().");
        return 0;
    }

    DWORD bytesNeeded;
    SERVICE_STATUS_PROCESS serviceStatus;
    if (!QueryServiceStatusEx(handle, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&serviceStatus), sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded))  {
        setError(GetLastError());
        return 0;
    }

    return serviceStatus.dwCurrentState;
}

bool QWindowsService::waitForStatus(DWORD requestedStatus)
{
    const unsigned long pollInterval = serviceNotifyTimeout / 20;		// A twentieth of the allowed timeout interval (in ms)

    // Do polling (Windows XP compatible)
    QElapsedTimer timeout;
    timeout.start();
    while (!timeout.hasExpired(serviceNotifyTimeout))  {
        if (status() == requestedStatus)
            return true;

        // Wait a bit before trying out
        QThread::msleep(pollInterval);
    }

    setError(QT_DAEMON_TRANSLATE("The service is not responding."));
    return false;
}

void QWindowsService::setError(DWORD code)
{
    switch (code)
    {
    case ERROR_ACCESS_DENIED:
        error = QT_DAEMON_TRANSLATE("Couldn't access service control, access denied.");
        break;
    case ERROR_INVALID_SERVICE_CONTROL:
        error = QT_DAEMON_TRANSLATE("The requested control is not valid for this service.");
        break;
    case ERROR_SERVICE_REQUEST_TIMEOUT:
        error = QT_DAEMON_TRANSLATE("The service did not respond to the start or control request in a timely fashion.");
        break;
    case ERROR_SERVICE_NO_THREAD:
        error = QT_DAEMON_TRANSLATE("A thread could not be created for the service.");
        break;
    case ERROR_SERVICE_DATABASE_LOCKED:
        error = QT_DAEMON_TRANSLATE("The service database is locked.");
        break;
    case ERROR_SERVICE_ALREADY_RUNNING:
        error = QT_DAEMON_TRANSLATE(" An instance of the service is already running.");
        break;
    case ERROR_INVALID_SERVICE_ACCOUNT:
        error = QT_DAEMON_TRANSLATE("The account name is invalid or does not exist, or the password is invalid for the account name specified.");
        break;
    case ERROR_SERVICE_DISABLED:
        error = QT_DAEMON_TRANSLATE("The service cannot be started, either because it is disabled or because it has no enabled devices associated with it.");
        break;
    case ERROR_SERVICE_DOES_NOT_EXIST:
        error = QT_DAEMON_TRANSLATE("The specified service does not exist as an installed service.");
        break;
    case ERROR_SERVICE_CANNOT_ACCEPT_CTRL:
        error = QT_DAEMON_TRANSLATE("The service cannot accept control messages at this time.");
        break;
    case ERROR_SERVICE_NOT_ACTIVE:
        error = QT_DAEMON_TRANSLATE("The service has not been started.");
        break;
    case ERROR_FAILED_SERVICE_CONTROLLER_CONNECT:
        error = QT_DAEMON_TRANSLATE("The service process could not connect to the service controller.");
        break;
    case ERROR_EXCEPTION_IN_SERVICE:
        error = QT_DAEMON_TRANSLATE("An exception occurred in the service when handling the control request.");
        break;
    case ERROR_SERVICE_SPECIFIC_ERROR:
        error = QT_DAEMON_TRANSLATE("The service has returned a service-specific error code.");
        break;
    case ERROR_PROCESS_ABORTED:
        error = QT_DAEMON_TRANSLATE("The process terminated unexpectedly.");
        break;
    case ERROR_SERVICE_LOGON_FAILED:
        error = QT_DAEMON_TRANSLATE("The service did not start due to a logon failure.");
        break;
    case ERROR_SERVICE_START_HANG:
        error = QT_DAEMON_TRANSLATE("After starting, the service hung in a start-pending state.");
        break;
    case ERROR_INVALID_SERVICE_LOCK:
        error = QT_DAEMON_TRANSLATE("The specified service database lock is invalid.");
        break;
    case ERROR_SERVICE_MARKED_FOR_DELETE:
        error = QT_DAEMON_TRANSLATE("The specified service has been marked for deletion.");
        break;
    case ERROR_SERVICE_EXISTS:
        error = QT_DAEMON_TRANSLATE("The specified service already exists.");
        break;
    case ERROR_SERVICE_NEVER_STARTED:
        error = QT_DAEMON_TRANSLATE("No attempts to start the service have been made since the last boot.");
        break;
    case ERROR_DUPLICATE_SERVICE_NAME:
        error = QT_DAEMON_TRANSLATE("The name is already in use as either a service name or a service display name.");
        break;
    case ERROR_DIFFERENT_SERVICE_ACCOUNT:
        error = QT_DAEMON_TRANSLATE("The account specified for this service is different from the account specified for other services running in the same process.");
        break;
    default:
        error = QT_DAEMON_TRANSLATE("A general error occured (refer to MSDN error codes for more information). Error code: %1").arg(code);
    }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

LPCTSTR QWindowsSystemPath::pathRegistryKey = TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");

QWindowsSystemPath::QWindowsSystemPath()
{
    // Retrieve the system path (RegOpenKeyTransacted breaks compatibility with Windows XP)
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pathRegistryKey, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &registryKey) != ERROR_SUCCESS)  {
        error = QT_DAEMON_TRANSLATE("Couldn't open the PATH registry key. You may need to update the system path manually.");
        registryKey = NULL;
    }
}

QWindowsSystemPath::~QWindowsSystemPath()
{
    if (registryKey && RegCloseKey(registryKey) != ERROR_SUCCESS)
        error = QT_DAEMON_TRANSLATE("Couldn't close the PATH registry key. You may need to update the system path manually.");
}

bool QWindowsSystemPath::addEntry(const QString & dir)
{
    if (!load())
        return false;

    QString path = QDir::cleanPath(dir);
    if (entries.contains(path, Qt::CaseInsensitive))
        return true;

    entries.append(path);
    return save();
}

bool QWindowsSystemPath::removeEntry(const QString & dir)
{
    if (!load())
        return false;

    QString path = QDir::cleanPath(dir);
    for (QStringList::Iterator i = entries.begin(), end = entries.end(); i != end; i++)  {		// Qt doesn't have case insensitive QStringList::removeAll()
        if (i->compare(path, Qt::CaseInsensitive) == 0)
            entries.erase(i);
    }

    return save();
}

bool QWindowsSystemPath::load()
{
    if (!registryKey)
        return false;

    // Get the size
    DWORD bufferSize;
    if (RegQueryValueEx(registryKey, TEXT("Path"), 0, NULL, NULL, &bufferSize) != ERROR_SUCCESS)  {
        error = QT_DAEMON_TRANSLATE("Couldn't retrieve the PATH registry key' size.");
        return false;
    }

    // Allocate enough to contain the PATH
    LPBYTE buffer = new BYTE[bufferSize];

    // Repeat the query, this time with a buffer to get the actual data
    if (RegQueryValueEx(registryKey, TEXT("Path"), 0, NULL, buffer, &bufferSize) != ERROR_SUCCESS)  {
        error = QT_DAEMON_TRANSLATE("Couldn't retrieve the PATH registry key. You may need to update the system path manually.");
        delete [] buffer;
        return false;
    }

    // Continuing to chop any type safety away
    QString path(reinterpret_cast<const QChar * const>(buffer), (bufferSize - 1) / sizeof(TCHAR) );

    // Normalize the paths
    entries = path.split(QLatin1Char(';'), QString::SkipEmptyParts);
    for (QStringList::Iterator i = entries.begin(), end = entries.end(); i != end; i++)
        *i = QDir::cleanPath(*i);

    // Free up the buffer
    delete [] buffer;

    // The PATH has been loaded
    return true;
}

bool QWindowsSystemPath::save()
{
    // Normalize the paths (to native separators)
    for (QStringList::Iterator i = entries.begin(), end = entries.end(); i != end; i++)
        *i = QDir::toNativeSeparators(*i);

    // Convert back to a big fat string
    QString path = entries.join(QLatin1Char(';'));

    // And again, try to forget there's such thing as type safety
    if (RegSetValueEx(registryKey, TEXT("Path"), 0, REG_SZ, reinterpret_cast<LPCBYTE>(path.utf16()), path.size() * sizeof(TCHAR) + 1) != ERROR_SUCCESS)  {
        error = QT_DAEMON_TRANSLATE("Couldn't save the PATH registry key. You may need to update the system path manually.");
        return false;
    }

    // Notify others of the change (don't catch errors as they aren't exactly reported with HWND_BROADCAST)
    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) L"Environment", SMTO_ABORTIFHUNG, serviceNotifyTimeout, NULL);

    return true;
}

QT_DAEMON_END_NAMESPACE
