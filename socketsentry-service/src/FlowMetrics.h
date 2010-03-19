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

#ifndef FLOWMETRICS_H_
#define FLOWMETRICS_H_

#include "FlowMetricsData.h"

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

class QDBusArgument;

/*
 * Observed metrics of a network connection or group of related connections.
 */
class FlowMetrics {
public:
    FlowMetrics();
    FlowMetrics(qlonglong bytesIn, qlonglong bytesOut, qlonglong packetsIn, qlonglong packetsOut);
    bool operator==(const FlowMetrics& rhs) const;
    virtual ~FlowMetrics();

    qlonglong getBytesIn() const { return _d->bytesIn; }
    qlonglong getBytesOut() const { return _d->bytesOut; }
    qlonglong getPacketsIn() const { return _d->packetsIn; }
    qlonglong getPacketsOut() const { return _d->packetsOut; }

    void setBytesIn(qlonglong bytesIn) { _d->bytesIn = bytesIn; }
    void setBytesOut(qlonglong bytesOut) { _d->bytesOut = bytesOut; }
    void setPacketsIn(qlonglong packetsIn) { _d->packetsIn = packetsIn; }
    void setPacketsOut(qlonglong packetsOut) { _d->packetsOut = packetsOut; }

    qlonglong getTotalBytes() const { return _d->bytesIn + _d->bytesOut; }
    qlonglong getTotalPackets() const { return _d->packetsIn + _d->packetsOut; }

    // Combine metrics from one or more additional connections into this object.
    void combineConnections(const FlowMetrics& other);

    // Combine metrics from another time interval into this object.
    void combineTimeIntervals(const FlowMetrics& other);

    // Return a string for this object suitable for debugging, but not end-user consumption.
    virtual QString toString() const;

private:
    QSharedDataPointer<FlowMetricsData> _d;
};

// DBUS argument marshalling.
QDBusArgument& operator<<(QDBusArgument& argument, const FlowMetrics& obj);
// DBUS argument unmarshalling.
const QDBusArgument& operator>>(const QDBusArgument& argument, FlowMetrics& obj);

// Make visible as a D-Bus data type.
Q_DECLARE_METATYPE(FlowMetrics)

#endif /* FLOWMETRICS_H_ */
