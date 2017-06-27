/*
    Copyright (c) 2017 Laurent Montel <montel@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef SMTPMAILTRANSPORTPLUGIN_H
#define SMTPMAILTRANSPORTPLUGIN_H

#include <MailTransport/TransportAbstractPlugin>
#include <QVariant>

class SMTPMailTransportPlugin : public MailTransport::TransportAbstractPlugin
{
    Q_OBJECT
public:
    explicit SMTPMailTransportPlugin(QObject *parent = nullptr, const QList<QVariant> & = {});
    ~SMTPMailTransportPlugin();

    QVector<MailTransport::TransportAbstractPluginInfo> names() const override;
    bool configureTransport(const QString &identifier, MailTransport::Transport *transport, QWidget *parent) override;
    void cleanUp(const QString &identifier) override;    
};

#endif // SMTPMAILTRANSPORTPLUGIN_H
