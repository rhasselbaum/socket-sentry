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

#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtNetwork/QHostInfo>

// Default maximum number of cached addresses.
const uint HostNameResolver::DEFAULT_MAX_SIZE = 500;

// Default age in seconds of a cache entry before it is eligible for eviction.
const uint HostNameResolver::DEFAULT_MAX_AGE_SECS = 60 * 60 * 3;    // 3 hours

HostNameResolver::HostNameResolver() :
    _cache(DEFAULT_MAX_SIZE, DEFAULT_MAX_AGE_SECS) {
    connect(&_cache, SIGNAL(evicted(const QSet<QString>&)), this, SLOT(cacheEntriesEvicted(const QSet<QString>&)));
}

HostNameResolver::HostNameResolver(uint maxSize, uint maxAgeSecs, uint timerIntervalMs, uint retentionPercent) :
    _cache(maxSize, maxAgeSecs, timerIntervalMs, retentionPercent) {
    connect(&_cache, SIGNAL(evicted(const QSet<QString>&)), this, SLOT(cacheEntriesEvicted(const QSet<QString>&)));
}

HostNameResolver::~HostNameResolver() {
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
                _cache.passiveUpdate(hostAddress, hostName);
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
    if (_cache.contains(hostAddress)) {
        // Cache hit.
        return _cache.value(hostAddress).toString();
    } else {
        // Cache miss. We'll look it up asynchronously and add it to the cache so we don't try to look it up more
        // than once.
        QString empty;  // default name
        _cache.insertNew(hostAddress, empty);
        // Now schedule address for resolution and store off the lookup ID.
        int lookupId = resolveAsync(hostAddress);
        _addressByLookupId.insert(lookupId, hostAddress);
        _lookupIdByAddress.insert(hostAddress, lookupId);
        return empty;                   // always return empty here
    }
}

QString HostNameResolver::softResolve(const QString& hostAddress) const {
    if (_cache.contains(hostAddress)) {
        // Cache hit.
        return _cache.value(hostAddress).toString();
    } else {
        return "";
    }
}

void HostNameResolver::cacheEntriesEvicted(const QSet<QString>& addresses) {
    foreach (const QString& hostAddress, addresses) {
        if (_lookupIdByAddress.contains(hostAddress)) {
            int lookupId = _lookupIdByAddress[hostAddress];
            _lookupIdByAddress.remove(hostAddress);
            _addressByLookupId.remove(lookupId);
        }
    }
}
