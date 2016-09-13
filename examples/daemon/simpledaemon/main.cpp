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

#include <QDaemonApplication>

#include "simpledaemon.h"

using namespace QtDaemon;

int main(int argc, char ** argv)
{
    QDaemonApplication app(argc, argv);

    QDaemonApplication::setApplicationName("SimpleDaemon example");
    QDaemonApplication::setApplicationDescription("The Simple Daemon example shows the basic requirements for creating a daemon");
    QDaemonApplication::setOrganizationDomain("qtdaemon.examples");

    SimpleDaemon controller(&app);
    QObject::connect(&app, &QDaemonApplication::daemonized, &controller, &SimpleDaemon::onDaemonReady);
    QObject::connect(&app, &QDaemonApplication::started, &controller, &SimpleDaemon::onStarted);
    QObject::connect(&app, &QDaemonApplication::stopped, &controller, &SimpleDaemon::onStopped);
    QObject::connect(&app, &QDaemonApplication::installed, &controller, &SimpleDaemon::onInstalled);
    QObject::connect(&app, &QDaemonApplication::uninstalled, &controller, &SimpleDaemon::onUninstalled);

    return QDaemonApplication::exec();
}
