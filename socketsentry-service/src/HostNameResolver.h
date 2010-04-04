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

#ifndef HOSTNAMERESOLVER_H_
#define HOSTNAMERESOLVER_H_

#include "TimeLimitedCache.h"

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QMultiMap>
#include <QtCore/QHash>

class QHostInfo;
template <class E> class QSet;

/*
 * Performs asynchronous lookups of hostnames by IP address and caches the results for a period of time. Entries remain
 * in the cache until they grow too old or the maximum size of the cache is reached. Entries are evicted from the cache
 * automatically, oldest to newest.
 */
class HostNameResolver : public QObject {
    Q_OBJECT

public:
    HostNameResolver();
    virtual ~HostNameResolver();

    // Resolve the host name for the given IP address. If the name is in the cache, it is returned. Else, an empty string
    // is returned and the address is queued for asynchronous name resolution so that it will (hopefully) be in the cache
    // next time. (This method never blocks.)
    QString resolve(const QString& hostAddress);

    // Resolve the host name for the given IP address if it is in the cache, but do NOT schedule it to be resolved if it's not.
    QString softResolve(const QString& hostAddress) const;

    // Get the current number of entries in the cache.
    int getCacheSize() const { return _cache.size(); }

    // Get the current number of asynchronous lookups in progress.
    int getPendingLookups() const { return _addressByLookupId.size(); }

public slots:
    // Invoked when an async host name lookup has completed. Updates the cache entry (if any) with the resoled host
    // name (if any).
    void lookedUp(const QHostInfo& hostInfo);

protected:
    // New instance with the given maximum cache size, maximum age of entries, timer interval, and retention percent values.
    HostNameResolver(uint maxSize, uint maxAgeSecs, uint timerIntervalMs, uint retentionPercent);

    // Resolve host name asynchronously. The Qt lookup ID is returned, which may be used to match the response when it
    // arrives. This method can be overridden in unit test subclasses to mock out the actual host name lookup.
    virtual int resolveAsync(const QString& hostAddress);

private slots:
    // Invoked when IP address-name pairs are evicted from the cache.
    void cacheEntriesEvicted(const QSet<QString>& addresses);

private:
    // Default maximum number of cached addresses.
    static const uint DEFAULT_MAX_SIZE;

    // Default age in seconds of a cache entry before it is eligible for eviction.
    static const uint DEFAULT_MAX_AGE_SECS;

    // Cache of IP addresses to resoled host names.
    TimeLimitedCache _cache;

    // Table of QHostInfo lookup IDs to IP address.
    QHash<int, QString> _addressByLookupId;

    // Table of IP addresses to QHostInfo lookup IDs.
    QHash<QString, int> _lookupIdByAddress;

};

#endif /* HOSTNAMERESOLVER_H_ */
