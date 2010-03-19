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

#ifndef INTERNETPROTOCOLDECODER_H_
#define INTERNETPROTOCOLDECODER_H_


#include "CommonTypes.h"

#include <QtCore/QList>
#include <QtNetwork/QNetworkAddressEntry>
#include <sys/types.h>

class IpEndpointPair;
class IpHeader;

// Transport layer ports and protocol.
struct TransportSummary {
    TransportSummary() : srcPort(0), destPort(0), protocol(UNKNOWN_L4PROTO) { }
    u_int16_t srcPort;
    u_int16_t destPort;
    L4Protocol protocol;
};

/*
 * Decodes IP headers and transport layer type and ports to produce an IP endpoint pair. This decoder supports both
 * IPv4 and IPv6 headers.
 */
class InternetProtocolDecoder {
public:
    InternetProtocolDecoder();
    virtual ~InternetProtocolDecoder();


    // Decode the given TCP/IP or UDP/IP header to determine the endpoint pair and direction of traffic flow.
    // Caller supplies the directionality from the data link layer (if known), a pointer to the start of the IP header,
    // and the length of the captured packet (excluding data link layer encapsulation) as input. This method attempts
    // to decode the packet and populate the endpoint pair output parameter with the two endpoints. If it successfully
    // decodes the packet, it returns the final direction of the traffic flow. Else, it returns UNKNOWN_DIRECTION and
    // the endpoint pair output parameter is unchanged.
    Direction decode(const Direction linkLayerDirection, uint caplen, const u_char* packet, IpEndpointPair& endpoints) const;

    const QList<QNetworkAddressEntry>& getLocalAddresses() const { return this->_localAddresses; }
    void setLocalAddresses(const QList<QNetworkAddressEntry>& localAddresses) { this->_localAddresses = localAddresses; }

private:
    // Try to decode the packet as IPv4.
    Direction tryIpv4(const Direction linkLayerDirection, uint caplen, const u_char* packet, IpEndpointPair& endpoints) const;

    // Try to decode the packet as IPv6.
    Direction tryIpv6(const Direction linkLayerDirection, uint caplen, const u_char* packet, IpEndpointPair& endpoints) const;

    // Try to determine the direction of the packet based on the IP source and destination addresses.
    Direction findDirection(const QHostAddress& srcAddr, const QHostAddress& destAddr, uint ipVersion) const;

    // Skips through zero or more IPv6 extension headers and tries to find the transport layer source and destination
    // ports and protocol (TCP or UDP). The caller passes a pointer to the next header in the packet buffer (either a
    // transport layer header or an extension header), the length of the buffer excluding previous headers, and the
    // header type as input. If the method finds a valid transport header at or after the current header, it populates
    // the transport summary output argument and returns true. Else, it returns false.
    bool findIpv6Transport(uint caplen, const u_char* header, u_int8_t headerType, TransportSummary& output) const;

    // Populate an IP endpoint pair given source and destination addresses/ports and a direction. If the direction
    // is INBOUND, the source is remote and the destination is local. If the direction is OUTBOUND, the source is
    // local and the destination is remote. If the direction is UNKNOWN_DIRECTION, no change is made to the endpoint
    // pair.
    void populateEndpoints(const Direction direction, const QHostAddress& srcAddr, const u_int16_t srcPort,
            const QHostAddress& destAddr, const u_int16_t destPort, const L4Protocol l4protocol,
            IpEndpointPair& output) const;

    QList<QNetworkAddressEntry> _localAddresses;
};

#endif /* INTERNETPROTOCOLDECODER_H_ */
