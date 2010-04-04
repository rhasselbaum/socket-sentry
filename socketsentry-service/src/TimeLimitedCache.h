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

#ifndef TIMELIMITEDCACHE_H_
#define TIMELIMITEDCACHE_H_

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QMultiMap>
#include <QtCore/QVariant>

template <class E> class QSet;
class QString;

/*
 * A cache that stores a limited number of key-value pairs for a limited period of time. Entries remain in the cache
 * until they grow too old or the maximum size of the cache is reached. Entries are evicted from the cache
 * automatically, oldest to newest. Keys must be represented as QStrings and values as QVariants, since Qt does
 * not allow template classes to derive from QObject.
 */
class TimeLimitedCache : public QObject {
    Q_OBJECT

public:
    TimeLimitedCache(uint maxSize, uint maxAgeSecs, uint timerIntervalMs = DEFAULT_TIMER_INTERVAL_MS,
            uint retentionPercent = DEFAULT_RETENTION_PERCENT);
    virtual ~TimeLimitedCache();

    // Update an existing entry in the cache without resetting the entry time. If the key does not exist, nothing
    // gets updated.
    void passiveUpdate(const QString& key, const QVariant& value);

    // Insert a new key-value pair into the cache, evicting old entries if the cache grows beyond the maximum size.
    // If the key is already in the cache, no change is made. The method returns true if the entry was added, false
    // if the key was already in the cache.
    bool insertNew(const QString& key, const QVariant& value);

    // Return the current number of entries in the cache.
    int size() const { return _lookupTable.size(); }

    // Returns true if the key is contained within this cache.
    bool contains(const QString& key) const { return _lookupTable.contains(key); }

    // Returns the value for the given key. If the key does not exist in the cache, a default constructed value is
    // returned.
    const QVariant value(const QString& key) const { return _lookupTable.value(key); }

signals:
    // Emitted when one or more entries are evicted from the cache.
    void evicted(const QSet<QString>& keys);

protected:
    // Timer event for time-based eviction sweeps.
    void timerEvent(QTimerEvent* event);

private:
    // Reduce the size of the cache to "retention %" of maximum. Oldest entries are removed first.
    void reduceSize();

    // Default interval between time-based eviction sweeps.
    static const uint DEFAULT_TIMER_INTERVAL_MS;

    // Default percentage of entries to retain when reducing cache size after it hits the maximum.
    // Value must be between 0 and 99.
    static const uint DEFAULT_RETENTION_PERCENT;

    // The maximum number of cached addresses.
    const uint _maxSize;

    // The age in seconds of a cache entry before it is eligible for eviction. Should be less than the typical DNS TTL.
    const uint _maxAgeSecs;

    // Percentage of entries to retain when reducing cache size after it hits the maximum. Value must be between 0 and 99.
    // A value of 80 means 80% of entries are retained and 20% are removed when the cache size hits the maximum to make
    // room for new entries.
    const uint _retentionPercent;

    // Table of keys to values.
    QHash<QString, QVariant> _lookupTable;

    // Map of cache entry times to keys.
    QMultiMap<qlonglong, QString> _keysByRequestTimeMs;

};

#endif /* TIMELIMITEDCACHE_H_ */
