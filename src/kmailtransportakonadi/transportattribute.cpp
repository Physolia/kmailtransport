/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "transportattribute.h"

#include "transportmanager.h"

using namespace Akonadi;
using namespace MailTransport;

class MailTransport::TransportAttributePrivate
{
public:
    int mId = -1;
};

TransportAttribute::TransportAttribute(int id)
    : d(new TransportAttributePrivate)
{
    d->mId = id;
}

TransportAttribute::~TransportAttribute() = default;

TransportAttribute *TransportAttribute::clone() const
{
    return new TransportAttribute(d->mId);
}

QByteArray TransportAttribute::type() const
{
    static const QByteArray sType("TransportAttribute");
    return sType;
}

QByteArray TransportAttribute::serialized() const
{
    return QByteArray::number(d->mId);
}

void TransportAttribute::deserialize(const QByteArray &data)
{
    d->mId = data.toInt();
}

int TransportAttribute::transportId() const
{
    return d->mId;
}

Transport *TransportAttribute::transport() const
{
    return TransportManager::self()->transportById(d->mId, false);
}

void TransportAttribute::setTransportId(int id)
{
    d->mId = id;
}
