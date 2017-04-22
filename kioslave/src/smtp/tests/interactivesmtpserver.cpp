/*  -*- c++ -*-
    interactivesmtpserver.cc

    Code based on the serverSocket example by Jesper Pedersen.

    This file is part of the testsuite of kio_smtp, the KDE SMTP kioslave.
    Copyright (c) 2004 Marc Mutz <mutz@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "interactivesmtpserver.h"

static const QHostAddress localhost(0x7f000001);   // 127.0.0.1

static QString err2str(QAbstractSocket::SocketError error)
{
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        return QStringLiteral("Connection refused");
    case QAbstractSocket::HostNotFoundError:
        return QStringLiteral("Host not found");
    default:
        return QStringLiteral("Unknown error");
    }
}

static QString escape(QString s)
{
    return s
           .replace(QLatin1Char('&'), QLatin1String("&amp;"))
           .replace(QLatin1Char('>'), QLatin1String("&gt;"))
           .replace(QLatin1Char('<'), QLatin1String("&lt;"))
           .replace(QLatin1Char('"'), QLatin1String("&quot;"))
    ;
}

static QString trim(const QString &s)
{
    if (s.endsWith(QLatin1String("\r\n"))) {
        return s.left(s.length() - 2);
    }
    if (s.endsWith(QLatin1String("\r")) || s.endsWith(QLatin1String("\n"))) {
        return s.left(s.length() - 1);
    }
    return s;
}

InteractiveSMTPServerWindow::~InteractiveSMTPServerWindow()
{
    if (mSocket) {
        mSocket->close();
        if (mSocket->state() == QAbstractSocket::ClosingState) {
            connect(mSocket, SIGNAL(disconnected()),
                    mSocket, SLOT(deleteLater()));
        } else {
            mSocket->deleteLater();
        }
        mSocket = nullptr;
    }
}

void InteractiveSMTPServerWindow::slotSendResponse()
{
    const QString line = mLineEdit->text();
    mLineEdit->clear();
    QTextStream s(mSocket);
    s << line + QLatin1String("\r\n");
    slotDisplayServer(line);
}

InteractiveSMTPServer::InteractiveSMTPServer(QObject *parent)
    : QTcpServer(parent)
{
    listen(localhost, 2525);
    setMaxPendingConnections(1);

    connect(this, SIGNAL(newConnection()), this, SLOT(newConnectionAvailable()));
}

void InteractiveSMTPServer::newConnectionAvailable()
{
    InteractiveSMTPServerWindow *w = new InteractiveSMTPServerWindow(nextPendingConnection());
    w->show();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    InteractiveSMTPServer server;

    qDebug("Server should now listen on localhost:2525");
    qDebug("Hit CTRL-C to quit.");

    return app.exec();
}

InteractiveSMTPServerWindow::InteractiveSMTPServerWindow(QTcpSocket *socket, QWidget *parent)
    : QWidget(parent)
    , mSocket(socket)
{
    QPushButton *but;
    Q_ASSERT(socket);

    QVBoxLayout *vlay = new QVBoxLayout(this);

    mTextEdit = new QTextEdit(this);
    vlay->addWidget(mTextEdit, 1);
    QWidget *mLayoutWidget = new QWidget;
    vlay->addWidget(mLayoutWidget);

    QHBoxLayout *hlay = new QHBoxLayout(mLayoutWidget);

    mLineEdit = new QLineEdit(this);
    mLabel = new QLabel(QStringLiteral("&Response:"), this);
    mLabel->setBuddy(mLineEdit);
    but = new QPushButton(QStringLiteral("&Send"), this);
    hlay->addWidget(mLabel);
    hlay->addWidget(mLineEdit, 1);
    hlay->addWidget(but);

    connect(mLineEdit, SIGNAL(returnPressed()), SLOT(slotSendResponse()));
    connect(but, SIGNAL(clicked()), SLOT(slotSendResponse()));

    but = new QPushButton(QStringLiteral("&Close Connection"), this);
    vlay->addWidget(but);

    connect(but, SIGNAL(clicked()), SLOT(slotConnectionClosed()));

    connect(socket, SIGNAL(disconnected()), SLOT(slotConnectionClosed()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(slotError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(readyRead()), SLOT(slotReadyRead()));

    mLineEdit->setText(QStringLiteral("220 hi there"));
    mLineEdit->setFocus();
}

void InteractiveSMTPServerWindow::slotDisplayClient(const QString &s)
{
    mTextEdit->append(QLatin1String("C:") + escape(s));
}

void InteractiveSMTPServerWindow::slotDisplayServer(const QString &s)
{
    mTextEdit->append(QLatin1String("S:") + escape(s));
}

void InteractiveSMTPServerWindow::slotDisplayMeta(const QString &s)
{
    mTextEdit->append(QLatin1String("<font color=\"red\">") + escape(s) + QLatin1String("</font>"));
}

void InteractiveSMTPServerWindow::slotReadyRead()
{
    while (mSocket->canReadLine()) {
        slotDisplayClient(trim(QString::fromLatin1(mSocket->readLine())));
    }
}

void InteractiveSMTPServerWindow::slotError(QAbstractSocket::SocketError error)
{
    slotDisplayMeta(QString::fromLatin1("E: %1").arg(err2str(error)));
}

void InteractiveSMTPServerWindow::slotConnectionClosed()
{
    slotDisplayMeta(QStringLiteral("Connection closed by peer"));
}

void InteractiveSMTPServerWindow::slotCloseConnection()
{
    mSocket->close();
}
