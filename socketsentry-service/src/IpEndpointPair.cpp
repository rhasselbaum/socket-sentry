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

#include <QtDBus/QDBusArgument>
#include <QtNetwork/QHostAddress>

#include "IpEndpointPair.h"
#include "HostAddressUtils.h"

IpEndpointPair::IpEndpointPair() {
    _d = new IpEndpointPairData();
}

IpEndpointPair::IpEndpointPair(const QHostAddress& localAddr, ushort localPort,
        const QHostAddress& remoteAddr, ushort remotePort, L4Protocol transport) {
    _d = new IpEndpointPairData();
    _d->localAddr = localAddr;
    _d->localPort = localPort;
    _d->remoteAddr = remoteAddr;
    _d->remotePort = remotePort;
    _d->transport = transport;
}

IpEndpointPair::IpEndpointPair(const QHostAddress& localAddr, const QHostAddress& remoteAddr) {
    // Aggregation of connections.
    _d = new IpEndpointPairData();
    _d->localAddr = localAddr;
    _d->remoteAddr = remoteAddr;
}


IpEndpointPair::~IpEndpointPair() {
}

bool IpEndpointPair::operator==(const IpEndpointPair& rhs ) const {
    if (_d == rhs._d)
        return true;
    return (_d->remoteAddr == rhs.getRemoteAddr()
            && _d->remotePort == rhs.getRemotePort()
            && _d->localAddr == rhs.getLocalAddr()
            && _d->localPort == rhs.getLocalPort()
            && _d->transport == rhs.getTransport());
}

QString IpEndpointPair::getTransportName() const {
    QString transport;
    switch(_d->transport) {
    case TCP:
        transport = "tcp4";
        break;
    case UDP:
        transport = "udp4";
        break;
    case TCP6:
        transport = "tcp6";
        break;
    case UDP6:
        transport = "udp6";
        break;
    default:
        transport = "tcp/udp";
    }
    return transport;
}

QString IpEndpointPair::toString() const {
    QString transport = getTransportName();
    if (_d->remoteHostName.isEmpty()) {
        return QString("%1 L: %2:%3 R: %4:%5")
            .arg(transport, _d->localAddr.toString())
            .arg(_d->localPort)
            .arg(_d->remoteAddr.toString())
            .arg(_d->remotePort);
    } else {
        return QString("%1 L: %2:%3 R: %4:%5 (%6)")
            .arg(transport, _d->localAddr.toString())
            .arg(_d->localPort)
            .arg(_d->remoteAddr.toString())
            .arg(_d->remotePort)
            .arg(_d->remoteHostName);
    }
}

bool IpEndpointPair::isIpv4MappedAs6() const {
    return HostAddressUtils::isIpv4MappedAs6(_d->localAddr) && HostAddressUtils::isIpv4MappedAs6(_d->remoteAddr);
}

IpEndpointPair IpEndpointPair::demoteIpv6To4() const {
    IpEndpointPair result;
    result.setLocalAddr(HostAddressUtils::demoteIpv6To4(_d->localAddr));
    result.setLocalPort(_d->localPort);
    result.setRemoteAddr(HostAddressUtils::demoteIpv6To4(_d->remoteAddr));
    result.setRemoteHostName(_d->remoteHostName);
    result.setRemotePort(_d->remotePort);
    L4Protocol transport = _d->transport;
    if (transport == TCP6) transport = TCP;
    if (transport == UDP6) transport = UDP;
    result.setTransport(transport);
    return result;
}

QString IpEndpointPair::getRemoteNameOrAddress(int subdomainLevels) const {
    if (_d->remoteHostName.isEmpty()) {
        return _d->remoteAddr.toString();
    } else {
        if (subdomainLevels > 0) {
            int partsLeft = subdomainLevels;
            int index = 0;
            while ((index = _d->remoteHostName.lastIndexOf('.', index - 1)) >= 0 && partsLeft-- > 0) {
                // No op.
            }
            if (index >= 0) {
                // Name has too many subdomains, so we truncate the left part.
                return _d->remoteHostName.right(_d->remoteHostName.size() - index - 1);
            } else {
                // Name does not have too many subdomains. Just return it in full.
                return _d->remoteHostName;
            }
        } else {
            return _d->remoteHostName;
        }
    }
}

uint qHash(const IpEndpointPair& key) {
    return qHash(key.getLocalAddr())
            ^ qHash(key.getLocalPort())
            ^ qHash(key.getRemoteAddr())
            ^ qHash(key.getRemotePort())
            ^ qHash(key.getTransport());
}


QDBusArgument& operator<<(QDBusArgument& arg, const IpEndpointPair& obj) {
    arg.beginStructure();
    arg << obj.getLocalAddr().toString() << obj.getLocalPort() << obj.getRemoteAddr().toString()
        << obj.getRemoteHostName() << obj.getRemotePort() << (int)obj.getTransport();
    arg.endStructure();
    return arg;
}

const QDBusArgument& operator>>(const QDBusArgument& arg, IpEndpointPair& obj) {
    arg.beginStructure();

    QString localAddrEnc;
    arg >> localAddrEnc;
    obj.setLocalAddr(QHostAddress(localAddrEnc));

    ushort localPort;
    arg >> localPort;
    obj.setLocalPort(localPort);

    QString remoteAddrEnc;
    arg >> remoteAddrEnc;
    obj.setRemoteAddr(QHostAddress(remoteAddrEnc));

    QString remoteHostName;
    arg >> remoteHostName;
    obj.setRemoteHostName(remoteHostName);

    ushort remotePort;
    arg >> remotePort;
    obj.setRemotePort(remotePort);

    int transportEnc;
    arg >> transportEnc;
    obj.setTransport((L4Protocol)transportEnc);

    arg.endStructure();
    return arg;
}

