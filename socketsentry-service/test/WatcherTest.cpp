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

#include "WatcherTest.h"
#include "Watcher.h"
#include "MockPcapManager.h"
#include "MockConnectionProcessCorrelator.h"
#include "TestMain.h"
#include "IpEndpointPair.h"
#include "FlowMetrics.h"
#include "FlowStatistics.h"
#include "OsProcess.h"
#include "CommunicationFlow.h"

#include <QtCore/QString>
#include <QtTest/QSignalSpy>
#include <QtCore/QStringList>
#include <QtNetwork/QHostAddress>
#include <QtCore/QHash>
#include <QtCore/QPair>
#include <QtCore/QDateTime>

using ::testing::Expectation;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::ReturnRef;
using ::testing::InSequence;
using ::testing::Assign;
using ::testing::_;

ACTION_P(ReturnPointee, p) { return *p; }

WatcherTest::WatcherTest() {
}

WatcherTest::~WatcherTest() {
}

void WatcherTest::testVariableOsConnections() {

    // One device.
    QString eth0 = "eth0";
    // Create mock pcap manager that expects to be capturing on this device.
    MockPcapManager* mockPcapMngr = createFullServicePcapManager(eth0, true);

    // Create two endpoints with stats that the mock pcap manager will return.
    // It returns the same two endpoints every time.
    IpEndpointPair endpoints1 = createEndpoints(2920, "147.129.1.1");
    IpEndpointPair endpoints2 = createEndpoints(4345, "10.20.1.1");
    FlowMetrics metrics(64000, 1200, 30, 40);
    FlowStatistics stats(3200, 4500, 96000, true, false);
    QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> > filledStats = createPacketStats(endpoints1, endpoints2, metrics, stats);
    EXPECT_CALL(*mockPcapMngr, fillStatistics(eth0, _, _))
        .Times(AtLeast(1))
        .WillRepeatedly(DoAll(SetArgReferee<1>(filledStats), Return(true)));
    EXPECT_CALL(*mockPcapMngr, isActive(eth0))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));

    // Create mock connection-process correlator that returns:
    // 1) Two connections (only one matching endpoints from the pcap manager), then
    // 2) Error
    QHash<IpEndpointPair, QList<OsProcess> > filledCorrelation;
    QList<OsProcess> processes1;
    OsProcess process1(222, "firefox", "rob", QDateTime::currentDateTime());
    processes1 << process1;
    filledCorrelation.insert(endpoints1, processes1);     // same endpoints as pcap manager
    IpEndpointPair endpoints3 = createEndpoints(6442, "192.168.1.5");
    QList<OsProcess> processes2;
    OsProcess process2(333, "wget", "rob", QDateTime::currentDateTime());
    processes2 << process2;
    filledCorrelation.insert(endpoints3, processes2);     // different endpoints
    QString correlationError = "Correlation error";
    MockConnectionProcessCorrelator* mockCorrelator = new MockConnectionProcessCorrelator;
    EXPECT_CALL(*mockCorrelator, correlate(_, _))
        .Times(AtLeast(2))
        .WillOnce(DoAll(SetArgReferee<0>(filledCorrelation), Return(true)))          // 1st time through, two OS connections
        .WillRepeatedly(DoAll(SetArgReferee<1>(correlationError), Return(false)));   // 2nd time, error

    // Create watcher and set it to monitor "eth0". It should emit some signals over time.
    const int correlationIntervalMs = 200;
    Watcher watcher(mockCorrelator, mockPcapMngr, 25, 100, correlationIntervalMs);
    QSignalSpy updateSpy(&watcher, SIGNAL(update(const QString&, const QList<CommunicationFlow>&)));
    QSignalSpy failureSpy(&watcher, SIGNAL(failure(const QString&, const QString&)));
    watcher.showInterest(eth0);
    QTest::qWait(correlationIntervalMs * 3 + 1000);    // wait through 3 correlation intervals plus 1 sec slack

    // Verify update and failure signals.
    verifyUpdateSignals(updateSpy, eth0, endpoints1, processes1, metrics, stats);
    QCOMPARE(failureSpy.count(), 1);    // Should get EXACTLY one failure for when the correlator returns an error.
    QList<QVariant> failureArgs = failureSpy.takeFirst();
    QCOMPARE(failureArgs.at(0).toString(), eth0);
    QCOMPARE(failureArgs.at(1).toString(), correlationError);
}

