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

#include "TimeLimitedCache.h"
#include "DateTimeUtils.h"

#include <QtCore/QString>
#include <QtCore/QMutableMapIterator>
#include <QtCore/QSet>

// Default interval between time-based eviction sweeps.
const uint TimeLimitedCache::DEFAULT_TIMER_INTERVAL_MS = 300000;   // 5 min

// Default percentage of entries to retain when reducing cache size after it hits the maximum.
// Value must be between 0 and 99.
const uint TimeLimitedCache::DEFAULT_RETENTION_PERCENT = 80;

TimeLimitedCache::TimeLimitedCache(uint maxSize, uint maxAgeSecs, uint timerIntervalMs, uint retentionPercent) :
    _maxSize(maxSize), _maxAgeSecs(maxAgeSecs), _retentionPercent(retentionPercent) {
    startTimer(timerIntervalMs);
}

TimeLimitedCache::~TimeLimitedCache() {
}

void TimeLimitedCache::timerEvent(QTimerEvent* event) {
    qlonglong nowMs = DateTimeUtils::currentTimeMs();
    // Look for cache entries that are older than the maximum allowed age and remove them from the cache.
    // The map is ordered by request time, so as soon as we see an entry that's not old, we can stop.
    QMutableMapIterator<qlonglong, QString> iter(_keysByRequestTimeMs);
    QSet<QString> evictedKeys;
    while (iter.hasNext()) {
        iter.next();
        qlonglong requestTimeMs = iter.key();
        if (requestTimeMs + (_maxAgeSecs * 1000) <= nowMs) {
            // Entry is too old.
            const QString& key = iter.value();
            _lookupTable.remove(key);
            evictedKeys << key;
            iter.remove();
        } else {
            // Remaining entries are new enough to avoid eviction.
            break;
        }
    }
    if (!evictedKeys.isEmpty()) emit evicted(evictedKeys);
}

void TimeLimitedCache::passiveUpdate(const QString& key, const QVariant& value) {
    if (_lookupTable.contains(key)) {
        _lookupTable.insert(key, value);
    }
}

bool TimeLimitedCache::insertNew(const QString& key, const QVariant& value) {
    bool exists = _lookupTable.contains(key);
    if (!exists) {
        // Cache miss. We'll add it now.
        if (_keysByRequestTimeMs.size() >= _maxSize) {
            // Cache full. Cut it down.
            reduceSize();
        }
        // Create entries in the name table and address by request time map.
        qlonglong nowMs = DateTimeUtils::currentTimeMs();
        _lookupTable.insert(key, value);
        _keysByRequestTimeMs.insert(nowMs, key);
    }
    return !exists;
}

void TimeLimitedCache::reduceSize() {
    Q_ASSERT(_retentionPercent >= 0 && _retentionPercent < 100);
    const uint newSize = _maxSize * _retentionPercent / 100;
    QMutableMapIterator<qlonglong, QString> iter(_keysByRequestTimeMs);
    QSet<QString> evictedKeys;
    while(iter.hasNext() && _lookupTable.size() > newSize) {
        iter.next();
        const QString& key = iter.value();
        _lookupTable.remove(key);
        evictedKeys << key;
        iter.remove();
    }
    if (!evictedKeys.isEmpty()) emit evicted(evictedKeys);
}

