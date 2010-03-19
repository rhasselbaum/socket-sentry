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

#ifndef WATCHERTEST_H_
#define WATCHERTEST_H_

#include <QtCore/QObject>

class IpEndpointPair;
class MockPcapManager;
class FlowMetrics;
class FlowStatistics;
template <class K, class V> class QHash;
template <class F, class S> class QPair;
class QSignalSpy;
class OsProcess;

/*
 * Unit test for Watcher.
 */
class WatcherTest : public QObject {
    Q_OBJECT

public:
    WatcherTest();
    virtual ~WatcherTest();

private slots:
    void initTestCase();
    // Ensure the watcher reacts correctly when the connection-process correlator returns some valid data and an error.
    void testVariableOsConnections();
    // Ensure the watcher reacts correctly when the pcap manager returns some valid data and an error.
    void testVariableCaptures();
    // Ensure the watcher sorts processes correctly in shared socket situations.
    void testProcessSorting();

private:
    // Create a dummy endpoint pair with variable local port and remote address.
    IpEndpointPair createEndpoints(int localPort, const QString& remoteAddr) const;

    // Verify that the signal spy has captured at least one update signal matching the given device. At least one
    // update must include exactly the communication flow provided as input. The rest must be empty.
    void verifyUpdateSignals(QSignalSpy& updateSpy, const QString& deviceName, const IpEndpointPair& endpoints,
            const QList<OsProcess>& osProcesses, const FlowMetrics& metrics, const FlowStatistics& stats) const;

    // Create packet manager stats containing two endpoint pairs. The first pair contains the given metrics and stats. The second pair is empty.
    QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> > createPacketStats(const IpEndpointPair& endpoints1,
            const IpEndpointPair& endpoints2, const FlowMetrics& endpoints1Metrics, const FlowStatistics& endpoints1Stats) const;

    // Create a new mock pcap manager that expects (at least once):
    // 1) "Show interest" on the given device name.
    // 2) "Find current devices" returning the given device name.
    // 3) "Is capturing" returning true, followed by stop or stop all, followed by
    //    "Is capturing" returning false. The expectStopAll argument controls whether
    //    the mock expects a stop call on the single device or all devices.
    // Caller takes ownership of the mock's memory.
    MockPcapManager* createFullServicePcapManager(const QString& deviceName, bool expectStopAll) const;

    // Create a new mock pcap manager that expects (at least once):
    // 1) "Show interest" on the given device name.
    // 2) "Find current devices" returning the given device name.
    // 3) "Is capturing" returning true
    // Caller takes ownership of the mock's memory.
    MockPcapManager* createNicePcapManager(const QString& deviceName) const;

};


#endif /* WATCHERTEST_H_ */