void WatcherTest::testVariableCaptures() {
    // One device.
    QString eth0 = "eth0";
    // Create mock pcap manager that expects to be capturing on this device.
    MockPcapManager* mockPcapMngr = createFullServicePcapManager(eth0, false);

    // Create two endpoints with stats that the mock pcap manager will return. It returns (in order):
    // 1) The filled statistics, then
    // 2) Error
    IpEndpointPair endpoints1 = createEndpoints(2920, "147.129.1.1");
    IpEndpointPair endpoints2 = createEndpoints(4345, "10.20.1.1");
    FlowMetrics metrics(64000, 1200, 30, 40);
    FlowStatistics stats(3200, 4500, 96000, true, false);
    QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> > filledStats = createPacketStats(endpoints1, endpoints2, metrics, stats);
    QString captureError = "Capture error";
    EXPECT_CALL(*mockPcapMngr, fillStatistics(eth0, _, _))
        .Times(AtLeast(2))
        .WillOnce(DoAll(SetArgReferee<1>(filledStats), Return(true)))
        .WillRepeatedly(DoAll(SetArgReferee<2>(captureError), Return(false)));
    EXPECT_CALL(*mockPcapMngr, isActive(eth0))
        .Times(AtLeast(2))
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));

    // Create mock connection-process correlator that returns the same connection every time, which matches endpoints1.
    QHash<IpEndpointPair, QList<OsProcess> > filledCorrelation;
    OsProcess process(222, "firefox", "rob", QDateTime::currentDateTime());
    QList<OsProcess> processes;
    processes << process;
    filledCorrelation.insert(endpoints1, processes);     // same endpoints as pcap manager
    MockConnectionProcessCorrelator* mockCorrelator = new MockConnectionProcessCorrelator;
    EXPECT_CALL(*mockCorrelator, correlate(_, _))
        .Times(AtLeast(1))
        .WillRepeatedly(DoAll(SetArgReferee<0>(filledCorrelation), Return(true)));

    // Create watcher and set it to monitor "eth0". It should emit some signals over time.
    const int updateIntervalMs = 100;
    Watcher watcher(mockCorrelator, mockPcapMngr, 25, updateIntervalMs, 200);
    QSignalSpy updateSpy(&watcher, SIGNAL(update(const QString&, const QList<CommunicationFlow>&)));
    QSignalSpy failureSpy(&watcher, SIGNAL(failure(const QString&, const QString&)));
    watcher.showInterest(eth0);
    QTest::qWait(updateIntervalMs * 2 + 1000);    // wait through 2 update intervals plus 1 sec slack

    // Verify update and failure signals.
    verifyUpdateSignals(updateSpy, eth0, endpoints1, processes, metrics, stats);
    QCOMPARE(failureSpy.count(), 1);    // Should get EXACTLY one failure for when the correlator returns an error.
    QList<QVariant> failureArgs = failureSpy.takeFirst();
    QCOMPARE(failureArgs.at(0).toString(), eth0);
    QCOMPARE(failureArgs.at(1).toString(), captureError);

}

void WatcherTest::verifyUpdateSignals(QSignalSpy& updateSpy, const QString& deviceName, const IpEndpointPair& endpoints,
        const QList<OsProcess>& osProcesses, const FlowMetrics& metrics, const FlowStatistics& stats) const {

    // Check the update signal(s).
    QVERIFY(updateSpy.count() >= 1);      // Should get at least one update signal.
    bool foundFlow = false;
    while (!updateSpy.isEmpty()) {
        // Verify that the update contains the one shared endpoint pair or nothing.
        QList<QVariant> updateArgs = updateSpy.takeFirst();
        QCOMPARE(updateArgs.at(0).toString(), deviceName);
        const QList<CommunicationFlow>& flows = qvariant_cast<QList<CommunicationFlow> >(updateArgs.at(1));
        if (!flows.isEmpty()) {
            QCOMPARE(flows.size(), 1);
            CommunicationFlow expected(endpoints, osProcesses, metrics, stats);
            QCOMPARE(flows[0], expected);
            foundFlow = true;
        }
    }
}

