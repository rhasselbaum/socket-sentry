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

#include "UserNameResolver.h"

#include <QtCore/QString>
#include <QtCore/QFuture>
#include <QtCore/QtConcurrentRun>

#include <pwd.h>
#include <unistd.h>

// Default maximum number of cached names.
const uint UserNameResolver::DEFAULT_MAX_SIZE = 200;

// Default age in seconds of a cache entry before it is eligible for eviction.
const uint UserNameResolver::DEFAULT_MAX_AGE_SECS = 60 * 60 * 3;    // 3 hours

UserNameResolver::UserNameResolver() :
    _cache(DEFAULT_MAX_SIZE, DEFAULT_MAX_AGE_SECS) {
}

UserNameResolver::UserNameResolver(uint maxSize, uint maxAgeSecs, uint timerIntervalMs, uint retentionPercent) :
    _cache(maxSize, maxAgeSecs, timerIntervalMs, retentionPercent) {
}

UserNameResolver::~UserNameResolver() {
}

QString UserNameResolver::resolve(const QString& uid) {
    // First, check the futures. There might already be a lookup in progress for this UID.
    QString result;
    if (_cache.contains(uid)) {
        // Cache hit!
        result = _cache.value(uid).toString();
    } else if (_futuresByUid.contains(uid)) {
        // Cache miss, but lookup is already pending. Is it done yet?
        const QFuture<QString>& future = _futuresByUid[uid];
        if (future.isFinished()) {
            // Move result from the pending futures table to the cache.
            _cache.insertNew(uid, future.result());
            result = future.result();
            _futuresByUid.remove(uid);
        } // Else, lookup is in progress, so we return empty string.
    } else {
        // Cache miss and no lookup is pending, so start a new one.
        QFuture<QString> future = QtConcurrent::run(this, &UserNameResolver::resolveNow, uid);
        _futuresByUid.insert(uid, future);
    }
    return result;
}


QString UserNameResolver::resolveNow(const QString& uid) {
    // Query the OS. Use reentrant function for thread-safety.
    long bufLen = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufLen <= 0) bufLen = 1024;
    char buf[bufLen];
    passwd userRec;
    passwd* resultRec = NULL;
    QString result;
    if (!getpwuid_r(uid.toUInt(), &userRec, buf, bufLen, &resultRec) && resultRec) {
        result = userRec.pw_name;
    }
    return result;
}
