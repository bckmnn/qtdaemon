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

#include "QtDaemon/qdaemonapplication.h"
#include "QtDaemon/qdaemon.h"
#include "QtDaemon/qdaemoncontroller.h"
#include "QtDaemon/private/qdaemonapplication_p.h"

#include <QtCore/qtextstream.h>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcommandlineoption.h>

#include <csignal>

QT_DAEMON_BEGIN_NAMESPACE

/*!
    \class QDaemonApplication
    \inmodule QtDaemon

    \brief The \l{QDaemonApplication} class provides an event loop for
    system-level services (deamons).

    It processes the application command line and provides for
    both the daemon itself and the controlling application.
    The application behavior is controlled through command line switches
    passed on startup. When no switches are provided the command line help text
    is displayed. The application object will emit the appropriate signal
    associated with a switch when there are no errors processing the command line.

    \include qtdaemon-cl-switches.qdocinc

    \sa QCoreApplication
*/
/*!
    \fn void QDaemonApplication::ready(const QStringList & arguments)

    This signal is emitted when the process has been daemonized.
    The string list argument \a arguments is a proper command line that
    can be used with a \l{QCommandLineParser} instance.
*/
/*!
    \fn void QDaemonApplication::started()

    This signal is emitted when the application is run as a control terminal
    and the daemon process is started successfully.
*/
/*!
    \fn void QDaemonApplication::stopped()

    This signal is emitted when the application is run as a control terminal
    and the daemon process is stopped successfully.
*/
/*!
    \fn void QDaemonApplication::installed()

    This signal is emitted when the application is run as a control terminal
    and the daemon application has been installed successfully.
*/
/*!
    \fn void QDaemonApplication::uninstalled()

    This signal is emitted when the application is run as a control terminal
    and the daemon application has been uninstalled successfully.
*/

/*!
    Constructs the daemon application object.

    The \a argc and \a argv arguments are processed by the application and are
    made available through the \l{QDaemonApplication::}{ready()} signal.

    \warning The data referred to by \a argc and \a argv must stay valid
    for the entire lifetime of the application object.

    \warning Do not rely on the arguments() function as there are command-line
    parameters used internally by the class.
*/
QDaemonApplication::QDaemonApplication(int & argc, char ** argv)
    : QCoreApplication(argc, argv), d_ptr(new QDaemonApplicationPrivate(this))
{
    qRegisterMetaType<DaemonStatus>();

    QMetaObject::invokeMethod(this, "_q_daemon_exec", Qt::QueuedConnection);
}

/*!
    Destroys the QDaemonApplicaiton object.
*/
QDaemonApplication::~QDaemonApplication()
{
    delete d_ptr;
}

/*!
    \property QDaemonApplication::applicationDescription
    \brief Holds the daemon application's description.

    \note The description is not required, but if supplied it will be used to provide extra
    information with the service control manager (windows) or the init.d script (linux).
*/
QString QDaemonApplication::applicationDescription()
{
    return QDaemonApplicationPrivate::description;
}

