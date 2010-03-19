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

#include "InternetProtocolDecoder.h"

#include <netinet/in.h>
#include <pcap/pcap.h>

#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkAddressEntry>
#include <QtCore/QPair>

#include "CommonTypes.h"
#include "IpEndpointPair.h"
#include "tcpdump-includes/ip.h"
#include "tcpdump-includes/ip6.h"


// Beginning of the transport header. It's the same for TCP and UDP.
struct TransportHeader {
    u_int16_t	srcPort;
    u_int16_t	destPort;
};

InternetProtocolDecoder::InternetProtocolDecoder() {
}

InternetProtocolDecoder::~InternetProtocolDecoder() {
}

Direction InternetProtocolDecoder::decode(const Direction linkLayerDirection, uint caplen, const u_char* packet, IpEndpointPair& endpoints) const {
    Direction result = tryIpv4(linkLayerDirection, caplen, packet, endpoints);
    if (result != UNKNOWN_DIRECTION) return result;
    result = tryIpv6(linkLayerDirection, caplen, packet, endpoints);
    return result;
}

Direction InternetProtocolDecoder::tryIpv4(const Direction linkLayerDirection, uint caplen, const u_char* packet, IpEndpointPair& endpoints) const {
    Q_ASSERT(packet);
    Direction result = UNKNOWN_DIRECTION;
    if (caplen > sizeof(ip)) {
        ip* iptr = (ip*)packet;
        if (IP_V(iptr) == 4 && (iptr->ip_p == IPPROTO_TCP || iptr->ip_p == IPPROTO_UDP)) {
            // Looks like TCP/UDP over IPv4. Perfect!
            const u_char* trPacket = packet + (IP_HL(iptr) * 4);
            if(trPacket + sizeof(TransportHeader) <= packet + caplen) {
                // Determine the source and destination ports.
                TransportHeader* trptr = (TransportHeader*)trPacket;
                u_int16_t srcPort = ntohs(trptr->srcPort);
                u_int16_t destPort = ntohs(trptr->destPort);
                // Determine the source and destination addresses.
                QHostAddress srcAddr(ntohl(iptr->ip_src.s_addr));
                QHostAddress destAddr(ntohl(iptr->ip_dst.s_addr));
                // Determine the direction.
                result = linkLayerDirection;
                if (result == UNKNOWN_DIRECTION) {
                    // Data link decoder couldn't determine direction. Let's try to do it by IP.
                    result  = findDirection(srcAddr, destAddr, 4);
                }
                if (result != UNKNOWN_DIRECTION) {
                    L4Protocol transport = iptr->ip_p == IPPROTO_TCP ? TCP : UDP;
                    populateEndpoints(result, srcAddr, srcPort, destAddr, destPort, transport, endpoints);
                }
            }
        }
    }

    return result;
}

Direction InternetProtocolDecoder::findDirection(const QHostAddress& srcAddr, const QHostAddress& destAddr, uint ipVersion) const {
    // First see if the source or destination IP address can be matched up to one of the interface addresses.
     QListIterator<QNetworkAddressEntry> i(_localAddresses);
     while (i.hasNext()) {
         const QNetworkAddressEntry& entry = i.next();
         QHostAddress ip = entry.ip();
         if(srcAddr == ip) {
             return OUTBOUND;
         } else if (destAddr == ip) {
             return INBOUND;
         }
     }

     if (ipVersion == 4) {
         // Is it IPv4 broadcast or multicast?
         i.toFront();
         while (i.hasNext()) {
             const QNetworkAddressEntry& entry = i.next();
             if (destAddr == entry.broadcast()) {
                 return INBOUND;	// IPv4 broadcast, treat as incoming
             } else {
                 quint32 firstQuad = entry.ip().toIPv4Address() >> 24;
                 if (firstQuad >= 224 && firstQuad <= 239) {
                     return INBOUND;	// IPv4 multicast, treat as incoming
                 }
             }
         }
     }

     // We don't handle IPv6 directionality for multicast (yet).
     // Hopefully the link layer decoder has determined it for us.

    return UNKNOWN_DIRECTION;
}

