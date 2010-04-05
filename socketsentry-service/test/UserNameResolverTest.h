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

#ifndef USERNAMERESOLVERTEST_H_
#define USERNAMERESOLVERTEST_H_

#include <QtCore/QObject>

class UnitTestUserNameResolver;

/*
 * Unit test for UserNameResolver.
 */
class UserNameResolverTest : public QObject {
    Q_OBJECT

public:
    UserNameResolverTest();
    virtual ~UserNameResolverTest();

    // Max age of cache entries.
    static uint MAX_CACHE_ENTRY_AGE_SECS;

private slots:
    // Test lookup and caching.
    void testLookup();

private:
    // Wait several seconds for an asynchronous result to be returned.
    void waitForResult(UnitTestUserNameResolver& resolver, const QString& uid, const QString& expected);
};

#endif /* USERNAMERESOLVERTEST_H_ */
