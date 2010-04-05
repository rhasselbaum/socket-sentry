/***************************************************************************
 *   Copyright (C) 2010 by Rob Hasselbaum <rob@hasselbaum.net>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include "UserNameResolverTest.h"
#include "UserNameResolver.h"
#include "Latch.h"

#include <QtTest/QtTest>
#include <QtCore/QAtomicInt>

uint UserNameResolverTest::MAX_CACHE_ENTRY_AGE_SECS = 3;

// A user name resolver in which the actual lookup has been stubbed out to return fixed strings after waiting on
// a latch.
class UnitTestUserNameResolver : public UserNameResolver {
public:
    UnitTestUserNameResolver() :
        UserNameResolver(3, UserNameResolverTest::MAX_CACHE_ENTRY_AGE_SECS, 1, 50), _count(0) { }
    virtual ~UnitTestUserNameResolver() { }

    // Get shared latch that lets the async lookup proceed.
    Latch& getLatch() { return _startResolveLatch; }

protected:
    // First time though, return "sally". Second time, "rob". Third and subsequent times, "david".
    virtual QString resolveNow(const QString& uid) {
        _startResolveLatch.wait();
        switch (_count.fetchAndAddRelaxed(1)) {
        case 0:
            return "sally";
        case 1:
            return "rob";
        default:
            return "david";
        }
    }

private:
    Latch _startResolveLatch;
    QAtomicInt _count;
};

UserNameResolverTest::UserNameResolverTest() {
}

UserNameResolverTest::~UserNameResolverTest() {
}

void UserNameResolverTest::testLookup() {
    UnitTestUserNameResolver resolver;

    // Test resolving single UID, going through entire entry lifecycle.
    QString uid1 = "1";
    QVERIFY(resolver.resolve(uid1).isEmpty());  // create lookup thread
    QTest::qWait(50);
    QVERIFY(resolver.resolve(uid1).isEmpty());  // lookup still pending with thread blocked on latch (probably)
    Latch& latch = resolver.getLatch();
    latch.flip();
    QString sally("sally");
    waitForResult(resolver, uid1, sally);       // now lookup should succeed with result added to cache
    QCOMPARE(resolver.resolve(uid1), sally);    // cache hit

    // Let entry expire, then re-query.
    QTest::qWait(MAX_CACHE_ENTRY_AGE_SECS * 1000 + 1000);
    QVERIFY(resolver.resolve(uid1).isEmpty());  // entry expired; create lookup thread
    QString rob("rob");
    waitForResult(resolver, uid1, rob);         // now lookup should succeed (same UID, different result)

    // Different UID.
    QString uid2 = "2";
    QVERIFY(resolver.resolve(uid2).isEmpty());  // create lookup thread
    QString david("david");
    waitForResult(resolver, uid2, david);       // now lookup should succeed (same UID, different result)

    // First UID should still be cached.
    QCOMPARE(resolver.resolve(uid1), rob);      // cache hit
}

void UserNameResolverTest::waitForResult(UnitTestUserNameResolver& resolver,
        const QString& uid, const QString& expected) {
    bool gotResult = false;
    for (int attempt = 0; attempt < 3; attempt++) {
        QTest::qWait(1000);
        QString actual = resolver.resolve(uid);
        if (!actual.isEmpty()) {
            QCOMPARE(actual, expected);
            gotResult = true;
            break;
        }
    }
    QVERIFY(gotResult);
}

QTEST_MAIN(UserNameResolverTest)

