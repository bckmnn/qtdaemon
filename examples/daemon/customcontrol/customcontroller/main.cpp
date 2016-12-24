/****************************************************************************
**
** Copyright (C) 2016 Konstantin Shegunov <kshegunov@gmail.com>
**
** This file is part of the documentation of the QtDaemon library.
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

#include <QCoreApplication>
#include <QTextStream>
#include <QDir>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDaemonController>

using namespace QtDaemon;

int main(int argc, char ** argv)
{
    QCoreApplication::setOrganizationDomain("void.company.domain");
    QCoreApplication::setOrganizationName("Void Company Name Inc.");

    QCoreApplication app(argc, argv);

    const QCommandLineOption install(QStringLiteral("install"), QStringLiteral("Installs the daemon"), QStringLiteral("daemon executable"));
    const QCommandLineOption uninstall(QStringLiteral("uninstall"), QStringLiteral("Uninstalls the daemon"));
    const QCommandLineOption start(QStringLiteral("start"), QStringLiteral("Starts the daemon"));
    const QCommandLineOption stop(QStringLiteral("stop"), QStringLiteral("Stops the daemon"));
    const QCommandLineOption status(QStringLiteral("status"), QStringLiteral("Retrieves the status of the daemon"));

    QCommandLineParser parser;
    parser.addOption(install);
    parser.addOption(uninstall);
    parser.addOption(start);
    parser.addOption(stop);
    parser.addOption(status);
    parser.addHelpOption();

    parser.parse(QCoreApplication::arguments());

    QDaemonController controller("QtDaemon Custom Control example");
    controller.setFlags(QtDaemon::UpdatePathFlag | QtDaemon::AgentFlag | QtDaemon::UserAgentFlag);
    controller.setInitScriptPrefix(QStringLiteral("/home/nye/Temp/daemon/initd"));
    controller.setDBusConfigurationPrefix(QStringLiteral("/home/nye/Temp/daemon/dbus"));
    controller.setDescription(QStringLiteral("The example demonstrates creating a fully customizable daemon and its corresponding controller application."));

    QTextStream out(stdout);

    bool ok = false;
    if (parser.isSet(install))  {
        QString executablePath = parser.value(install);     // Run with --install=../customdaemon/(debug/)customdaemon(.exe)
        QFileInfo executableInfo(executablePath);
        if (!executablePath.isEmpty() && executableInfo.isExecutable())
            ok = controller.install(executableInfo.absoluteFilePath(), parser.positionalArguments());
        else
            out << QStringLiteral("File %1 isn't a valid executable").arg(executablePath) << endl;
    }
    else if (parser.isSet(uninstall))
        ok = controller.uninstall();
    else if (parser.isSet(start))  {
        QStringList arguments = parser.positionalArguments();
        if (arguments.isEmpty())
            ok = controller.start();
        else
            ok = controller.start(arguments);
    }
    else if (parser.isSet(stop))
        ok = controller.stop();
    else if (parser.isSet(status))  {
        out << (controller.status() == QtDaemon::RunningStatus ? "Running" : "Not running") << endl;
        ok = true;
    }
    else  {
        out << parser.helpText();
        ok = true;
    }

    if (!ok)  {
        out << QStringLiteral("Operation failed!");
        return 1;
    }

    out << QStringLiteral("Operation succeeded!");
    return 0;
}
