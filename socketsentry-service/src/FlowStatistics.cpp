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

#include "FlowStatistics.h"

#include <QtDBus/QDBusArgument>

FlowStatistics::FlowStatistics() {
    _d = new FlowStatisticsData();
}

FlowStatistics::FlowStatistics(qlonglong recentBytesInPerSec, qlonglong recentBytesOutPerSec, qlonglong peakBytesPerSec,
        bool sendingNow, bool receivingNow) {
    _d = new FlowStatisticsData();
    _d->recentBytesInPerSec = recentBytesInPerSec;
    _d->recentBytesOutPerSec = recentBytesOutPerSec;
    _d->peakBytesPerSec = peakBytesPerSec;
    _d->sendingNow = sendingNow;
    _d->receivingNow = receivingNow;

}

FlowStatistics::~FlowStatistics() {
}

bool FlowStatistics::operator==(const FlowStatistics& rhs) const {
    if(_d == rhs._d)
        return true;
    return _d->recentBytesInPerSec == rhs.getRecentBytesInPerSec()
            && _d->recentBytesOutPerSec == rhs.getRecentBytesOutPerSec()
            && _d->peakBytesPerSec == rhs.getPeakBytesPerSec()
            && _d->receivingNow == rhs.isReceivingNow()
            && _d->sendingNow == rhs.isSendingNow();
}


QString FlowStatistics::toString() const {
    return QString("BpsIn: %1 BpsOut: %2 BpsTot: %3 PeakBps: %4 Rec: %5 Send: %6")
            .arg(_d->recentBytesInPerSec)
            .arg(_d->recentBytesOutPerSec)
            .arg(getRecentBytesPerSec())
            .arg(_d->peakBytesPerSec)
            .arg(_d->receivingNow)
            .arg(_d->sendingNow);
}

void FlowStatistics::quietDevice() {
    _d->receivingNow = false;
    _d->sendingNow = false;
}

void FlowStatistics::combineConnections(const FlowStatistics& other) {
    _d->recentBytesInPerSec += other.getRecentBytesInPerSec();
    _d->recentBytesOutPerSec += other.getRecentBytesOutPerSec();
    _d->peakBytesPerSec += other.getPeakBytesPerSec();  // add, not max
    if (other.isReceivingNow()) {
        _d->receivingNow = true;
    }
    if (other.isSendingNow()) {
        _d->sendingNow = true;
    }
}

QDBusArgument& operator<<(QDBusArgument& arg, const FlowStatistics& obj) {
    arg.beginStructure();
    arg << obj.getRecentBytesInPerSec() << obj.getRecentBytesOutPerSec() << obj.getPeakBytesPerSec()
            << obj.isReceivingNow() << obj.isSendingNow();
    arg.endStructure();
    return arg;
}

const QDBusArgument& operator>>(const QDBusArgument& arg, FlowStatistics& obj) {
    arg.beginStructure();

    qlonglong recentBytesInPerSec;
    arg >> recentBytesInPerSec;
    obj.setRecentBytesInPerSec(recentBytesInPerSec);

    qlonglong recentBytesOutPerSec;
    arg >> recentBytesOutPerSec;
    obj.setRecentBytesOutPerSec(recentBytesOutPerSec);

    qlonglong peakBytesPerSec;
    arg >> peakBytesPerSec;
    obj.setPeakBytesPerSec(peakBytesPerSec);

    bool receivingNow;
    arg >> receivingNow;
    obj.setReceivingNow(receivingNow);

    bool sendingNow;
    arg >> sendingNow;
    obj.setSendingNow(sendingNow);

    arg.endStructure();
    return arg;
}
