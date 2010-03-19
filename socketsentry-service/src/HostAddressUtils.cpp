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

#include "HostAddressUtils.h"

#include <QtNetwork/QHostAddress>
#include <QtNetwork/QAbstractSocket>

#include <netinet/in.h>

HostAddressUtils::HostAddressUtils() {
}

HostAddressUtils::~HostAddressUtils() {
}

bool HostAddressUtils::isIpv4MappedAs6(const QHostAddress& hostAddress) {
    if(hostAddress.protocol() == QAbstractSocket::IPv6Protocol) {
        Q_IPV6ADDR addr = hostAddress.toIPv6Address();
        uchar mask[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF };
        return memcmp(&addr, mask, sizeof(mask)) == 0;
    }
    return false;
}

QHostAddress HostAddressUtils::demoteIpv6To4(const QHostAddress& hostAddress) {
    if(hostAddress.protocol() == QAbstractSocket::IPv6Protocol) {
        Q_IPV6ADDR ipv6addr = hostAddress.toIPv6Address();
        Q_ASSERT(sizeof(Q_IPV6ADDR) == 16);
        const uchar* ipv4Bytes = &ipv6addr[12];
        quint32* ipv4addr = (quint32*)ipv4Bytes;
        return QHostAddress(ntohl(*ipv4addr));
    } else {
        return hostAddress;
    }
}

