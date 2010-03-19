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

#ifndef HOSTNAMERESOLVERTEST_H_
#define HOSTNAMERESOLVERTEST_H_

#include <QtCore/QObject>
#include <QtNetwork/QHostInfo>

class UnitTestHostNameResolver;

/*
 * Unit test for HostNameResolver.
 */
class HostNameResolverTest : public QObject {
    Q_OBJECT

public:
    HostNameResolverTest();
    virtual ~HostNameResolverTest();

private slots:
    // Test basic lookups and caching.
    void testLookup();

    // Test cache expirations by size threshold.
    void testExpireBySize();

    // Test cache expirations by time.
    void testExpireByTime();

private:
    // Create a new host info object with the given attributes.
    QHostInfo createHostInfo(int lookupId, QHostInfo::HostInfoError error, const QString& name) const;

    // Asks the resolver to resolve the address, then immediately invokes the lookedUp slot with the given name and no error.
    void autoResolve(UnitTestHostNameResolver& resolver, int lookupId, const QString& address, const QString& name) const;

};

#endif /* HOSTNAMERESOLVERTEST_H_ */
