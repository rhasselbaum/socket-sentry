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

#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QListIterator>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDebug>

#include "Watcher.h"
#include "OsProcess.h"
#include "IpEndpointPair.h"
#include "FlowMetrics.h"
#include "FlowStatistics.h"
#include "CommunicationFlow.h"
#include "ConnectionProcessCorrelator.h"
#include "DateTimeUtils.h"
#include "PcapManager.h"


// Default interval between wake-up times when the watcher performs its duties.
const int Watcher::DEFAULT_TIMER_INTERVAL_MS = 500;

// Default interval between attempts to map captured packet endpoints to OS processes and connections.
const int Watcher::DEFAULT_CORRELATION_INTERVAL_MS = 2000;

// Default interval between update signals.
const int Watcher::DEFAULT_UPDATE_INTERVAL_MS = 1000;

Watcher::Watcher() :
    _correlator(new ConnectionProcessCorrelator), _pcapManager(new PcapManager),
    _timerIntervalMs(DEFAULT_TIMER_INTERVAL_MS), _correlationIntervalMs(DEFAULT_CORRELATION_INTERVAL_MS ),
    _updateIntervalMs(DEFAULT_UPDATE_INTERVAL_MS), _resolveNames(false), _osProcessSortAscending(true) {
    init();
}

Watcher::Watcher(IConnectionProcessCorrelator* correlator, IPcapManager* pcapManager, int timerIntervalMs,
        int updateIntervalMs, int correlationIntervalMs) :
    _correlator(correlator), _pcapManager(pcapManager), _timerIntervalMs(timerIntervalMs),
    _updateIntervalMs(updateIntervalMs), _correlationIntervalMs(correlationIntervalMs), _resolveNames(false) {
    init();
}

void Watcher::init() {
    startTimer(_timerIntervalMs);
    _lastUpdateMs = DateTimeUtils::currentTimeMs();
    _lastCorrelationMs = _lastUpdateMs;
}

Watcher::~Watcher() {
    delete _pcapManager;
    _pcapManager = NULL;
    delete _correlator;
    _correlator = NULL;
}

void Watcher::showInterest(const QString& device) {
    _pcapManager->showInterest(device);
}

void Watcher::timerEvent(QTimerEvent* event) {
    qlonglong currTime = DateTimeUtils::currentTimeMs();
    QString correlationError;
    bool correlated = true;
    // Querying the OS for connections and processes is expensive, so we try to minimize it. Only update
    // connection processes if the correlation interval has passed AND there has been some captured traffic.
    if (_lastCorrelationMs + _correlationIntervalMs <= currTime && _pcapManager->anyTrafficSince(_lastCorrelationMs / 1000)) {
        // Do OS connection and process correlation.
        _connectionProcesses.clear();
        correlated = _correlator->correlate(_connectionProcesses, correlationError);
        _lastCorrelationMs = currTime;
    }
    if (!correlated) {
        // Connection-process correlation failed. Cancel all packet captures.
        QStringList devices = _pcapManager->findCurrentDevices();
        if (!devices.isEmpty()) {
            QListIterator<QString> i(devices);
            while (i.hasNext()) {
                const QString& device = i.next();
                emit failure(device, correlationError);
            }
            _pcapManager->releaseAll();
        }
    } else if (_lastUpdateMs + _updateIntervalMs <= currTime) {
        QStringList devices = _pcapManager->findCurrentDevices();
        QListIterator<QString> i(devices);
        while (i.hasNext()) {
            // Get capture statistics for this device and emit signal.
            const QString& device = i.next();
            QString captureError;
            QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> > captureStats;
            bool ok = _pcapManager->fillStatistics(device, captureStats, captureError);
            if (!ok) {
                // Encountered an error. Let listeners know.
                emit failure(device, captureError);
            } else {
                // Send update to listeners.
                QList<CommunicationFlow> flows;
                createFlows(captureStats, flows);
                emit update(device, flows);
            }
            // If the capture encountered an error or expired, release it so we won't consider it next time.
            if (!_pcapManager->isActive(device)) {
                _pcapManager->release(device);
            }
        }
        _lastUpdateMs = currTime;
    }
}

QStringList Watcher::findDevices(QString& error) const {
    return _pcapManager->findAllDevices(error);
}

void Watcher::createFlows(const QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& captureStats,
        QList<CommunicationFlow>& result) {

    if (!captureStats.isEmpty() && !_connectionProcesses.isEmpty()) {
        // Iterate over the smaller of the two hashes.
        QListIterator<IpEndpointPair>* iter = NULL;
        if (captureStats.size() > _connectionProcesses.size()) {
            iter = new QListIterator<IpEndpointPair>(_connectionProcesses.uniqueKeys());
        } else {
            iter = new QListIterator<IpEndpointPair>(captureStats.uniqueKeys());
        }
        while (iter->hasNext()) {
            const IpEndpointPair& ipEndpointPair = iter->next();
            if (_connectionProcesses.contains(ipEndpointPair) && captureStats.contains(ipEndpointPair)) {
                // Endpoints appear in the kernel connection table and the packet capture. Add to result.
                const QPair<FlowMetrics, FlowStatistics>& numbers = captureStats[ipEndpointPair];
                IpEndpointPair flowEndpoints = ipEndpointPair;
                if (_resolveNames) {
                    QString hostName = _hostNameResolver.resolve(flowEndpoints.getRemoteAddr().toString());
                    flowEndpoints.setRemoteHostName(hostName);
                }
                QList<OsProcess> osProcesses = _connectionProcesses[ipEndpointPair];
                sortProcesses(osProcesses);
                CommunicationFlow flow(flowEndpoints, osProcesses, numbers.first, numbers.second);
                result.append(flow);
            }
        }
        delete iter;
        iter = NULL;
    }

}

void Watcher::sortProcesses(QList<OsProcess>& osProcesses) {
    // Sort processes.
    if (_osProcessSortAscending) {
        qSort(osProcesses.begin(), osProcesses.end(), Watcher::processLessThan);
    } else {
        qSort(osProcesses.begin(), osProcesses.end(), Watcher::processGreaterThan);
    }
}

bool Watcher::processLessThan(const OsProcess& proc1, const OsProcess& proc2) {
    if (proc1.getStartTime() == proc2.getStartTime()) {
        return proc1.getPid() < proc2.getPid();
    } else {
        return proc1.getStartTime() < proc2.getStartTime();
    }
}

bool Watcher::processGreaterThan(const OsProcess& proc1, const OsProcess& proc2) {
    if (proc1.getStartTime() == proc2.getStartTime()) {
        return proc1.getPid() > proc2.getPid();
    } else {
        return proc1.getStartTime() > proc2.getStartTime();
    }
}
