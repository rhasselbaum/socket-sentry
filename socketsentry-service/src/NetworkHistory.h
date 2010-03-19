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

#ifndef NETWORKHISTORY_H_
#define NETWORKHISTORY_H_

#include <QtCore/QVector>

class FlowMetrics;
class FlowStatistics;
template <class K, class V> class QHash;
class IpEndpointPair;
template <class F, class S> class QPair;

/*
 * A rolling history of network activity observed on a packet capture device.
 */
class NetworkHistory {
public:

    // New instance.
    NetworkHistory();

    // New instance with specific historical ranges. Only values >= 2 are valid.
    NetworkHistory(const int historySecs, const int recentHistorySecs);

    // Dtor.
    virtual ~NetworkHistory();

    // Record new metrics describing a flow of data between two endpoints observed at a particular point in
    // time. The historical range is rolled forward to the specified time (if necessary) and the metrics
    // are combined with any existing metrics for the same endpoint pair at that time. If there were no metrics
    // for the pair at that time, the given metrics are stored as-is. If the sample time is earlier than the
    // earliest point in the historical range, the metrics are silently discarded as being too old.
    void record(const IpEndpointPair& flow, const FlowMetrics& newMetrics, const time_t& sampleTime);

    // Export statics based on current history. First, the historical range is rolled forward to the given
    // end time (if necessary). Then, the history is consolidated into one set of flow metrics and statistics
    // per IP endpoint pair. The statistics hashtable should be empty when this method is called. It will be
    // filled upon return.
    void exportStatistics(QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& result, const time_t& endTime);

    // Check to see if traffic has been recorded since the specified time.
    bool anyTrafficSince(time_t timeSecs) const;

private:
    // The size of the historical range in seconds. Must be >= 2 because the "current" second is not considered
    // in statistics generation.
    static const int DEFAULT_HISTORY_SECS;

    // Number of seconds of recent history to use when calculating real-time statistics (e.g. transfer rate).
    // The value must be in the range 2 <= RECENT_HISTORY_SECS <= HISTORY_SECS.
    static const int DEFAULT_RECENT_HISTORY_SECS;

    // Roll the historical range forward (if necessary) to include the specified time. If the specified time
    // is before the current end of the historical range, then nothing happens.
    void rollForward(const time_t& rollTo);

    // Get the index of a time in the circular buffer.
    int indexOf(const time_t& pos) { return pos % _historySecs; }

    // The size of the historical range in seconds. Must be >= 2 because the "current" second is not considered
    // in statistics generation.
    const int _historySecs;

    // Number of seconds of recent history to use when calculating real-time statistics (e.g. transfer rate).
    // The value must be in the range 2 <= RECENT_HISTORY_SECS <= HISTORY_SECS.
    const int _recentHistorySecs;

    // Upper bound of the historical time range.
    time_t _lastRoll;

    // Time of the most recent traffic recording.
    time_t _lastRecording;

    // The circular buffer to which history is recorded. Each element holds a second's worth of network activity
    // in wall clock time. The hashtable maps IP endpoint pairs to observed activity between those endpoints in
    // one second.
    QVector<QHash<IpEndpointPair, FlowMetrics> > _circularBuf;
};

#endif /* NETWORKHISTORY_H_ */
