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

#ifndef USERNAMERESOLVER_H_
#define USERNAMERESOLVER_H_

#include "TimeLimitedCache.h"

#include <QtCore/QHash>

class QString;
template <class V> class QFuture;

/*
 * Performs asynchronous lookups of username by UID and caches the results for a period of time. Entries remain
 * in the cache until they grow too old or the maximum size of the cache is reached. Entries are evicted from the cache
 * automatically, oldest to newest.
 */
class UserNameResolver {
public:
    UserNameResolver();
    virtual ~UserNameResolver();

    // Resolve the user name for the given UID string. If the name is in the cache, it is returned. Else, an empty string
    // is returned and the UID is queued for asynchronous name resolution so that it will (hopefully) be in the cache
    // next time. (This method never blocks.)
    QString resolve(const QString& uid);

protected:
    // Constructor for unit testing.
    UserNameResolver(uint maxSize, uint maxAgeSecs, uint timerIntervalMs, uint retentionPercent);

    // Resolve the user name for the given UID string synchronously, bypassing the cache. This method blocks until the
    // result is obtained. It's an instance method to permit overriding in unit tests, but the default implementation
    // does not access any shared state, so it can run safely in a sepate thread.
    virtual QString resolveNow(const QString& uid);

private:
    // Default maximum number of cached names.
    static const uint DEFAULT_MAX_SIZE;

    // Default age in seconds of a cache entry before it is eligible for eviction.
    static const uint DEFAULT_MAX_AGE_SECS;

    // Cache of UIDs to resolved user names.
    TimeLimitedCache _cache;

    // A hash of UID strings to future lookup results.
    QHash<QString, QFuture<QString> > _futuresByUid;
};

#endif /* USERNAMERESOLVER_H_ */
