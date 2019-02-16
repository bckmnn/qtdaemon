/****************************************************************************
**
** Copyright (C) 2016 Samuel Gaist <samuel.gaist@edeltech.ch>
**
** This file is part of the QDaemon library.
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

#include "QtDaemon/qdaemon_global.h"
#include "QtDaemon/private/qdaemoncontroller_p.h"

#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qprocess.h>
#include <QtCore/qregularexpression.h>

QT_DAEMON_BEGIN_NAMESPACE

bool QDaemonControllerPrivate::start()
{
    QProcess launchctl;
    launchctl.start(QStringLiteral("launchctl"), QStringList() << QStringLiteral("load") << state.plistPath());

    if (!launchctl.waitForStarted() || !launchctl.waitForFinished()) {
        lastError = QT_DAEMON_TRANSLATE("Couldn't start the daemon. Error: %1").arg(QString::fromUtf8(launchctl.readAllStandardError()));
        return false;
    }

    return true;
}

bool QDaemonControllerPrivate::stop()
{
    QProcess launchctl;
    launchctl.start(QStringLiteral("launchctl"), QStringList() << QStringLiteral("unload") << state.plistPath());

    if (!launchctl.waitForStarted() || !launchctl.waitForFinished()) {
        lastError = QT_DAEMON_TRANSLATE("Couldn't stop the daemon. Error: %1").arg(QString::fromUtf8(launchctl.readAllStandardError()));
        return false;
    }

    return true;
}

bool QDaemonControllerPrivate::install()
{
    state.generatePListPath();

    QFile plistTemplate(QStringLiteral(":/resources/plist"));
    if (!plistTemplate.open(QFile::ReadOnly | QFile::Text))  {
        lastError = QT_DAEMON_TRANSLATE("Couldn't read the daemon's resources!");
        return false;
    }

    const QString plistPath = state.plistPath();

    QFile plistOutFile(plistPath);
    if (!plistOutFile.open(QFile::WriteOnly | QFile::Text))  {
        lastError = QT_DAEMON_TRANSLATE("Couldn't open output file. Check your permissions for %1!").arg(plistPath);
        return false;
    }

    QTextStream in(&plistTemplate), out(&plistOutFile);

    QString plistTemplateText = in.readAll();
    plistTemplateText = plistTemplateText.replace(QStringLiteral("%%APPLICATION_IDENTIFIER%%"), state.name())
                                         .replace(QStringLiteral("%%APPLICATION_PATH%%"), state.path())
                                         .replace(QStringLiteral("%%APPLICATION_DIRECTORY%%"), state.directory());

    out << plistTemplateText;
    if (out.status() != QTextStream::Ok)  {
        lastError = QT_DAEMON_TRANSLATE("An error occured while writing the plist configuration file.");
        return false;
    }

    return true;
}

bool QDaemonControllerPrivate::uninstall()
{
    if (!stop()) {
        lastError = QT_DAEMON_TRANSLATE("Couldn't stop the daemon");
        return false;
    }

    state.generatePListPath();
    const QString plistPath = state.plistPath();

    if (!QFile::remove(plistPath)) {
        lastError = QT_DAEMON_TRANSLATE("Couldn't remove configuration file. Check your permissions for %1!").arg(plistPath);
        return false;
    }

    return true;
}

QtDaemon::DaemonStatus QDaemonControllerPrivate::status()
{
    QProcess launchctl;
    launchctl.start(QStringLiteral("launchctl"), QStringList() << QStringLiteral("list"));

    if (!launchctl.waitForStarted() || !launchctl.waitForFinished()) {
        lastError = QT_DAEMON_TRANSLATE("Couldn't run launchctl. Error: %1").arg(QString::fromUtf8(launchctl.readAllStandardError()));
        return UnknownStatus;
    }

    QRegularExpression re(QStringLiteral("(\\d+)\\t") + state.name());
    QString launchctlOutput = QString::fromUtf8(launchctl.readAllStandardOutput());

    QRegularExpressionMatch match = re.match(launchctlOutput);
    if (match.hasMatch() && !match.captured(1).toInt())
        return RunningStatus;

    return NotRunningStatus;
}

QT_DAEMON_END_NAMESPACE
