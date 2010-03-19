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

#include <QtNetwork/QHostAddress>
#include <QtCore/QHash>
#include <QtCore/QPair>
#include <sys/time.h>

#include "NetworkHistoryTest.h"
#include "NetworkHistory.h"
#include "IpEndpointPair.h"
#include "FlowMetrics.h"
#include "FlowStatistics.h"

NetworkHistoryTest::NetworkHistoryTest() {
}

NetworkHistoryTest::~NetworkHistoryTest() {
}

void NetworkHistoryTest::testExportStatistics() {
    NetworkHistory history(6, 4);
    IpEndpointPair webFlow(QHostAddress("192.168.100.1"), 123, QHostAddress("10.10.1.1"), 80, TCP);
    IpEndpointPair sshFlow(QHostAddress("192.168.100.1"), 543, QHostAddress("10.10.1.20"), 22, TCP);
    time_t timePos = 100;
    // These will be ignored because they will fall out of range.
    history.record(webFlow, FlowMetrics(35000, 2320, 24, 5), timePos);
    history.record(sshFlow, FlowMetrics(2320, 7535, 1, 2), timePos);
    timePos++;
    // Now we are within the historical range.
    history.record(webFlow, FlowMetrics(3000, 0, 2, 0), timePos);
    history.record(sshFlow, FlowMetrics(30, 4590, 5, 2), timePos);
    history.record(webFlow, FlowMetrics(0, 97672, 0, 15), timePos);
    history.record(sshFlow, FlowMetrics(0, 6274, 0, 9), timePos);
    timePos++;
    history.record(webFlow, FlowMetrics(85000, 4000, 39, 4), timePos);
    timePos++;
    history.record(sshFlow, FlowMetrics(92766, 4250, 56, 40), timePos);
    timePos += 2;
    history.record(webFlow, FlowMetrics(1970, 0, 1, 0), timePos);
    history.record(sshFlow, FlowMetrics(0, 7420, 0, 3), timePos);
    timePos++;
    // This will be ignored because it's the current time slot.
    history.record(webFlow, FlowMetrics(5600, 2923, 12, 2), timePos);

    // Set result expectations.
    FlowMetrics expectedWebMetrics(89970, 101672, 42, 19);
    FlowStatistics expectedWebStats(656, 0, 100672, false, true);
    FlowMetrics expectedSshMetrics(92796, 22534, 61, 54);
    FlowStatistics expectedSshStats(30922, 3889, 97016, true, false);

    // Export and extract results.
    QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> > result;
    history.exportStatistics(result, timePos);
    QPair<FlowMetrics, FlowStatistics> webResult = result.value(webFlow);
    QPair<FlowMetrics, FlowStatistics> sshResult = result.value(sshFlow);

    // Verify.
    QCOMPARE(webResult.first, expectedWebMetrics);
    QCOMPARE(webResult.second, expectedWebStats);
    QCOMPARE(sshResult.first, expectedSshMetrics);
    QCOMPARE(sshResult.second, expectedSshStats);

}

void NetworkHistoryTest::testRecordAndRoll() {
    IpEndpointPair endpoints(QHostAddress("192.168.100.1"), 123, QHostAddress("10.10.1.1"), 80, TCP);
    const time_t time = 1000;
    FlowMetrics firstMetrics(1000, 0, 1, 0);
    NetworkHistory history(3, 2);
    history.record(endpoints, firstMetrics, time);          // 1st metrics recorded at time+0
    FlowMetrics secondMetrics(500, 0, 2, 0);
    history.record(endpoints, secondMetrics, time + 1);     // 2nd metrics recorded at time+1
    FlowMetrics combinedMetrics(1500, 0, 3, 0);             // combines 1st and 2nd metrics
    QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> > result;

    // Results don't include the current time.
    history.exportStatistics(result, time + 1);
    verifySingleResult(result, endpoints, firstMetrics);    // time+1 includes 1st metrics (2nd metrics are in current time slot)

    history.exportStatistics(result, time + 2);
    verifySingleResult(result, endpoints, combinedMetrics); // time+2 includes 1st and 2nd metrics

    history.exportStatistics(result, time + 3);
    verifySingleResult(result, endpoints, secondMetrics);   // time+3 includes 2nd metrics (1st has expired)

    history.exportStatistics(result, time + 4);
    QCOMPARE(result.size(), 0);                             // time+4 and both have expired

    history.record(endpoints, firstMetrics, time + 5);      // 1st metrics recorded at time+5
    history.exportStatistics(result, time + 5);
    QCOMPARE(result.size(), 0);                             // time+5 includes no metrics (1st metrics are in current time slot)

    history.exportStatistics(result, time + 8);
    QCOMPARE(result.size(), 0);                             // jump to time+8 and all metrics have expired

    history.record(endpoints, firstMetrics, time + 5);      // very old metrics recorded
    history.exportStatistics(result, time + 8);
    QCOMPARE(result.size(), 0);                             // very old metrics ignored

    history.record(endpoints, firstMetrics, time + 6);      // slightly old metrics recorded
    history.exportStatistics(result, time + 8);
    verifySingleResult(result, endpoints, firstMetrics);    // slightly old metrics fall within historical range

    history.record(endpoints, secondMetrics, time + 8);     // 2nd metrics recorded at time+8
    history.exportStatistics(result, time + 10);
    verifySingleResult(result, endpoints, secondMetrics);   // time+10 still includes 2nd metrics (not quite expired)

    // Test "any traffic since".
    QVERIFY(!history.anyTrafficSince(time + 9));
    history.record(endpoints, firstMetrics, time + 9);
    QVERIFY(history.anyTrafficSince(time + 9));
}

void NetworkHistoryTest::verifySingleResult(const QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& result,
        const IpEndpointPair& endpoints, const FlowMetrics& metrics) {
    QCOMPARE(result.size(), 1);
    QVERIFY(result.contains(endpoints));
    QPair<FlowMetrics, FlowStatistics> actualPair = result.value(endpoints);
    QCOMPARE(actualPair.first, metrics);
}

QTEST_MAIN(NetworkHistoryTest)
