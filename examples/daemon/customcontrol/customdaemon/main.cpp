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
#include <QDateTime>
#include <QFile>
#include <QTextStream>

#include <QDaemon>

using namespace QtDaemon;

int main(int argc, char ** argv)
{
    QCoreApplication::setOrganizationDomain("void.company.domain");
    QCoreApplication::setOrganizationName("Void Company Name Inc.");

    QCoreApplication app(argc, argv);

    QDaemon daemon("QtDaemon Custom Control example");
    QObject::connect(&daemon, &QDaemon::ready, [&daemon] (const QStringList & arguments) -> void {
        QFile outFile(QStringLiteral("%1.log").arg(daemon.filePath()));
        if (!outFile.open(QFile::WriteOnly | QFile::Append | QFile::Text))
            return;

        QTextStream out(&outFile);

        out << QStringLiteral("[%1] The custom daemon is running! Arguments: %2").arg(QDateTime::currentDateTime().toString(Qt::ISODate)).arg(arguments.join(' ')) << endl;
    });

    return QCoreApplication::exec();
}
