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

#ifndef DATALINKPACKETDECODER_H_
#define DATALINKPACKETDECODER_H_

#include "CommonTypes.h"

#include <sys/types.h>

struct pcap_h;

struct IpHeader {
    IpHeader() : start(0), direction(UNKNOWN_DIRECTION) { }
    const u_char* start;
    Direction direction;
};

struct pcap_pkthdr;
class QNetworkInterface;
class QString;

/*
 * Decodes data link layer packets to determine IP header and directionality.
 */
class DataLinkPacketDecoder {
public:
    DataLinkPacketDecoder();
    virtual ~DataLinkPacketDecoder();

    // Decode the given data link packet to determine the IP header start and directionality. The
    // method is passed the pcap header and packet bytes If the decoder does not find an IP packet,
    // it should return 0 (NULL) as the start position of the IP header (i.e. invalid packet).
    virtual IpHeader decode(const pcap_pkthdr* pcapHdr, const u_char* packet) = 0;

    // Return printable name of this decoder for debugging.
    virtual QString name() const = 0;

    // Factory method returns a suitable decoder for the given link type or NULL if no suitable
    // decoder is found. The network interface is also provided if it is available.
    // When the caller is done with the decoder, it must free its memory.
    static DataLinkPacketDecoder* fromLinkType(int linkType, const QNetworkInterface* iface);
};

#endif /* DATALINKPACKETDECODER_H_ */
