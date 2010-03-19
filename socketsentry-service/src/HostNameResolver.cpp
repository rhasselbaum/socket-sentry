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

#include "HostNameResolver.h"
#include "DateTimeUtils.h"

#include <QtCore/QTimerEvent>
#include <QtCore/QMutableMapIterator>
#include <QtNetwork/QHostInfo>
#include <QtCore/QListIterator>


// Default maximum number of cached addresses.
const uint HostNameResolver::DEFAULT_MAX_SIZE = 500;

// Default age in seconds of a cache entry before it is eligible for eviction.
const uint HostNameResolver::DEFAULT_MAX_AGE_SECS = 60 * 60 * 3;    // 3 hours

// Default interval between time-based eviction sweeps.
const uint HostNameResolver::DEFAULT_TIMER_INTERVAL_MS = 300000;   // 5 min

// Default percentage of entries to retain when reducing cache size after it hits the maximum.
// Value must be between 0 and 99.
const uint HostNameResolver::DEFAULT_RETENTION_PERCENT = 80;

HostNameResolver::HostNameResolver() :
    _maxSize(DEFAULT_MAX_SIZE), _maxAgeSecs(DEFAULT_MAX_AGE_SECS), _timerIntervalMs(DEFAULT_TIMER_INTERVAL_MS),
    _retentionPercent(DEFAULT_RETENTION_PERCENT) {
    startTimer(_timerIntervalMs);
}

HostNameResolver::HostNameResolver(uint maxSize, uint maxAgeSecs, uint timerIntervalMs, uint retentionPercent) :
    _maxSize(maxSize), _maxAgeSecs(maxAgeSecs), _timerIntervalMs(timerIntervalMs), _retentionPercent(retentionPercent) {
    startTimer(_timerIntervalMs);
}

HostNameResolver::~HostNameResolver() {
}

void HostNameResolver::timerEvent(QTimerEvent* event) {
    qlonglong nowMs = DateTimeUtils::currentTimeMs();
    // Look for cache entries that are older than the maximum allowed age and remove them from this resolver.
    // The map is ordered by request time, so as soon as we see an entry that's not old, we can stop.
    QMutableMapIterator<qlonglong, QString> iter(_addressesByRequestTimeMs);
    while (iter.hasNext()) {
        iter.next();
        qlonglong requestTimeMs = iter.key();
        if (requestTimeMs + (_maxAgeSecs * 1000) <= nowMs) {
            // Entry is too old.
            evictCurrent(iter);
        } else {
            // Remaining entries are new enough to avoid eviction.
            break;
        }
    }
}

void HostNameResolver::lookedUp(const QHostInfo& hostInfo) {
    int lookupId = hostInfo.lookupId();
    if (_addressByLookupId.contains(lookupId)) {
        const QString& hostAddress = _addressByLookupId[lookupId];
        if (hostInfo.error() == QHostInfo::NoError) {
            // Sometimes, a "successful" lookup produces a host name equal to IP address. No thanks.
            QString hostName = hostInfo.hostName();
            if (hostAddress != hostName) {
                // Update the name table.
                _nameTable.insert(hostAddress, hostName);
            }
        }
        // Remove lookup ID from hash tables b/c we're done with it now.
        _lookupIdByAddress.remove(hostAddress);
        _addressByLookupId.remove(lookupId);
    } // Else, the entry was evicted before we finished looking it up. Weird.
}

int HostNameResolver::resolveAsync(const QString& hostAddress) {
    return QHostInfo::lookupHost(hostAddress, this, SLOT(lookedUp(QHostInfo)));
}

QString HostNameResolver::resolve(const QString& hostAddress) {
    if (_nameTable.contains(hostAddress)) {
        // Cache hit.
        return _nameTable[hostAddress];
    } else {
        // Cache miss. We'll add it now.
        if (_addressesByRequestTimeMs.size() >= _maxSize) {
            // Cache full. Cut it down.
            reduceSize();
        }
        // Create entries in the name table and address by request time map.
        qlonglong nowMs = DateTimeUtils::currentTimeMs();
        QString empty = _nameTable[hostAddress];    // adds new entry
        _addressesByRequestTimeMs.insert(nowMs, hostAddress);
        // Now schedule address for resolution and store off the lookup ID.
        int lookupId = resolveAsync(hostAddress);
        _addressByLookupId.insert(lookupId, hostAddress);
        _lookupIdByAddress.insert(hostAddress, lookupId);
        return empty;                   // always return empty here
    }
}

QString HostNameResolver::softResolve(const QString& hostAddress) const {
    if (_nameTable.contains(hostAddress)) {
        // Cache hit.
        return _nameTable[hostAddress];
    } else {
        return "";
    }
}

void HostNameResolver::reduceSize() {
    Q_ASSERT(_retentionPercent >= 0 && _retentionPercent < 100);
    const uint newSize = _maxSize * _retentionPercent / 100;
    QMutableMapIterator<qlonglong, QString> iter(_addressesByRequestTimeMs);
    while(iter.hasNext() && _nameTable.size() > newSize) {
        iter.next();
        evictCurrent(iter);
    }
}

void HostNameResolver::evictCurrent(QMutableMapIterator<qlonglong, QString>& iter) {
    QString hostAddress = iter.value();
    _nameTable.remove(hostAddress);
    if (_lookupIdByAddress.contains(hostAddress)) {
        int lookupId = _lookupIdByAddress[hostAddress];
        _lookupIdByAddress.remove(hostAddress);
        _addressByLookupId.remove(lookupId);
    }
    iter.remove();
}
