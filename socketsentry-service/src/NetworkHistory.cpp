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

#include <QtCore/QHash>
#include <QtCore/QHashIterator>
#include <QtCore/QPair>

#include "NetworkHistory.h"
#include "FlowMetrics.h"
#include "FlowStatistics.h"
#include "IpEndpointPair.h"

const int NetworkHistory::DEFAULT_HISTORY_SECS = 30;
const int NetworkHistory::DEFAULT_RECENT_HISTORY_SECS = DEFAULT_HISTORY_SECS / 3;

NetworkHistory::NetworkHistory():
    _historySecs(DEFAULT_HISTORY_SECS), _recentHistorySecs(DEFAULT_RECENT_HISTORY_SECS),
    _lastRoll(0), _lastRecording(0), _circularBuf(DEFAULT_HISTORY_SECS) {
}

NetworkHistory::NetworkHistory(const int historySecs, const int recentHistorySecs):
    _historySecs(historySecs), _recentHistorySecs(recentHistorySecs),
    _lastRoll(0), _lastRecording(0), _circularBuf(historySecs) {
}


NetworkHistory::~NetworkHistory() {
}

void NetworkHistory::record(const IpEndpointPair& flow, const FlowMetrics& newMetrics, const time_t& sampleTime) {
    if(sampleTime > _lastRoll - _historySecs) {
        // Sample falls within historical range. Accumulate the metrics for this flow at the specified time.
        rollForward(sampleTime);
        QHash<IpEndpointPair, FlowMetrics>& activityAtTime = _circularBuf[indexOf(sampleTime)];
        FlowMetrics& metrics = activityAtTime[flow];
        metrics.combineTimeIntervals(newMetrics);
        if (_lastRecording < sampleTime) {
            _lastRecording = sampleTime;
        }
    } // Else, sample is too old. Skip it.
}

void NetworkHistory::rollForward(const time_t& rollTo) {
    if (_lastRoll < rollTo) {
        if (_lastRoll + _historySecs <= rollTo) {
            // Entire history is obsolete. Reset the buffer.
            for (int i = 0; i < _historySecs; i++) {
                _circularBuf[i].clear();
            }
        } else {
            // Clear history between last roll time (exclusive) and roll-to time (inclusive).
            for(time_t i = _lastRoll + 1; i <= rollTo; i++) {
                _circularBuf[indexOf(i)].clear();
            }
        }
        _lastRoll = rollTo;
    } // Else, history has already advanced to or beyond the specified point, so nothing to do.
}

void NetworkHistory::exportStatistics(
        QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& result,
        const time_t& endTime) {

    result.clear();	// just in case
    rollForward(endTime);

    // Loop over historical range from oldest time (inclusive) to newest time (exclusive).
    for (time_t timePos = _lastRoll - _historySecs + 1; timePos < _lastRoll; timePos++) {
        QHash<IpEndpointPair, FlowMetrics>& flowsAtTime = _circularBuf[indexOf(timePos)];
        QHashIterator<IpEndpointPair, FlowMetrics> iter(flowsAtTime);
        // Loop over all flows for the current time slot.
        while (iter.hasNext()) {
            iter.next();
            const IpEndpointPair& endpoints = iter.key();
            const FlowMetrics& metricsAtTime = iter.value();

            QPair<FlowMetrics, FlowStatistics>& endpointResult = result[endpoints];
            FlowMetrics& rangeMetrics = endpointResult.first;
            FlowStatistics& rangeStats = endpointResult.second;

            // Fold metrics into roll-up.
            rangeMetrics.combineTimeIntervals(metricsAtTime);

            // Update peak rate.
            if (metricsAtTime.getTotalBytes() > rangeStats.getPeakBytesPerSec()) {
                rangeStats.setPeakBytesPerSec(metricsAtTime.getTotalBytes());
            }

            if (timePos > _lastRoll - _recentHistorySecs) {
                // This time slot is "recent history". Add current time slot's contribution
                // to the= transfer rate. We lose some precision doing integer division, but
                // it's not significant.
                qlonglong oldBpsInRate = rangeStats.getRecentBytesInPerSec();
                qlonglong newBytesInContrib = metricsAtTime.getBytesIn() / (_recentHistorySecs - 1);
                rangeStats.setRecentBytesInPerSec(oldBpsInRate + newBytesInContrib);

                qlonglong oldBpsOutRate = rangeStats.getRecentBytesOutPerSec();
                qlonglong newBytesOutContrib = metricsAtTime.getBytesOut() / (_recentHistorySecs - 1);
                rangeStats.setRecentBytesOutPerSec(oldBpsOutRate + newBytesOutContrib);

                if (timePos == _lastRoll - 1) {
                    // This is the last time slot. Set flags to indicate that we
                    // are sending and/or receiving data "now".
                    bool receivingNow = metricsAtTime.getBytesIn() > 0;
                    rangeStats.setReceivingNow(receivingNow);
                    bool sendingNow = metricsAtTime.getBytesOut() > 0;
                    rangeStats.setSendingNow(sendingNow);
                }
            }
        }
    }
}

bool NetworkHistory::anyTrafficSince(time_t timeSecs) const {
    return _lastRecording >= timeSecs;
}
