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

#include "EthernetPacketDecoder.h"
#include "tcpdump-includes/ether.h"
#include "tcpdump-includes/ethertype.h"

#include <pcap/pcap.h>
#include <netinet/in.h>
#include <QtNetwork/QNetworkInterface>
#include <QtCore/QString>
#include <QtCore/QByteArray>

EthernetPacketDecoder::EthernetPacketDecoder() :
    _hwAddress(NULL) {
}

EthernetPacketDecoder::EthernetPacketDecoder(const QNetworkInterface* iface) :
    _hwAddress(NULL) {
    if (iface) {
        // Extract the MAC access if it's valid.
        QByteArray candidateAddress = QByteArray::fromHex(iface->hardwareAddress().toAscii());
        if (candidateAddress.size() == ETHER_ADDR_LEN) {
            _hwAddress = new QByteArray(candidateAddress);
        }
    }
}

EthernetPacketDecoder::EthernetPacketDecoder(const QByteArray* hwAddress) :
    _hwAddress(NULL) {
    if (hwAddress && hwAddress->size() == ETHER_ADDR_LEN) {
        _hwAddress = new QByteArray(*hwAddress);
    }
}

EthernetPacketDecoder::~EthernetPacketDecoder() {
    delete _hwAddress;
    _hwAddress = NULL;
}

IpHeader EthernetPacketDecoder::decode(const pcap_pkthdr* pcapHdr, const u_char* packet) {
    // This routine is adapted from the iftop project, copyright 2002-2010 by Paul Warren, Chris Lightfoot, and others.
    // http://www.ex-parrot.com/pdw/iftop
    IpHeader ipHeader;
    if (sizeof(ether_header) < pcapHdr->caplen) {
        ether_header* eptr = (ether_header*)packet;
        int ether_type = ntohs(eptr->ether_type);
        const unsigned char* payload = packet + sizeof(ether_header);
        if (ether_type == ETHERTYPE_8021Q && sizeof(ether_header) + sizeof(vlan_8021q_header) < pcapHdr->caplen) {
            vlan_8021q_header* vptr = (vlan_8021q_header*)payload;
            ether_type = ntohs(vptr->ether_type);
            payload += sizeof(vlan_8021q_header);
        }
        if ((ether_type == ETHERTYPE_IP || ether_type == ETHERTYPE_IPV6) && payload < packet + pcapHdr->caplen) {
            ipHeader.start = payload;
            if (_hwAddress) {
                // Is a direction implied by the MAC addresses?
                if (memcmp(eptr->ether_shost, (const void*)(*_hwAddress), ETHER_ADDR_LEN) == 0) {
                    ipHeader.direction = OUTBOUND;
                } else if (memcmp(eptr->ether_dhost, (const void*)(*_hwAddress), ETHER_ADDR_LEN) == 0 ) {
                    ipHeader.direction = INBOUND;
                } else if (memcmp("\xFF\xFF\xFF\xFF\xFF\xFF", eptr->ether_dhost, ETHER_ADDR_LEN) == 0) {
                    // broadcast packet, count as incoming
                    ipHeader.direction = INBOUND;
                } else if (memcmp("\x33\x33", eptr->ether_dhost, 2) == 0) {
                    // IPv6 multicast packet (RFC 3307), count as incoming
                    ipHeader.direction = INBOUND;
                }
            }
        }
    }
    return ipHeader;
}

QString EthernetPacketDecoder::name() const {
    return QString("Ethernet");
}
