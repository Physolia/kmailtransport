/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "outboxactions_p.h"

#include "mailtransportakonadi_debug.h"
#include "kmailtransportakonadi/dispatchmodeattribute.h"
#include "kmailtransportakonadi/errorattribute.h"

#include <itemmodifyjob.h>
#include <messageflags.h>

using namespace Akonadi;
using namespace MailTransport;

SendQueuedAction::SendQueuedAction()
{
}

SendQueuedAction::~SendQueuedAction()
{
}

ItemFetchScope SendQueuedAction::fetchScope() const
{
    ItemFetchScope scope;
    scope.fetchFullPayload(false);
    scope.fetchAttribute<DispatchModeAttribute>();
    scope.fetchAttribute<ErrorAttribute>();
    scope.setCacheOnly(true);
    return scope;
}

bool SendQueuedAction::itemAccepted(const Item &item) const
{
    if (!item.hasAttribute<DispatchModeAttribute>()) {
        qCWarning(MAILTRANSPORTAKONADI_LOG) << "Item doesn't have DispatchModeAttribute.";
        return false;
    }

    return item.attribute<DispatchModeAttribute>()->dispatchMode() == DispatchModeAttribute::Manual;
}

Job *SendQueuedAction::itemAction(const Item &item, FilterActionJob *parent) const
{
    Item cp = item;
    cp.addAttribute(new DispatchModeAttribute);   // defaults to Automatic
    if (cp.hasAttribute<ErrorAttribute>()) {
        cp.removeAttribute<ErrorAttribute>();
        cp.clearFlag(Akonadi::MessageFlags::HasError);
    }
    return new ItemModifyJob(cp, parent);
}

ClearErrorAction::ClearErrorAction()
{
}

ClearErrorAction::~ClearErrorAction()
{
}

ItemFetchScope ClearErrorAction::fetchScope() const
{
    ItemFetchScope scope;
    scope.fetchFullPayload(false);
    scope.fetchAttribute<ErrorAttribute>();
    scope.setCacheOnly(true);
    return scope;
}

bool ClearErrorAction::itemAccepted(const Item &item) const
{
    return item.hasAttribute<ErrorAttribute>();
}

Job *ClearErrorAction::itemAction(const Item &item, FilterActionJob *parent) const
{
    Item cp = item;
    cp.removeAttribute<ErrorAttribute>();
    cp.clearFlag(Akonadi::MessageFlags::HasError);
    cp.setFlag(Akonadi::MessageFlags::Queued);
    return new ItemModifyJob(cp, parent);
}

DispatchManualTransportAction::DispatchManualTransportAction(int transportId)
    : mTransportId(transportId)
{
}

DispatchManualTransportAction::~DispatchManualTransportAction()
{
}

ItemFetchScope DispatchManualTransportAction::fetchScope() const
{
    ItemFetchScope scope;
    scope.fetchFullPayload(false);
    scope.fetchAttribute<TransportAttribute>();
    scope.fetchAttribute<DispatchModeAttribute>();
    scope.setCacheOnly(true);
    return scope;
}

bool DispatchManualTransportAction::itemAccepted(const Item &item) const
{
    if (!item.hasAttribute<DispatchModeAttribute>()) {
        qCWarning(MAILTRANSPORTAKONADI_LOG) << "Item doesn't have DispatchModeAttribute.";
        return false;
    }

    if (!item.hasAttribute<TransportAttribute>()) {
        qCWarning(MAILTRANSPORTAKONADI_LOG) << "Item doesn't have TransportAttribute.";
        return false;
    }

    return item.attribute<DispatchModeAttribute>()->dispatchMode() == DispatchModeAttribute::Manual;
}

Job *DispatchManualTransportAction::itemAction(const Item &item, FilterActionJob *parent) const
{
    Item cp = item;
    cp.attribute<TransportAttribute>()->setTransportId(mTransportId);
    cp.removeAttribute<DispatchModeAttribute>();
    cp.addAttribute(new DispatchModeAttribute);   // defaults to Automatic
    cp.setFlag(Akonadi::MessageFlags::Queued);
    return new ItemModifyJob(cp, parent);
}