bool InternetProtocolDecoder::findIpv6Transport(uint caplen, const u_char* header,
        u_int8_t headerType, TransportSummary& output) const {

    switch(headerType) {
    case IPPROTO_HOPOPTS:
    case IPPROTO_ROUTING:
    case IPPROTO_DSTOPTS:
        // Variable length extension header. The first two bytes contain the length and next header type.
        // Use those to advance to the next header. We don't care about the extension itself.
        if (caplen > sizeof(ip6_ext)) {
            ip6_ext* extHeader = (ip6_ext*)header;
            uint extHdrLen = (extHeader->ip6e_len + 1) * 8;
            if (caplen > extHdrLen) {
                return findIpv6Transport(caplen - extHdrLen, header + extHdrLen, extHeader->ip6e_nxt, output);
            }
        }
        break;
    case IPPROTO_FRAGMENT:
        // Fixed length fragment extension header. Advanced to the next header.
        if (caplen > sizeof(ip6_frag)) {
            uint extHdrLen = sizeof(ip6_frag);
            ip6_frag* extHeader = (ip6_frag*)header;
            return findIpv6Transport(caplen - extHdrLen, header + extHdrLen, extHeader->ip6f_nxt, output);
        }
        break;
    case IPPROTO_TCP:
    case IPPROTO_UDP:
        // Transport header.
        if (caplen >= sizeof(TransportHeader)) {
            // Yay! We can get the ports here.
            TransportHeader* trHeader = (TransportHeader*)header;
            output.srcPort = ntohs(trHeader->srcPort);
            output.destPort = ntohs(trHeader->destPort);
            output.protocol = (headerType == IPPROTO_TCP) ? TCP6 : UDP6;
            return true;
        }
        break;
    default:
        // No other headers types supported right now.
        break;
    }
    return false;
}

Direction InternetProtocolDecoder::tryIpv6(const Direction linkLayerDirection, uint caplen, const u_char* packet, IpEndpointPair& endpoints) const {
    Q_ASSERT(packet);
    Direction result = UNKNOWN_DIRECTION;
    if (caplen > sizeof(ip6_hdr)) {
        ip6_hdr* iptr = (ip6_hdr*)packet;
        if ((iptr->ip6_vfc >> 4) == 6) {
            // Looks like an IPv6 header. So far so good.
            QHostAddress srcAddr((quint8*)&iptr->ip6_src);
            QHostAddress destAddr((quint8*)&iptr->ip6_dst);
            const u_char* nextHeader = packet + sizeof(ip6_hdr);
            TransportSummary transport;
            if (findIpv6Transport(caplen - sizeof(ip6_hdr), nextHeader, iptr->ip6_nxt, transport)) {
                // Found transport layer header.
                result = linkLayerDirection;
                if (result == UNKNOWN_DIRECTION) {
                    // Data link decoder couldn't determine direction. Let's try to do it by IP.
                    result = findDirection(srcAddr, destAddr, 6);
                }
                if (result != UNKNOWN_DIRECTION) {
                    populateEndpoints(result, srcAddr, transport.srcPort, destAddr, transport.destPort,
                            transport.protocol, endpoints);
                }
            }
        }
    }
    return result;
}

void InternetProtocolDecoder::populateEndpoints(const Direction direction, const QHostAddress& srcAddr,
        const u_int16_t srcPort, const QHostAddress& destAddr, const u_int16_t destPort,
        const L4Protocol l4protocol, IpEndpointPair& output) const {

    // Populate the result.
    switch (direction) {
    case INBOUND:
        output.setLocalAddr(destAddr);
        output.setLocalPort(destPort);
        output.setRemoteAddr(srcAddr);
        output.setRemotePort(srcPort);
        output.setTransport(l4protocol);
        break;
    case OUTBOUND:
        output.setLocalAddr(srcAddr);
        output.setLocalPort(srcPort);
        output.setRemoteAddr(destAddr);
        output.setRemotePort(destPort);
        output.setTransport(l4protocol);
        break;
    default:
        break;
    }

}

