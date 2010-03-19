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

#ifndef HOSTADDRESSUTILS_H_
#define HOSTADDRESSUTILS_H_

class QHostAddress;

/*
 * Set of utility functions for working wirh QHostAddress objects.
 */
class HostAddressUtils {
public:
    HostAddressUtils();
    virtual ~HostAddressUtils();

    // Returns true if the address is an IPv6 address that corresponds to an IPv4 endpoint. Such "mapped" IP addresses
    // follow the form ::ffff:xxxx:xxxx where xxxx:xxxx is the 32-bit Ipv4 address.
    static bool isIpv4MappedAs6(const QHostAddress& hostAddress);

    // Discard all but the last 32-bits of an IPv6 address and return a new IPv4 address based on this value. This is most
    // useful for restoring IPv4 addresses that were previously mapped into IPv6 (see "isIpv4MappedAs6"). If the input
    // IP address is not IPv6, then this function just returns a copy of it.
    static QHostAddress demoteIpv6To4(const QHostAddress& hostAddress);
};

#endif /* HOSTADDRESSUTILS_H_ */
