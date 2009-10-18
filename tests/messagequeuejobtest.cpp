/*
    Copyright 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "messagequeuejobtest.h"

#include <QStringList>

#include <akonadi/agentinstance.h>
#include <akonadi/agentmanager.h>
#include <akonadi/collection.h>
#include <akonadi/collectionstatistics.h>
#include <akonadi/collectionstatisticsjob.h>
#include <akonadi/control.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/qtest_akonadi.h>
#include <akonadi/kmime/specialcollections.h>
#include <akonadi/kmime/specialcollectionsrequestjob.h>
#include <akonadi/kmime/addressattribute.h>

#include <kmime/kmime_message.h>
#include <boost/shared_ptr.hpp>

#include <mailtransport/dispatchmodeattribute.h>
#include <mailtransport/errorattribute.h>
#include <mailtransport/messagequeuejob.h>
#include <mailtransport/sentbehaviourattribute.h>
#include <mailtransport/transport.h>
#include <mailtransport/transportattribute.h>
#include <mailtransport/transportmanager.h>

#define SPAM_ADDRESS ( QStringList() << "idanoka@gmail.com" )

using namespace Akonadi;
using namespace KMime;
using namespace MailTransport;


void MessageQueueJobTest::initTestCase()
{
  Control::start();
  // HACK: Otherwise the MDA is not switched offline soon enough apparently...
  QTest::qWait( 1000 );

  // Switch MDA offline to avoid spam.
  AgentInstance mda = AgentManager::self()->instance( "akonadi_maildispatcher_agent" );
  QVERIFY( mda.isValid() );
  mda.setIsOnline( false );

  // check that outbox is empty
  SpecialCollectionsRequestJob *rjob = new SpecialCollectionsRequestJob( this );
  rjob->requestDefaultCollection( SpecialCollections::Outbox );
  QTest::kWaitForSignal( rjob, SIGNAL(result(KJob*)) );
  verifyOutboxContents( 0 );
}

void MessageQueueJobTest::testValidMessages()
{
  // check transport
  int tid = TransportManager::self()->defaultTransportId();
  QVERIFY2( tid >= 0, "I need a default transport, but there is none." );

  // send a valid message using the default transport
  MessageQueueJob *qjob = new MessageQueueJob;
  qjob->setTransportId( tid );
  Message::Ptr msg = Message::Ptr( new Message );
  msg->setContent( "\nThis is message #1 from the MessageQueueJobTest unit test.\n" );
  qjob->setMessage( msg );
  qjob->setTo( SPAM_ADDRESS );
  verifyOutboxContents( 0 );
  AKVERIFYEXEC( qjob );

  // fetch the message and verify it
  QTest::qWait( 1000 );
  verifyOutboxContents( 1 );
  ItemFetchJob *fjob = new ItemFetchJob( SpecialCollections::self()->defaultCollection( SpecialCollections::Outbox ) );
  fjob->fetchScope().fetchFullPayload();
  fjob->fetchScope().fetchAllAttributes();
  AKVERIFYEXEC( fjob );
  QCOMPARE( fjob->items().count(), 1 );
  Item item = fjob->items().first();
  QVERIFY( !item.remoteId().isEmpty() ); // stored by the resource
  QVERIFY( item.hasPayload<Message::Ptr>() );
  AddressAttribute *addrA = item.attribute<AddressAttribute>();
  QVERIFY( addrA );
  QVERIFY( addrA->from().isEmpty() );
  QCOMPARE( addrA->to().count(), 1 );
  QCOMPARE( addrA->to(), SPAM_ADDRESS );
  QCOMPARE( addrA->cc().count(), 0 );
  QCOMPARE( addrA->bcc().count(), 0 );
  DispatchModeAttribute *dA = item.attribute<DispatchModeAttribute>();
  QVERIFY( dA );
  QCOMPARE( dA->dispatchMode(), DispatchModeAttribute::Immediately ); // default mode
  SentBehaviourAttribute *sA = item.attribute<SentBehaviourAttribute>();
  QVERIFY( sA );
  QCOMPARE( sA->sentBehaviour(), SentBehaviourAttribute::MoveToDefaultSentCollection ); // default sent collection
  TransportAttribute *tA = item.attribute<TransportAttribute>();
  QVERIFY( tA );
  QCOMPARE( tA->transportId(), tid );
  ErrorAttribute *eA = item.attribute<ErrorAttribute>();
  QVERIFY( !eA ); // no error
  QCOMPARE( item.flags().count(), 1 );
  QVERIFY( item.flags().contains( "queued" ) );

  // delete message, for further tests
  ItemDeleteJob *djob = new ItemDeleteJob( item );
  AKVERIFYEXEC( djob );
  verifyOutboxContents( 0 );

  // TODO test with no To: but only BCC:

  // TODO test due-date sending

  // TODO test sending with custom sent-mail collections
}

void MessageQueueJobTest::testInvalidMessages()
{
  MessageQueueJob *job = 0;
  Message::Ptr msg;

  // without message
  job = new MessageQueueJob;
  job->setTransportId( TransportManager::self()->defaultTransportId() );
  job->setTo( SPAM_ADDRESS );
  QVERIFY( !job->exec() );

  // without recipients
  job = new MessageQueueJob;
  msg = Message::Ptr( new Message );
  msg->setContent( "\nThis is a message sent from the MessageQueueJobTest unittest. This shouldn't have been sent.\n" );
  job->setMessage( msg );
  job->setTransportId( TransportManager::self()->defaultTransportId() );
  QVERIFY( !job->exec() );

  // without transport
  job = new MessageQueueJob;
  msg = Message::Ptr( new Message );
  msg->setContent( "\nThis is a message sent from the MessageQueueJobTest unittest. This shouldn't have been sent.\n" );
  job->setMessage( msg );
  job->setTo( SPAM_ADDRESS );
  QVERIFY( !job->exec() );

  // with AfterDueDate and no due date
  job = new MessageQueueJob;
  msg = Message::Ptr( new Message );
  msg->setContent( "\nThis is a message sent from the MessageQueueJobTest unittest. This shouldn't have been sent.\n" );
  job->setMessage( msg );
  job->setTo( SPAM_ADDRESS );
  job->setDispatchMode( DispatchModeAttribute::AfterDueDate );
  QVERIFY( !job->exec() );

  // with MoveToCollection and no sent-mail folder
  job = new MessageQueueJob;
  msg = Message::Ptr( new Message );
  msg->setContent( "\nThis is a message sent from the MessageQueueJobTest unittest. This shouldn't have been sent.\n" );
  job->setMessage( msg );
  job->setTo( SPAM_ADDRESS );
  job->setSentBehaviour( SentBehaviourAttribute::MoveToCollection );
  QVERIFY( !job->exec() );
}

void MessageQueueJobTest::verifyOutboxContents( qlonglong count )
{
  QVERIFY( SpecialCollections::self()->hasDefaultCollection( SpecialCollections::Outbox ) );
  Collection outbox = SpecialCollections::self()->defaultCollection( SpecialCollections::Outbox );
  QVERIFY( outbox.isValid() );
  CollectionStatisticsJob *job = new CollectionStatisticsJob( outbox );
  AKVERIFYEXEC( job );
  QCOMPARE( job->statistics().count(), count );
}


QTEST_AKONADIMAIN( MessageQueueJobTest, NoGUI )

#include "messagequeuejobtest.moc"