void QDaemonApplication::setApplicationDescription(const QString & description)
{
    QDaemonApplicationPrivate::description = description;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------- //

QString QDaemonApplicationPrivate::description;

QDaemonApplicationPrivate::QDaemonApplicationPrivate(QDaemonApplication * q)
    : q_ptr(q)
{
}

QDaemonApplicationPrivate::~QDaemonApplicationPrivate()
{
}

void QDaemonApplicationPrivate::_q_daemon_exec()
{
    Q_Q(QDaemonApplication);

    QStringList arguments = QDaemonApplication::arguments();
    if (arguments.empty())  {
        QDaemon * daemon = new QDaemon(QCoreApplication::applicationName(), q);
        if (daemon->isValid())  {
            // Connect the signal handlers
            std::signal(SIGTERM, QDaemonApplicationPrivate::processSignalHandler);
            std::signal(SIGINT, QDaemonApplicationPrivate::processSignalHandler);
            std::signal(SIGSEGV, QDaemonApplicationPrivate::processSignalHandler);

            // Connect the daemon signals/slots
            QObject::connect(daemon, &QDaemon::ready, q, &QDaemonApplication::ready);
            QObject::connect(daemon, &QDaemon::error, q, &QDaemonApplication::error);

            return;
        }

        q->quit();
        return;
    }

    QCommandLineParser parser;

    const QCommandLineOption install(QStringList() << QT_DAEMON_TRANSLATE("i") << QT_DAEMON_TRANSLATE("install"), QT_DAEMON_TRANSLATE("Install the daemon"));
    const QCommandLineOption uninstall(QStringList() << QT_DAEMON_TRANSLATE("u") << QT_DAEMON_TRANSLATE("uninstall"), QT_DAEMON_TRANSLATE("Uninstall the daemon"));
    const QCommandLineOption start(QStringList() << QT_DAEMON_TRANSLATE("s") << QT_DAEMON_TRANSLATE("start"), QT_DAEMON_TRANSLATE("Start the daemon"));
    const QCommandLineOption stop(QStringList() << QT_DAEMON_TRANSLATE("t") << QT_DAEMON_TRANSLATE("stop"), QT_DAEMON_TRANSLATE("Stop the daemon"));
    const QCommandLineOption status(QT_DAEMON_TRANSLATE("status"), QT_DAEMON_TRANSLATE("Check the daemon status"));

    // TODO: discuss adding the --fake option

    parser.addOption(install);
    parser.addOption(uninstall);
    parser.addOption(start);
    parser.addOption(stop);
    parser.addOption(status);
    parser.addHelpOption();

#if defined(Q_OS_WIN)
    const QCommandLineOption updatePath(QStringLiteral("update-path"), QCoreApplication::translate("main", "Update the system PATH on install/uninstall."));
    parser.addOption(updatePath);
#elif defined(Q_OS_LINUX)
    const QCommandLineOption initdPrefix(QStringLiteral("initd-prefix"), QCoreApplication::translate("main", "Sets the path for the installed init.d script"), QCoreApplication::translate("main", "path"), QStringLiteral("/etc/init.d"));
    const QCommandLineOption dbusPrefix(QStringLiteral("dbus-prefix"), QCoreApplication::translate("main", "Sets the path for the installed dbus configuration file"), QCoreApplication::translate("main", "path"), QStringLiteral("/etc/dbus-1/system.d"));
    parser.addOption(initdPrefix);
    parser.addOption(dbusPrefix);
#elif defined(Q_OS_OSX)
    const QCommandLineOption agent(QStringLiteral("agent"), QCoreApplication::translate("main", "Sets the daemon as an agent"));
    const QCommandLineOption userAgent(QStringLiteral("user"), QCoreApplication::translate("main", "Sets the agent as user local (affects only the agent option)"));
    parser.addOption(agent);
    parser.addOption(userAgent);
#endif

    parser.parse(arguments);

    // Check the flags first
    QDaemonFlags flags;
#if defined(Q_OS_WIN)
    if (parser.isSet(updatePath))
        flags |= QtDaemon::UpdatePathFlag;
#elif defined(Q_OS_OSX)
    if (parser.isSet(agent))  {
        flags |= QtDaemon::AgentFlag;
        if (parser.isSet(userAgent))
            flags |= QtDaemon::UserAgentFlag;
    }
#endif

    // Initialize the controller
    QDaemonController controller(QCoreApplication::applicationName());
    controller.setFlags(flags);
#if defined(Q_OS_LINUX)
    controller.setInitScriptPrefix(parser.value(initdPrefix));
    controller.setDBusConfigurationPrefix(parser.value(dbusPrefix));
#endif
    controller.setDescription(description);

    // Proceed with parsing the main CL option
    if (parser.isSet(install) && controller.install(QCoreApplication::applicationFilePath(), parser.positionalArguments()))
        QMetaObject::invokeMethod(q, "installed", Qt::QueuedConnection);
    else if (parser.isSet(uninstall) && controller.uninstall())
        QMetaObject::invokeMethod(q, "uninstalled", Qt::QueuedConnection);
    else if (parser.isSet(start))  {
        QStringList arguments = parser.positionalArguments();
        if (arguments.isEmpty() ? controller.start() : controller.start(arguments))
            QMetaObject::invokeMethod(q, "started", Qt::QueuedConnection);
    }
    else if (parser.isSet(stop) && controller.stop())
        QMetaObject::invokeMethod(q, "stopped", Qt::QueuedConnection);
    else if (parser.isSet(status))  {
        QtDaemon::DaemonStatus daemonStatus = controller.status();
        QMetaObject::invokeMethod(q, "status", Qt::QueuedConnection, Q_ARG(QtDaemon::DaemonStatus, daemonStatus));
    }
    else  {
        QTextStream out(stdout);
        out << parser.helpText();
    }

    QString errorText = controller.lastError();
    if (!errorText.isEmpty())
        QMetaObject::invokeMethod(q, "error", Qt::QueuedConnection, Q_ARG(const QString &, errorText));

    q->quit();
}

void QDaemonApplicationPrivate::processSignalHandler(int signalNumber)
{
    switch (signalNumber)
    {
    case SIGSEGV:
       ::exit(-1);
    case SIGTERM:
    case SIGINT:
        {
            QCoreApplication * app = QCoreApplication::instance();
            if (app)
                app->quit();
            else
                ::exit(-1);
        }
        break;
    default:
        return;
    }
}

QT_DAEMON_END_NAMESPACE

#include "moc_qdaemonapplication.cpp"
