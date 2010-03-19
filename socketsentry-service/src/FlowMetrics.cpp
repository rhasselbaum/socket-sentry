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

#include "FlowMetrics.h"

#include <QtDBus/QDBusArgument>

FlowMetrics::FlowMetrics() {
    _d = new FlowMetricsData();
}

FlowMetrics::FlowMetrics(qlonglong bytesIn, qlonglong bytesOut, qlonglong packetsIn, qlonglong packetsOut) {
    _d = new FlowMetricsData();
    _d->bytesIn = bytesIn;
    _d->bytesOut = bytesOut;
    _d->packetsIn = packetsIn;
    _d->packetsOut = packetsOut;
}

bool FlowMetrics::operator==(const FlowMetrics& rhs) const {
    if (_d == rhs._d)
        return true;
    return _d->bytesIn == rhs.getBytesIn()
            && _d->bytesOut == rhs.getBytesOut()
            && _d->packetsIn == rhs.getPacketsIn()
            && _d->packetsOut == rhs.getPacketsOut();
}

FlowMetrics::~FlowMetrics() {
}

QString FlowMetrics::toString() const {
    return QString("ByIn: %1 ByOut: %2 PackIn: %3 PackOut: %4")
            .arg(_d->bytesIn)
            .arg(_d->bytesOut)
            .arg(_d->packetsIn)
            .arg(_d->packetsOut);
}

void FlowMetrics::combineConnections(const FlowMetrics& other) {
    _d->bytesIn += other.getBytesIn();
    _d->bytesOut += other.getBytesOut();
    _d->packetsIn += other.getPacketsIn();
    _d->packetsOut += other.getPacketsOut();
}

void FlowMetrics::combineTimeIntervals(const FlowMetrics& other) {
    _d->bytesIn += other.getBytesIn();
    _d->bytesOut += other.getBytesOut();
    _d->packetsIn += other.getPacketsIn();
    _d->packetsOut += other.getPacketsOut();
}

QDBusArgument& operator<<(QDBusArgument& arg, const FlowMetrics& obj) {
    arg.beginStructure();
    arg << obj.getBytesIn() << obj.getBytesOut() << obj.getPacketsIn() << obj.getPacketsOut();
    arg.endStructure();
    return arg;
}

const QDBusArgument& operator>>(const QDBusArgument& arg, FlowMetrics& obj) {
    arg.beginStructure();

    qlonglong bytesIn;
    arg >> bytesIn;
    obj.setBytesIn(bytesIn);

    qlonglong bytesOut;
    arg >> bytesOut;
    obj.setBytesOut(bytesOut);

    qlonglong packetsIn;
    arg >> packetsIn;
    obj.setPacketsIn(packetsIn);

    qlonglong packetsOut;
    arg >> packetsOut;
    obj.setPacketsOut(packetsOut);

    arg.endStructure();
    return arg;
}
