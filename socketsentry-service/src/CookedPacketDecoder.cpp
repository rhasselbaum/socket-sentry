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

#include "CookedPacketDecoder.h"

#include "tcpdump-includes/sll.h"
#include "tcpdump-includes/ethertype.h"

#include <pcap/pcap.h>
#include <netinet/in.h>
#include <QtCore/QString>


CookedPacketDecoder::CookedPacketDecoder() {
}

CookedPacketDecoder::~CookedPacketDecoder() {
}

IpHeader CookedPacketDecoder::decode(const pcap_pkthdr* pcapHdr, const u_char* packet) {
    // This routine is adapted from the iftop project, copyright 2002-2010 by Paul Warren, Chris Lightfoot, and others.
    // http://www.ex-parrot.com/pdw/iftop
    IpHeader ipHeader;
    if (SLL_HDR_LEN < pcapHdr->caplen) {
        sll_header* sptr = (sll_header*)packet;
        int ether_type = ntohs(sptr->sll_protocol);
        const unsigned char* payload = packet + SLL_HDR_LEN;
        if ((ether_type == ETHERTYPE_IP || ether_type == ETHERTYPE_IPV6) && payload < packet + pcapHdr->caplen) {
            ipHeader.start = payload;
            switch (ntohs(sptr->sll_pkttype)) {
            case LINUX_SLL_HOST:
            case LINUX_SLL_BROADCAST:
            case LINUX_SLL_MULTICAST:
                ipHeader.direction = INBOUND;
                break;
            case LINUX_SLL_OUTGOING:
                ipHeader.direction = OUTBOUND;
                break;
            }
        }
    }
    return ipHeader;
}

QString CookedPacketDecoder::name() const {
    return QString("Cooked");
}

