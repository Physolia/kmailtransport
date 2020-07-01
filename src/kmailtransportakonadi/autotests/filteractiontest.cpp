/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filteractiontest.h"
#include <collectionpathresolver_p.h>
#include <testattribute.h>

#include <attributefactory.h>
#include <collectioncreatejob.h>
#include <itemcreatejob.h>
#include <itemfetchjob.h>
#include <itemfetchscope.h>
#include <itemmodifyjob.h>
#include <qtest_akonadi.h>
#include <filteractionjob_p.h>

using namespace Akonadi;

QTEST_AKONADIMAIN(FilterActionTest, NoGUI)

static const QByteArray acceptable = "acceptable";
static const QByteArray unacceptable = "unacceptable";
static const QByteArray modified = "modified";
static Collection res1;

class MyFunctor : public FilterAction
{
    virtual Akonadi::ItemFetchScope fetchScope() const
    {
        ItemFetchScope scope;
        scope.fetchAttribute<TestAttribute>();
        return scope;
    }

    virtual bool itemAccepted(const Akonadi::Item &item) const
    {
        if (!item.hasAttribute<TestAttribute>()) {
            return false;
        }
        return item.attribute<TestAttribute>()->data == acceptable;
    }

    virtual Akonadi::Job *itemAction(const Akonadi::Item &item, FilterActionJob *parent) const
    {
        Item cp(item);
        TestAttribute *newa = new TestAttribute;
        newa->data = modified;
        cp.addAttribute(newa);
        return new ItemModifyJob(cp, parent);
    }
};

void FilterActionTest::initTestCase()
{
    AttributeFactory::registerAttribute<TestAttribute>();

    CollectionPathResolver *resolver = new CollectionPathResolver("res1", this);
    QVERIFY(resolver->exec());
    res1 = Collection(resolver->collection());
}

void FilterActionTest::testMassModifyItem()
{
    Collection col = createCollection("testMassModifyItem");

    // Test acceptable item.
    Item item = createItem(col, true);
    FilterActionJob *mjob = new FilterActionJob(item, new MyFunctor, this);
    AKVERIFYEXEC(mjob);
    ItemFetchJob *fjob = new ItemFetchJob(item, this);
    fjob->fetchScope().fetchAllAttributes();
    AKVERIFYEXEC(fjob);
    QCOMPARE(fjob->items().count(), 1);
    item = fjob->items().first();
    QVERIFY(item.hasAttribute<TestAttribute>());
    TestAttribute *attr = item.attribute<TestAttribute>();
    QCOMPARE(attr->data, modified);

    // Test unacceptable item.
    item = createItem(col, false);
    mjob = new FilterActionJob(item, new MyFunctor, this);
    AKVERIFYEXEC(mjob);
    fjob = new ItemFetchJob(item, this);
    fjob->fetchScope().fetchAllAttributes();
    AKVERIFYEXEC(fjob);
    QCOMPARE(fjob->items().count(), 1);
    item = fjob->items().first();
    QVERIFY(item.hasAttribute<TestAttribute>());
    attr = item.attribute<TestAttribute>();
    QCOMPARE(attr->data, unacceptable);
}

void FilterActionTest::testMassModifyItems()
{
    Collection col = createCollection("testMassModifyItems");

    // Test a bunch of acceptable and unacceptable items.
    Item::List acc, unacc;
    for (int i = 0; i < 5; i++) {
        acc.append(createItem(col, true));
        unacc.append(createItem(col, false));
    }
    Item::List all = acc + unacc;
    QCOMPARE(all.count(), 10);
    FilterActionJob *mjob = new FilterActionJob(all, new MyFunctor, this);
    AKVERIFYEXEC(mjob);
    ItemFetchJob *fjob = new ItemFetchJob(col, this);
    fjob->fetchScope().fetchAllAttributes();
    AKVERIFYEXEC(fjob);
    QCOMPARE(fjob->items().count(), 10);
    foreach (const Item &item, fjob->items()) {
        QVERIFY(item.hasAttribute<TestAttribute>());
        const QByteArray data = item.attribute<TestAttribute>()->data;
        if (data == unacceptable) {
            QVERIFY(unacc.contains(item));
            unacc.removeAll(item);
        } else if (data == modified) {
            QVERIFY(acc.contains(item));
            acc.removeAll(item);
        } else {
            QVERIFY2(false, QByteArray(QByteArray("Got bad data \"") + data + QByteArray("\"")));
        }
    }
    QCOMPARE(acc.count(), 0);
    QCOMPARE(unacc.count(), 0);
}

void FilterActionTest::testMassModifyCollection()
{
    Collection col = createCollection("testMassModifyCollection");

    // Test a bunch of acceptable and unacceptable items.
    Item::List acc, unacc;
    for (int i = 0; i < 5; i++) {
        acc.append(createItem(col, true));
        unacc.append(createItem(col, false));
    }
    FilterActionJob *mjob = new FilterActionJob(col, new MyFunctor, this);
    qDebug() << "Executing FilterActionJob.";
    AKVERIFYEXEC(mjob);
    ItemFetchJob *fjob = new ItemFetchJob(col, this);
    fjob->fetchScope().fetchAllAttributes();
    AKVERIFYEXEC(fjob);
    QCOMPARE(fjob->items().count(), 10);
    foreach (const Item &item, fjob->items()) {
        QVERIFY(item.hasAttribute<TestAttribute>());
        const QByteArray data = item.attribute<TestAttribute>()->data;
        if (data == unacceptable) {
            QVERIFY(unacc.contains(item));
            unacc.removeAll(item);
        } else if (data == modified) {
            QVERIFY(acc.contains(item));
            acc.removeAll(item);
        } else {
            QVERIFY2(false, QByteArray(QByteArray("Got bad data \"") + data + QByteArray("\"")));
        }
    }
    QCOMPARE(acc.count(), 0);
    QCOMPARE(unacc.count(), 0);
}

Collection FilterActionTest::createCollection(const QString &name)
{
    Collection col;
    col.setParentCollection(res1);
    col.setName(name);
    CollectionCreateJob *ccjob = new CollectionCreateJob(col, this);
    Q_ASSERT(ccjob->exec());
    return ccjob->collection();
}

Item FilterActionTest::createItem(const Collection &col, bool accept)
{
    Q_ASSERT(col.isValid());

    Item item;
    item.setMimeType("text/directory");
    TestAttribute *attr = new TestAttribute;
    if (accept) {
        attr->data = acceptable;
    } else {
        attr->data = unacceptable;
    }
    item.addAttribute(attr);
    ItemCreateJob *cjob = new ItemCreateJob(item, col, this);
    Q_ASSERT(cjob->exec());
    return cjob->item();
}
