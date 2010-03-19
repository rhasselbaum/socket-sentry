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

#ifndef IPENDPOINTPAIR_H_
#define IPENDPOINTPAIR_H_

#include "IpEndpointPairData.h"

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

class QString;
class QDBusArgument;
class QHostAddress;

/*
 * A pair of endpoints and a transport protocol that collectively identify a unique connection or datagram pathway.
 * The pair can be used as a key in QHash instances. The pair may optionally include the remote host name, but the
 * name is not considered in equality tests and hash value. Optionally, the ports and transport protocol can be
 * left unspecified in which case this object represents an aggregation of connections between two hosts.
 */
class IpEndpointPair {
public:
    IpEndpointPair();
    IpEndpointPair(const QHostAddress& localAddr, ushort localPort,
            const QHostAddress& remoteAddr, ushort remotePort, L4Protocol transport);
    IpEndpointPair(const QHostAddress& localAddr, const QHostAddress& remoteAddr);
    bool operator==(const IpEndpointPair& rhs) const;
    virtual ~IpEndpointPair();

    QHostAddress getLocalAddr() const { return _d->localAddr; }
    ushort getLocalPort() const { return _d->localPort; }
    QHostAddress getRemoteAddr() const { return _d->remoteAddr; }
    QString getRemoteHostName() const { return _d->remoteHostName; }
    ushort getRemotePort() const { return _d->remotePort; }
    L4Protocol getTransport() const { return _d->transport; }

    void setLocalAddr(const QHostAddress& localAddr) { _d->localAddr = localAddr; }
    void setLocalPort(ushort localPort) { _d->localPort = localPort; }
    void setRemoteAddr(const QHostAddress& remoteAddr) { _d->remoteAddr = remoteAddr; }
    void setRemoteHostName(const QString& remoteHostName) { _d->remoteHostName = remoteHostName; }
    void setRemotePort(ushort remotePort) { _d->remotePort = remotePort; }
    void setTransport(L4Protocol transport) { _d->transport = transport; }

    // Format the pair as a string suitable for debug output, but not end-user consumption.
    virtual QString toString() const;

    // Return the remote host name if it is non-empty. Else, return the remote host IP. In case a host name is
    // returned, the argument specifies the maximum number of subdomain levels to return in the result. A zero
    // or negative value means unlimited.
    QString getRemoteNameOrAddress(int subdomainLevels = 0) const;

    // Return the transport protocol as a name string.
    QString getTransportName() const;

    // Return the remote host name (if known) or IP address and port as "<host>:<port>". If the port is
    // unspecified, only the host name or IP address is returned. The argument specifies the maximum number of
    // subdomain levels to return for the host name. A zero or negative value means unlimited.
    QString getFormattedRemoteEndpoint(int subdomainLevels = 0) const {
        if (_d->remotePort <= 0) {
            return getRemoteNameOrAddress(subdomainLevels);
        } else {
            return QString("%1:%2").arg(getRemoteNameOrAddress(subdomainLevels)).arg(_d->remotePort);
        }
    }

    // Return the local IP address and port as "<address>:<port>" or just the IP address if the port is unspecified.
    QString getLocalEndpoint() const {
        if (_d->localPort <= 0) {
            return _d->localAddr.toString();
        } else {
            return QString("%1:%2").arg(_d->localAddr.toString()).arg(_d->localPort);
        }
    }

    // Return the remote IP address and port as "<address>:<port>" or just the IP address if the port is unspecified.
    QString getRemoteEndpoint() const {
        if (_d->remotePort <= 0) {
            return _d->remoteAddr.toString();
        } else {
            return QString("%1:%2").arg(_d->remoteAddr.toString()).arg(_d->remotePort);
        }
    }

    // Returns true if both endpoints are IPv4 addresses mapped as IPv6. See HostAddressUtils method of the same name
    // for explanation of what this means.
    bool isIpv4MappedAs6() const;

    // Demote an IPv6 connection to IPv4 by converting the IPv6 addresses to IPv4 and switching the transport layer
    // protocol to the IPv4 equivalent. This is useful for restoring IPv4 connections that were mapped into IPv6
    // address space by the operating system (see "isIpv4MappedAs6"). If this endpoint pair is not IPv6, then the
    // addresses in the returned object are unchanged.
    IpEndpointPair demoteIpv6To4() const;

private:
    QSharedDataPointer<IpEndpointPairData> _d;
};

// Global hash function required to use the class as a key in QHash containers.
uint qHash(const IpEndpointPair& key);

// DBUS argument marshalling.
QDBusArgument& operator<<(QDBusArgument& argument, const IpEndpointPair& obj);
// DBUS argument unmarshalling.
const QDBusArgument& operator>>(const QDBusArgument& argument, IpEndpointPair& obj);

// Make visible as a D-Bus data type.
Q_DECLARE_METATYPE(IpEndpointPair)

#endif /* IPENDPOINTPAIR_H_ */
