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

#include "customdaemon.h"

#include <QCoreApplication>
#include <QDaemon>

using namespace QtDaemon;

int main(int argc, char ** argv)
{
    QCoreApplication::setOrganizationDomain(QStringLiteral("qtdaemon.examples"));
    QCoreApplication::setOrganizationName(QStringLiteral("QtDaemon examples"));
    QCoreApplication::setApplicationName(QStringLiteral("QtDaemon Custom Control example"));

    QCoreApplication app(argc, argv);

    QDaemon daemon(QtDaemon::SystemScope);

    CustomDaemon controller;

    QObject::connect(&daemon, &QDaemon::ready, &controller, &CustomDaemon::onReady);
    QObject::connect(&daemon, &QDaemon::error, &controller, &CustomDaemon::onError);

    return QCoreApplication::exec();
}
