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

#include "HostNameResolverTest.h"
#include "HostNameResolver.h"

#include <QtTest/QtTest>
#include <QtNetwork/QHostInfo>

// A host name resolver in which the DNS integration is stubbed out.
class UnitTestHostNameResolver : public HostNameResolver {
public:
    UnitTestHostNameResolver(uint maxSize, uint maxAgeSecs, uint timerIntervalMs, uint retentionPercent) :
        HostNameResolver(maxSize, maxAgeSecs, timerIntervalMs, retentionPercent), _lookupCounter(0) {
    }
    virtual ~UnitTestHostNameResolver() { }

    // Just return a sequence number starting at 1.
    virtual int resolveAsync(const QString& hostAddress) { return ++_lookupCounter;  }

private:
    int _lookupCounter;
};

HostNameResolverTest::HostNameResolverTest() {
}

HostNameResolverTest::~HostNameResolverTest() {
}

void HostNameResolverTest::testLookup() {
    UnitTestHostNameResolver resolver(5, 1, 0, 50);     // cache is big enough to hold all our test hosts

    // Normal resolution.
    int lookupCount = 1;
    QString capricaAddr = "192.168.168.131";
    QVERIFY(resolver.resolve(capricaAddr).isEmpty());
    QHostInfo caprica = createHostInfo(lookupCount++, QHostInfo::NoError, "caprica");
    QCOMPARE(resolver.getPendingLookups(), 1);
    resolver.lookedUp(caprica);
    QCOMPARE(resolver.getPendingLookups(), 0);
    QVERIFY(resolver.resolve(capricaAddr) == caprica.hostName());

    // Name shouldn't equal IP address, even if Qt says it should.
    QString aerelonAddr = "192.168.168.98";
    QVERIFY(resolver.resolve(aerelonAddr).isEmpty());
    QHostInfo aerelon = createHostInfo(lookupCount++, QHostInfo::NoError, aerelonAddr);
    resolver.lookedUp(aerelon);
    QVERIFY(resolver.resolve(aerelonAddr).isEmpty());

    // Ignore errors.
    QString gemenonAddr = "192.168.168.145";
    QVERIFY(resolver.resolve(gemenonAddr).isEmpty());
    QHostInfo gemenon = createHostInfo(lookupCount++, QHostInfo::HostNotFound, "");
    resolver.lookedUp(gemenon);
    QVERIFY(resolver.resolve(gemenonAddr).isEmpty());

    // ALL lookups should generate cache entries. Even failed ones.
    QCOMPARE(resolver.getCacheSize(), 3);

    // Lookup results for non-existent entries should just be ignored.
    QHostInfo tauron = createHostInfo(lookupCount++, QHostInfo::NoError, "tauron");
    resolver.lookedUp(tauron);
    QCOMPARE(resolver.getPendingLookups(), 0);
    QCOMPARE(resolver.getCacheSize(), 3);
    QCOMPARE(resolver.getPendingLookups(), 0);

}

void HostNameResolverTest::testExpireBySize() {
    // Make cache hold 4, retains 50% after hitting max size, and don't expire entries based on time.
    UnitTestHostNameResolver resolver(4, 300, 0, 50);

    int lookupCount = 1;

    QString capricaAddr = "192.168.168.131";
    QString capricaName = "caprica";
    autoResolve(resolver, lookupCount++, capricaAddr, capricaName);
    QTest::qWait(50);

    QString aerelonAddr = "192.168.168.98";
    QString aerelonName = "aerelon";
    autoResolve(resolver, lookupCount++, aerelonAddr, aerelonName);
    QTest::qWait(50);

    QString gemenonAddr = "192.168.168.145";
    QString gemenonName = "gemenon";
    autoResolve(resolver, lookupCount++, gemenonAddr, gemenonName);
    QTest::qWait(50);

    QString tauronAddr = "192.168.168.188";
    QString tauronName = "tauron";
    autoResolve(resolver, lookupCount++, tauronAddr, tauronName);
    QTest::qWait(50);

    QCOMPARE(capricaName, resolver.resolve(capricaAddr));
    QCOMPARE(aerelonName, resolver.resolve(aerelonAddr));
    QCOMPARE(gemenonName, resolver.resolve(gemenonAddr));
    QCOMPARE(tauronName, resolver.resolve(tauronAddr));
    QCOMPARE(4, resolver.getCacheSize());

    // Now, push the size beyond max.
    QString aquariaAddr = "192.168.168.190";
    QString aquariaName = "aquaria";
    autoResolve(resolver, lookupCount++, aquariaAddr, aquariaName);

    // Should have reduced old size by 50%, plus one new entry. 50% of 4 plus 1 is 3.
    QCOMPARE(3, resolver.getCacheSize());

    // Perform SOFT resolves to validate the updated cache contents without affecting the cache.
    QVERIFY(resolver.softResolve(capricaAddr).isEmpty());       // evicted
    QVERIFY(resolver.softResolve(aerelonAddr).isEmpty());       // evicted
    QCOMPARE(gemenonName, resolver.softResolve(gemenonAddr));   // still in cache
    QCOMPARE(tauronName, resolver.softResolve(tauronAddr));     // still in cache
    QCOMPARE(aquariaName, resolver.softResolve(aquariaAddr));   // newest entry
    QCOMPARE(3, resolver.getCacheSize());

}

void HostNameResolverTest::testExpireByTime() {
    // Make cache expire entries after 1 second.
    UnitTestHostNameResolver resolver(5, 1, 50, 50);

    // Standard expiration.
    QString capricaAddr = "192.168.168.131";
    QString capricaName = "caprica";
    autoResolve(resolver, 1, capricaAddr, capricaName);

    QTest::qWait(200);
    QCOMPARE(capricaName, resolver.softResolve(capricaAddr));   // 200ms (or so) later, still there.

    QTest::qWait(2000);
    QString aerelonAddr = "192.168.168.98";
    QString aerelonName = "aerelon";
    autoResolve(resolver, 2, aerelonAddr, aerelonName);

    QTest::qWait(200);
    QCOMPARE(aerelonName, resolver.softResolve(aerelonAddr));   // 2.2 seconds later, new entry (aerelon) should still be here.
    QVERIFY(resolver.softResolve(capricaAddr).isEmpty());       // Old entry should be long gone now.

    // Now verify that a lookup that never completes will still be expired.
    resolver.resolve(capricaAddr);

    QTest::qWait(200);
    QVERIFY(resolver.resolve(capricaAddr).isEmpty());           // 200ms (or so) later, still waiting.
    QCOMPARE(resolver.getPendingLookups(), 1);

    QTest::qWait(2000);
    QCOMPARE(resolver.getPendingLookups(), 0);                  // gave up

}

void HostNameResolverTest::autoResolve(UnitTestHostNameResolver& resolver, int lookupId,
        const QString& address, const QString& name) const {
    resolver.resolve(address);
    QHostInfo result = createHostInfo(lookupId, QHostInfo::NoError, name);
    resolver.lookedUp(result);
}

QHostInfo HostNameResolverTest::createHostInfo(int lookupId, QHostInfo::HostInfoError error, const QString& name) const {
    QHostInfo result;
    result.setLookupId(lookupId);
    result.setError(error);
    result.setHostName(name);
    return result;
}

QTEST_MAIN(HostNameResolverTest)
