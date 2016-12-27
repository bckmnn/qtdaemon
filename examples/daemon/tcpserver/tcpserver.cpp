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

#include "tcpserver.h"
#include "tcpsession.h"

#include <QThread>
#include <QCommandLineParser>
#include <QCommandLineOption>

const quint16 TcpServer::defaultPort = 5890;

TcpServer::TcpServer(QObject * parent)
    : QTcpServer(parent), logFile("tcpserver.log"), out(stdout)
{
    qRegisterMetaType<qintptr>("qintptr");

    if (logFile.open(QFile::WriteOnly | QFile::Append | QFile::Text))
        out.setDevice(&logFile);
}

void TcpServer::start(const QStringList & arguments)
{
    // Parse the command line
    const QCommandLineOption portOption(QStringList() << QStringLiteral("p") << QStringLiteral("port"), QStringLiteral("The port the server will listen to."), QStringLiteral("<port number>"));

    QCommandLineParser parser;
    parser.addOption(portOption);
    parser.parse(arguments);

    // Set the port
    quint16 port = defaultPort;
    if (parser.isSet(portOption))  {
        bool ok;
        quint16 portOptionValue = parser.value(portOption).toInt(&ok);
        if (ok)
            port = portOptionValue;
    }

    // Start the TCP server
    if (!listen(QHostAddress::Any, port))  {
        // We couldn't start the server ...
        // Log the error
        out << QStringLiteral("Couldn't start the server. QTcpServer::listen() failed") << endl;
        // Give up and quit
        qApp->quit();
        return;
    }
}

void TcpServer::stop()
{
    if (!isListening())
        return;

    close();

    emit stopped();

    // Wait for the remaining session threads to finish
    threadWaitSemaphore.acquire(activeSessions.fetchAndStoreAcquire(0));
}

void TcpServer::threadStarted()
{
    activeSessions.fetchAndAddAcquire(1);
}

void TcpServer::threadFinished()
{
    if (activeSessions.fetchAndSubAcquire(1) <= 0)
        threadWaitSemaphore.release();
}

void TcpServer::incomingConnection(qintptr descriptor)
{
    QThread * thread = new QThread;
    TcpSession * session = new TcpSession;

    QObject::connect(session, &TcpSession::messageReceived, this, &TcpServer::logMessage);

    QObject::connect(this, &TcpServer::stopped, session, &TcpSession::close);
    QObject::connect(this, &TcpServer::stopped, thread, &QThread::quit);
    QObject::connect(session, &TcpSession::closed, thread, &QThread::quit);

    QObject::connect(thread, &QThread::started, this, &TcpServer::threadStarted, Qt::DirectConnection);
    QObject::connect(thread, &QThread::finished, this, &TcpServer::threadFinished, Qt::DirectConnection);

    QObject::connect(thread, &QThread::finished, session, &QObject::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
    session->moveToThread(thread);

    // Open the session in the worker thread
    QMetaObject::invokeMethod(session, "open", Qt::QueuedConnection, Q_ARG(qintptr, descriptor));
}

void TcpServer::logMessage(const QString & message)
{
    out << message << endl;
}
