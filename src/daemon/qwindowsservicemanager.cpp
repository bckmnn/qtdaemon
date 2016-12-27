#include "QtDaemon/private/qwindowsservicemanager_p.h"

#include <QtCore/qelapsedtimer.h>
#include <QtCore/qdir.h>

QT_DAEMON_BEGIN_NAMESPACE

QWindowsServiceManager QWindowsServiceManager::open()
{
    // Open a handle to the service manager
    SC_HANDLE manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!manager)
        qDaemonLog(QStringLiteral("Couldn't open a handle to the service manager (Error code %1)").arg(GetLastError()), QDaemonLog::ErrorEntry);

    return QWindowsServiceManager(manager);
}

void QWindowsServiceManager::close()
{
    if (handle && !CloseServiceHandle(handle))
        qDaemonLog(QStringLiteral("Error while closing the service manager (Error code %1).").arg(GetLastError()), QDaemonLog::WarningEntry);

    handle = Q_NULLPTR;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

static const qint32 serviceNotifyTimeout = 30000;			// Up to 30 seconds

bool QWindowsService::open(int access)
{
    LPCTSTR lpName = reinterpret_cast<LPCTSTR>(serviceName.utf16());

    // Open the service
    handle = OpenService(manager.handle, lpName, access);
    if (!handle)  {
        DWORD error = GetLastError();
        switch (error)
        {
        case ERROR_ACCESS_DENIED:
            qDaemonLog(QStringLiteral("Couldn't open service control, access denied."), QDaemonLog::ErrorEntry);
            break;
        case ERROR_SERVICE_DOES_NOT_EXIST:
            qDaemonLog(QStringLiteral("The service is not installed."), QDaemonLog::ErrorEntry);
            break;
        default:
            qDaemonLog(QStringLiteral("Couldn't open the service (Error code %1).").arg(error), QDaemonLog::ErrorEntry);
        }
    }

    return handle;
}

void QWindowsService::close()
{
    if (handle && !CloseServiceHandle(handle))
        qDaemonLog(QStringLiteral("Error while closing the service handle (Error code %1).").arg(GetLastError()), QDaemonLog::WarningEntry);

    handle = Q_NULLPTR;
}

bool QWindowsService::create()
{
    LPCTSTR lpName = reinterpret_cast<LPCTSTR>(serviceName.utf16());
    LPCTSTR lpPath = reinterpret_cast<LPCTSTR>(serviceExecutable.utf16());

    // Create the service
    handle = CreateService(manager.handle, lpName, lpName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, lpPath, NULL, NULL, NULL, NULL, NULL);
    if (!handle)  {
        DWORD error =  GetLastError();
        switch (error)
        {
        case ERROR_ACCESS_DENIED:
            qDaemonLog(QStringLiteral("Couldn't install the service, access denied."), QDaemonLog::ErrorEntry);
            break;
        case ERROR_DUPLICATE_SERVICE_NAME:
        case ERROR_SERVICE_EXISTS:
            qDaemonLog(QStringLiteral("The service was already installed (Uninstall first)."), QDaemonLog::ErrorEntry);
            break;
        default:
            qDaemonLog(QStringLiteral("Couldn't install the service (Error code %1).").arg(error), QDaemonLog::ErrorEntry);
        }

        return false;
    }

    // Set the description if provided
    if (!serviceDescription.isEmpty())  {
        LPTSTR lpDescription = reinterpret_cast<LPTSTR>(const_cast<ushort *>(serviceDescription.utf16()));		// MS doesn't care about constness

        SERVICE_DESCRIPTION serviceDescription;
        serviceDescription.lpDescription = lpDescription;
        if (!ChangeServiceConfig2(handle, SERVICE_CONFIG_DESCRIPTION, &serviceDescription))
            qDaemonLog(QStringLiteral("Couldn't set the service description (Error code %1).").arg(GetLastError()), QDaemonLog::WarningEntry);
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
    if (!DeleteService(handle))  {
        DWORD error = GetLastError();
        switch (error)
        {
        case ERROR_SERVICE_MARKED_FOR_DELETE:
            qDaemonLog(QStringLiteral("The service had already been marked for deletion."), QDaemonLog::ErrorEntry);
            break;
        default:
            qDaemonLog(QStringLiteral("Couldn't delete the service (Error code %1).").arg(error), QDaemonLog::ErrorEntry);
        }

        return false;
    }

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
        DWORD error = GetLastError();
        switch (error)
        {
        case ERROR_ACCESS_DENIED:
            qDaemonLog(QStringLiteral("Couldn't stop the service: Access denied."), QDaemonLog::ErrorEntry);
            break;
        case ERROR_PATH_NOT_FOUND:
            qDaemonLog(QStringLiteral("Couldn't start the service: The path to the executable is set incorrectly."), QDaemonLog::ErrorEntry);
            break;
        case ERROR_SERVICE_ALREADY_RUNNING:
            qDaemonLog(QStringLiteral("The service is already running."), QDaemonLog::ErrorEntry);
            break;
        case ERROR_SERVICE_REQUEST_TIMEOUT:
            qDaemonLog(QStringLiteral("The service is not responding."), QDaemonLog::ErrorEntry);
            break;
        default:
            qDaemonLog(QStringLiteral("Couldn't start the service (Error code %1).").arg(error), QDaemonLog::ErrorEntry);
        }

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
        DWORD error = GetLastError();
        switch (error)
        {
        case ERROR_ACCESS_DENIED:
            qDaemonLog(QStringLiteral("Couldn't stop the service, access denied."), QDaemonLog::ErrorEntry);
            break;
        case ERROR_SERVICE_NOT_ACTIVE:
            qDaemonLog(QStringLiteral("The service is not running."), QDaemonLog::ErrorEntry);
            break;
        case ERROR_SERVICE_REQUEST_TIMEOUT:
            qDaemonLog(QStringLiteral("The service is not responding."), QDaemonLog::ErrorEntry);
            break;
        default:
            qDaemonLog(QStringLiteral("Couldn't stop the service (Error code %1).").arg(error), QDaemonLog::ErrorEntry);
        }

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
        DWORD error = GetLastError();
        switch (error)
        {
        case ERROR_ACCESS_DENIED:
            qDaemonLog(QStringLiteral("Couldn't get the service's status, access denied."), QDaemonLog::ErrorEntry);
            break;
        default:
            qDaemonLog(QStringLiteral("Couldn't get the service's status (Error code %1).").arg(error), QDaemonLog::ErrorEntry);
        }

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

    qDaemonLog(QStringLiteral("The service is not responding."), QDaemonLog::ErrorEntry);
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

LPCTSTR QWindowsSystemPath::pathRegistryKey = TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");

QWindowsSystemPath::QWindowsSystemPath()
{
    // Retrieve the system path (RegOpenKeyTransacted breaks compatibility with Windows XP)
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pathRegistryKey, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &registryKey) != ERROR_SUCCESS)  {
        qDaemonLog(QStringLiteral("Couldn't open the PATH registry key. You may need to update the system path manually."), QDaemonLog::WarningEntry);
        registryKey = Q_NULLPTR;
    }
}

QWindowsSystemPath::~QWindowsSystemPath()
{
    if (registryKey && RegCloseKey(registryKey) != ERROR_SUCCESS)
        qDaemonLog(QStringLiteral("Couldn't close the PATH registry key. You may need to update the system path manually."), QDaemonLog::WarningEntry);
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
        qDaemonLog(QStringLiteral("Couldn't retrieve the PATH registry key' size."), QDaemonLog::WarningEntry);
        return false;
    }

    // Allocate enough to contain the PATH
    LPBYTE buffer = new BYTE[bufferSize];

    // Repeat the query, this time with a buffer to get the actual data
    if (RegQueryValueEx(registryKey, TEXT("Path"), 0, NULL, buffer, &bufferSize) != ERROR_SUCCESS)  {
        qDaemonLog(QStringLiteral("Couldn't retrieve the PATH registry key. You may need to update the system path manually."), QDaemonLog::WarningEntry);
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
        qDaemonLog(QStringLiteral("Couldn't save the PATH registry key. You may need to update the system path manually."), QDaemonLog::WarningEntry);
        return false;
    }

    // Notify others of the change (don't catch errors as they aren't exactly reported with HWND_BROADCAST)
    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) L"Environment", SMTO_ABORTIFHUNG, serviceNotifyTimeout, NULL);

    return true;
}

QT_DAEMON_END_NAMESPACE
