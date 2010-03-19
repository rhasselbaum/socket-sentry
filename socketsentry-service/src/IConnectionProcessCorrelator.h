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


#ifndef ICONNECTIONPROCESSCORRELATOR_H_
#define ICONNECTIONPROCESSCORRELATOR_H_

template <class K, class V> class QHash;
template <class E> class QList;
class IpEndpointPair;
class OsProcess;
class QHostAddress;
class QString;

/*
 * Matches IP network connections to OS processes. Retrieves active connection tables and process data from the OS and
 * joins them.
 */
class IConnectionProcessCorrelator {
public:
    virtual ~IConnectionProcessCorrelator() { }

    // Examine the kernel's table of IP connections and match them to processes. Only "real" connections are
    // returned, which excludes, for example, sockets in a LISTEN state or connections involving the ANY addresses.
    // Additionally, for IPv6 entries, the correlator adds equivalent IPv4 entries if it detects that the endpoints
    // are, in fact, IPv4 endpoints mapped into into IPv6 address space, since the OS may express some IPv4 connections
    // that way.
    //
    // The caller supplies an empty result object to receive the output. It returns true on success. If it fails
    // to obtain the data it needs from the OS, it returns false and the error argument is populated with a message.
    virtual bool correlate(QHash<IpEndpointPair, QList<OsProcess> >& result, QString& error) const = 0;
};

#endif /* ICONNECTIONPROCESSCORRELATOR_H_ */
