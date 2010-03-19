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

#ifndef FLOWSTATISTICS_H_
#define FLOWSTATISTICS_H_

#include "FlowStatisticsData.h"

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

class QDBusArgument;

/*
 * Statistics derived from observed metrics of a connection or group of connections.
 */
class FlowStatistics {
public:
    FlowStatistics();
    FlowStatistics(qlonglong recentBytesInPerSec, qlonglong recentBytesOutPerSec, qlonglong peakBytesPerSec,
            bool sendingNow, bool receivingNow);
    bool operator==(const FlowStatistics& rhs) const;
    virtual ~FlowStatistics();

    bool isReceivingNow() const { return _d->receivingNow; }
    bool isSendingNow() const { return _d->sendingNow; }
    qlonglong getPeakBytesPerSec() const { return _d->peakBytesPerSec; }
    qlonglong getRecentBytesInPerSec() const { return _d->recentBytesInPerSec; }
    qlonglong getRecentBytesOutPerSec() const { return _d->recentBytesOutPerSec; }
    qlonglong getRecentBytesPerSec() const { return _d->recentBytesInPerSec + _d->recentBytesOutPerSec; }

    void setPeakBytesPerSec(qlonglong peakBytesPerSec) { _d->peakBytesPerSec = peakBytesPerSec; }
    void setRecentBytesInPerSec(qlonglong recentBytesInPerSec) { _d->recentBytesInPerSec = recentBytesInPerSec; }
    void setRecentBytesOutPerSec(qlonglong recentBytesOutPerSec) { _d->recentBytesOutPerSec = recentBytesOutPerSec; }
    void setReceivingNow(bool receivingNow) { _d->receivingNow = receivingNow; }
    void setSendingNow(bool sendingNow) { _d->sendingNow = sendingNow; }

    // Combine statistics from one or more additional connections into this object.
    void combineConnections(const FlowStatistics& other);

    // Create a string of this flow statistics object suitable for debugging, but not end-user consumption.
    virtual QString toString() const;

    // Reset the device state flags ("sending now" and "receiving now") to false.
    void quietDevice();

private:
    QSharedDataPointer<FlowStatisticsData> _d;
};

// DBUS argument marshalling.
QDBusArgument& operator<<(QDBusArgument& argument, const FlowStatistics& obj);
// DBUS argument unmarshalling.
const QDBusArgument& operator>>(const QDBusArgument& argument, FlowStatistics& obj);

// Make visible as a D-Bus data type.
Q_DECLARE_METATYPE(FlowStatistics)

#endif /* FLOWSTATISTICS_H_ */
