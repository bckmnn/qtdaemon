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

#include "tcpsession.h"

#include <QDaemonApplication>
#include <QDaemonLog>

using namespace QtDaemon;

TcpSession::TcpSession(QObject * parent)
    : QObject(parent), socket(this)
{
    QObject::connect(&socket, &QTcpSocket::connected, this, &TcpSession::opened);
    QObject::connect(&socket, &QTcpSocket::disconnected, this, &TcpSession::closed);
    QObject::connect(&socket, &QTcpSocket::readyRead, this, &TcpSession::readSocketData);
}

void TcpSession::open(qintptr descriptor)
{
    if (!socket.setSocketDescriptor(descriptor))  {
        QMetaObject::invokeMethod(this, "closed", Qt::QueuedConnection);    // Give up on the session
        return;
    }
}

void TcpSession::close()
{
    socket.disconnectFromHost();   // Will emit closed() when the socket connection is dropped
}

void TcpSession::readSocketData()
{
    QString message = socket.readAll();
    if (message.trimmed() == "quit")
        qApp->quit();
    else if (message.trimmed() == "close")
        close();
    else
        qDaemonLog() << message;
}
