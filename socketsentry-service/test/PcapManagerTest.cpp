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

#include "PcapManagerTest.h"
#include "TestMain.h"
#include "PcapManager.h"
#include "IPcapThread.h"
#include "MockPcapThread.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtTest/QTest>
#include <gmock/gmock.h>

using ::testing::Return;
using ::testing::AtLeast;
using ::testing::AnyNumber;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::ReturnRef;
using ::testing::InSequence;
using ::testing::_;

// A subclass of pcap manager that creates mock threads for thread lifecycle management testing.
class FullServicePcapManager : public PcapManager {
public:
    FullServicePcapManager() :
        PcapManager(PcapManagerTest::TIMER_INTERVAL_MS, true), _threadsCreated(0) {
    }

    // Get number of threads created by the factory method.
    int getThreadsCreated() const { return _threadsCreated; }

protected:
    // Factory method to create a new IPcapThread instance. May be overridden in a subclass for unit tests (mock threads).
    virtual IPcapThread* createPcapThread(const QString& device, const QString& customFilter) {

        // Create a mock thread that expects keep-alive calls. The first one succeeds, the
        // second one fails (thread shutting down).
        MockPcapThread* mock = new MockPcapThread();
        EXPECT_CALL(*mock, keepAlive())
            .Times(AnyNumber())
            .WillOnce(Return(true))
            .WillRepeatedly(Return(false));
        // The mock may expect a call to fill statistics, which succeeds.
        EXPECT_CALL(*mock, fillStatistics(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(true));

        // Life cycle methods.
        {
            // Mock thread must be started. Eventually, the pcap manager cancels it to shut it down.
            // This could happen in response to a failed keep-alive request, but not necessarily.
            // Once it's been "canceled", it takes some time to shut down, so isDone returns false
            // the first time, true the second.
            InSequence s;
            EXPECT_CALL(*mock, begin())
                .Times(1);
            EXPECT_CALL(*mock, cancel())
                .Times(1);
            EXPECT_CALL(*mock, isDone())
                .Times(2)
                .WillOnce(Return(false))
                .WillRepeatedly(Return(true));
        }
        _threadsCreated++;
        return mock;
    }

private:
    int _threadsCreated;
};

// A subclass of pcap manager that creates mock threads. The first reports no
// traffic. The second and subsequent ones always report traffic.
class TrafficTestPcapManager : public PcapManager {
public:
    TrafficTestPcapManager() :
        PcapManager(PcapManagerTest::TIMER_INTERVAL_MS, true), _createdOneThread(false) {
    }

protected:
    // Factory method to create a new IPcapThread instance. May be overridden in a subclass for unit tests (mock threads).
    virtual IPcapThread* createPcapThread(const QString& device, const QString& customFilter) {

        // First thread reports no traffic.
        MockPcapThread* mock = new MockPcapThread();
        EXPECT_CALL(*mock, anyTrafficSince(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(_createdOneThread));
        EXPECT_CALL(*mock, begin())
            .Times(1);
        EXPECT_CALL(*mock, cancel())
            .Times(1);
        EXPECT_CALL(*mock, isDone())
            .Times(AtLeast(1))
            .WillRepeatedly(Return(true));
        _createdOneThread = true;
        return mock;
    }

private:
    bool _createdOneThread;
};

// A subclass of pcap manager that creates mock threads, which always report that they are active.
class ActiveThreadTestPcapManager : public PcapManager {
public:
    ActiveThreadTestPcapManager() :
        PcapManager(PcapManagerTest::TIMER_INTERVAL_MS, true) {
    }

protected:
    // Factory method to create a new IPcapThread instance.
    virtual IPcapThread* createPcapThread(const QString& device, const QString& customFilter) {
        MockPcapThread* mock = new MockPcapThread();
        EXPECT_CALL(*mock, begin())
            .Times(1);
        EXPECT_CALL(*mock, canContinue())
            .Times(AtLeast(1))
            .WillRepeatedly(Return(true));
        EXPECT_CALL(*mock, cancel())
            .Times(1);
        EXPECT_CALL(*mock, isDone())
            .Times(AtLeast(1))
            .WillRepeatedly(Return(true));
        return mock;
    }
};



// Timer interval to use in the PcapManager under test.
const int PcapManagerTest::TIMER_INTERVAL_MS = 10;

PcapManagerTest::PcapManagerTest() {
}

PcapManagerTest::~PcapManagerTest() {
}

void PcapManagerTest::testThreadManagement() {
    // What we're trying to validate here is that new threads get created after old ones are finished, and
    //  all threads are deleted. GMock warns us if a mock thread doesn't get deleted.
    FullServicePcapManager pcapManager;
    QString eth0 = "eth0";
    QString eth1 = "eth1";
    pcapManager.showInterest(eth0);
    QVERIFY(!pcapManager.isStopped());
    QCOMPARE(pcapManager.getThreadsCreated(), 1);
    pcapManager.showInterest(eth0);           // calls keep-alive, which succeeds the first time
    QCOMPARE(pcapManager.getThreadsCreated(), 1);
    pcapManager.showInterest(eth0);           // keep-alive fails this time, manager creates new thread
    QCOMPARE(pcapManager.getThreadsCreated(), 2);
    pcapManager.showInterest(eth0);           // calls keep-alive on 2nd thread, which succeeds
    QCOMPARE(pcapManager.getThreadsCreated(), 2);
    pcapManager.showInterest(eth1);           // new device, new thread
    QCOMPARE(pcapManager.getThreadsCreated(), 3);

    // Two devices should be active.
    QStringList allDevices;
    allDevices << eth0 << eth1;
    QCOMPARE(pcapManager.findCurrentDevices(), allDevices);

    // Test polling for statistics.
    QString error;
    QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> > stats;
    QVERIFY(pcapManager.fillStatistics(eth0, stats, error));      // device exists; should succeed
    QVERIFY(error.isEmpty());
    QVERIFY(!pcapManager.fillStatistics("lo", stats, error));     // not capturing; should fail
    QVERIFY(!error.isEmpty());

    // Manually release one device.
    pcapManager.release(eth1);
    QTest::qWait(TIMER_INTERVAL_MS * 2 + 500);      // allow threads to drain out
    QStringList eth0Only;
    eth0Only << eth0;
    QCOMPARE(pcapManager.findCurrentDevices(), eth0Only);

    // Release remaining devices.
    pcapManager.releaseAll();
    QVERIFY(!pcapManager.isStopped());
    QTest::qWait(TIMER_INTERVAL_MS * 2 + 500);      // allow threads to drain out
    QStringList noDevices;
    QCOMPARE(pcapManager.findCurrentDevices(), noDevices);
    QVERIFY(pcapManager.isStopped());
}

void PcapManagerTest::testAnyTrafficSince() {
    TrafficTestPcapManager pcapManager;
    pcapManager.showInterest("eth0");
    time_t timeSecs = 1000;
    QVERIFY(!pcapManager.anyTrafficSince(timeSecs));
    pcapManager.showInterest("eth1");
    QVERIFY(pcapManager.anyTrafficSince(timeSecs));
    pcapManager.releaseAll();
    QTest::qWait(TIMER_INTERVAL_MS * 2 + 500);      // allow threads to drain out
    QVERIFY(pcapManager.isStopped());
}

void PcapManagerTest::testActiveThreadProbe() {
    ActiveThreadTestPcapManager pcapManager;
    pcapManager.showInterest("eth0");
    QVERIFY(pcapManager.isActive("eth0"));
    time_t timeSecs = 1000;
    pcapManager.releaseAll();
    QVERIFY(!pcapManager.isActive("eth0"));
    QTest::qWait(TIMER_INTERVAL_MS * 2 + 500);      // allow threads to drain out
    QVERIFY(pcapManager.isStopped());

}

QTEST_GMOCK_MAIN(PcapManagerTest)