void WatcherTest::testProcessSorting() {
    // One device.
    QString eth0 = "eth0";
    // Create mock pcap manager that expects to be capturing on this device.
    MockPcapManager* mockPcapMngr = createNicePcapManager(eth0);

    // Create one endpoints with empty stats that the mock pcap manager will return.
    // It returns the same two endpoints every time.
    IpEndpointPair endpoints = createEndpoints(2920, "147.129.1.1");
    QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> > filledStats;
    filledStats.insert(endpoints, QPair<FlowMetrics, FlowStatistics>());
    EXPECT_CALL(*mockPcapMngr, fillStatistics(eth0, _, _))
        .Times(AtLeast(1))
        .WillRepeatedly(DoAll(SetArgReferee<1>(filledStats), Return(true)));
    EXPECT_CALL(*mockPcapMngr, isActive(eth0))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));

    // Create mock connection-process correlator that returns one connection with three processes.
    const QDateTime now = QDateTime::currentDateTime();
    const QDateTime later = now.addSecs(1);
    const int basePid = 100;
    OsProcess process1(basePid, "apache", "rob", now);
    OsProcess process2(basePid + 2, "apache", "rob", later);
    OsProcess process3(basePid + 1, "apache", "rob", later);
    QHash<IpEndpointPair, QList<OsProcess> > filledCorrelation;
    QList<OsProcess> unsortedProcesses;
    unsortedProcesses << process1 << process2 << process3;
    filledCorrelation.insert(endpoints, unsortedProcesses);     // same endpoints as pcap manager
    MockConnectionProcessCorrelator* mockCorrelator = new MockConnectionProcessCorrelator;
    EXPECT_CALL(*mockCorrelator, correlate(_, _))
        .Times(AtLeast(1))
        .WillRepeatedly(DoAll(SetArgReferee<0>(filledCorrelation), Return(true)));

    // Create watcher and set it to monitor "eth0". It should emit some signals over time.
    const int updateIntervalMs = 10;
    Watcher watcher(mockCorrelator, mockPcapMngr, 5, updateIntervalMs, 5);
    watcher.setOsProcessSortAscending(true);
    QSignalSpy updateSpy(&watcher, SIGNAL(update(const QString&, const QList<CommunicationFlow>&)));
    watcher.showInterest(eth0);
    QTest::qWait(updateIntervalMs * 2 + 1000);    // wait through 2 update intervals plus 1 sec slack

    // Verify oldest-to-newest sort.
    QList<OsProcess> oldestToNewest;
    oldestToNewest << process1 << process3 << process2;
    verifyUpdateSignals(updateSpy, eth0, endpoints, oldestToNewest, FlowMetrics(), FlowStatistics());

    // Verify newest-to-oldest sort.
    watcher.setOsProcessSortAscending(false);
    watcher.showInterest(eth0);
    QTest::qWait(updateIntervalMs * 2 + 1000);    // wait through 2 update intervals plus 1 sec slack

    QList<OsProcess> newestToOldest;
    newestToOldest << process2 << process3 << process1;
    verifyUpdateSignals(updateSpy, eth0, endpoints, newestToOldest, FlowMetrics(), FlowStatistics());

}

void WatcherTest::initTestCase() {
    qRegisterMetaType<QList<CommunicationFlow> >("QList<CommunicationFlow>");
}

QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> > WatcherTest::createPacketStats(const IpEndpointPair& endpoints1,
        const IpEndpointPair& endpoints2, const FlowMetrics& metrics, const FlowStatistics& stats) const {
    QPair<FlowMetrics, FlowStatistics> filledTrafficNumbers(metrics, stats);
    QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> > filledStats;
    filledStats.insert(endpoints1, filledTrafficNumbers);
    QPair<FlowMetrics, FlowStatistics> emptyTrafficNumbers;
    filledStats.insert(endpoints2, emptyTrafficNumbers);
    return filledStats;
}

MockPcapManager* WatcherTest::createNicePcapManager(const QString& deviceName) const {
    QStringList fullDeviceList;
    fullDeviceList << deviceName;
    QStringList emptyDeviceList;

    // Create mock pcap manager that expects to be capturing on this device.
    MockPcapManager* mockPcapMngr = new MockPcapManager;
    EXPECT_CALL(*mockPcapMngr, showInterest(deviceName))
        .Times(AtLeast(1));
    EXPECT_CALL(*mockPcapMngr, anyTrafficSince(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mockPcapMngr, findCurrentDevices())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(fullDeviceList));
    return mockPcapMngr;

}

MockPcapManager* WatcherTest::createFullServicePcapManager(const QString& deviceName, bool expectStopAll) const {
    QStringList fullDeviceList;
    fullDeviceList << deviceName;
    QStringList emptyDeviceList;

    // Create mock pcap manager that expects to be capturing on this device.
    MockPcapManager* mockPcapMngr = new MockPcapManager;
    EXPECT_CALL(*mockPcapMngr, showInterest(deviceName))
        .Times(1);
    EXPECT_CALL(*mockPcapMngr, anyTrafficSince(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    {
        InSequence s;
        EXPECT_CALL(*mockPcapMngr, findCurrentDevices())
            .Times(AtLeast(1))
            .WillRepeatedly(Return(fullDeviceList));
        if (expectStopAll) {
            EXPECT_CALL(*mockPcapMngr, releaseAll())
                .Times(1);
        } else {
            EXPECT_CALL(*mockPcapMngr, release(deviceName))
                .Times(1);
        }
        EXPECT_CALL(*mockPcapMngr, findCurrentDevices())
            .Times(AtLeast(1))
            .WillRepeatedly(Return(emptyDeviceList));

    }
    return mockPcapMngr;
}

IpEndpointPair WatcherTest::createEndpoints(int localPort, const QString& remoteAddr) const {
    QHostAddress local("192.168.168.131");
    QHostAddress remote(remoteAddr);
    return IpEndpointPair(local, localPort, remote, 80, TCP);
}

QTEST_GMOCK_MAIN(WatcherTest)
