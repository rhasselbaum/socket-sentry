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

#ifndef WATCHER_H_
#define WATCHER_H_

#include "HostNameResolver.h"
#include "IPcapManager.h"

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QDebug>

class PcapThread;
class QTimerEvent;
class QStringList;
class IpEndpointPair;
class OsProcess;
template<class F, class S> class QPair;
template<class T> class QList;
class CommunicationFlow;
class FlowMetrics;
class FlowStatistics;
class IConnectionProcessCorrelator;


/*
 * Main interface that provides access to real-time traffic flows. Fetch methods must be
 * invoked with high privilege (i.e. root or whatever privilege is required to perform packet
 * captures and access all process file descriptors on the host).
 */
class Watcher : public QObject {
    Q_OBJECT

public:
    // New instance.
    Watcher();
    // New instance with the given helpers and time intervals. Useful for unit testing. This object takes ownership over
    // the helpers' memory. Time intervals can also be set here.
    Watcher(IConnectionProcessCorrelator* correlator, IPcapManager* pcapManager, int timerIntervalMs, int updateIntervalMs,
            int correlationIntervalMs);

    virtual ~Watcher();

    // Fund the list of network devices that can be watched. Updates error argument if list
    // cannot be obtained due to lack of permission or another problem.
    QStringList findDevices(QString& error) const;

    // True if this watcher should do network name resolution. This may be expensive.
    bool getResolveNames() const { return _resolveNames; }
    void setResolveNames(bool resolveNames) { _resolveNames = resolveNames; }

    // If true, each list of OS processes sharing an IP endpoint pair socket is sorted in ascending order by start
    // time and PID. If false, the order is reversed.
    bool getOsProcessSortAscending() const { return _osProcessSortAscending; }
    void setOsProcessSortAscending(bool osProcessSortAscending) { _osProcessSortAscending  = osProcessSortAscending; }

    // A custom pcap filter applied across all devices.
    QString getCustomFilter() const { return _pcapManager->getCustomFilter(); }
    void setCustomFilter(const QString& customFilter) { _pcapManager->setCustomFilter(customFilter); }

public slots:
    // Show interest in a device for a period of time. The watcher will begin monitoring this device if it
    // is not already doing so and periodically emit "update" signals with current traffic statistics (or a "failure"
    // signal in case of a problem). If this watcher is already watching the device, then calling this
    // method will keep the monitoring alive for a renewed period of time. Clients must show interest periodically
    // in order to get uninterrupted updates. Otherwise, the watcher will eventually stop monitoring the device.
    // If a failure signal is emitted for the device, clients can expect no further updates until they show
    // interest again. If the caller supplies an invalid device name to this method, no action is taken. Use
    // "findDevices" to see what devices are available.
    void showInterest(const QString& device);

signals:
    // Traffic update for the specified device.
    void update(const QString& device, const QList<CommunicationFlow>& flows);

    // Indicates a failure watching the given device. After a failure signal, there will be no further updates until
    // a client shows interest in the device again.
    void failure(const QString& device, const QString& error);

protected:
    // Perform periodic tasks (emit signals, compute statistics, etc.).
    void timerEvent(QTimerEvent* event);

private:
    // Generate communication flows by matching up packet capture statistics to corresponding OS
    // connections and processes. For each match, a row is added to the result argument. Optionally,
    // host names will be resolved in this step. If the same socket (IP endpoint pair) is shared by
    // multiple processes, the process list in the resultant communication flow object is ordered
    // according to the watcher's "OS process sort asending" property.
    void createFlows(const QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& captureStats,
            QList<CommunicationFlow>& result);

    // Sort the list of processes according to this watcher's "OS process sort asending" property.
    // If true, processes are sorted oldest-to-newest. Else, newest-to-oldest. Returns a reference
    // to the argument.
    void sortProcesses(QList<OsProcess>& osProcesses);

    // Shared initialization logic.
    void init();

    // Returns true if the start time of the first process is less than the start time of the second
    // In case of a tie, returns true if the PID of the first process is less than the PID of the second.
    // Else, returns false.
    static bool processLessThan(const OsProcess& proc1, const OsProcess& proc2);

    // Returns true if the start time of the first process is greater than the start time of the second
    // In case of a tie, returns true if the PID of the first process is greater than the PID of the second.
    // Else, returns false.
    static bool processGreaterThan(const OsProcess& proc1, const OsProcess& proc2);

    // Interval between wake-up times when the watcher performs its duties.
    static const int DEFAULT_TIMER_INTERVAL_MS;
    const int _timerIntervalMs;

    // Interval between update signals.
    static const int DEFAULT_UPDATE_INTERVAL_MS;
    const int _updateIntervalMs;

    // Default interval between attempts to map OS connections to processes.
    static const int DEFAULT_CORRELATION_INTERVAL_MS;
    const int _correlationIntervalMs;

    // Time we last emitted a signal with a traffic update (or error) for devices.
    qlonglong _lastUpdateMs;

    // Time we last correlated OS connections with processes.
    qlonglong _lastCorrelationMs;

    // The most recent correlation of OS connections and processes.
    QHash<IpEndpointPair, QList<OsProcess> > _connectionProcesses;

    // Correlates the current OS connections and processes.
    IConnectionProcessCorrelator* _correlator;

    // Manages packet capture threads for devices.
    IPcapManager* _pcapManager;

    // Performs name lookups by host address.
    HostNameResolver _hostNameResolver;

    // True if this watcher should do network name resolution. This may be expensive.
    bool _resolveNames;

    // If true, each list of OS processes sharing an IP endpoint pair socket is sorted in ascending order by start
    // time and PID. If false, the order is reversed.
    bool _osProcessSortAscending;

};

#endif /* WATCHER_H_ */
