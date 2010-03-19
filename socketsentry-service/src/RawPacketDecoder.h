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

#ifndef RAWPACKETDECODER_H_
#define RAWPACKETDECODER_H_

#include "DataLinkPacketDecoder.h"

/*
 * A trivial decoder that assumes the entire packet is an IP packet with no data link enclosure.
 */
class RawPacketDecoder : public DataLinkPacketDecoder {
public:
    RawPacketDecoder();
    virtual ~RawPacketDecoder();

    // Returns the packet address as the start of the IP header descriptor.
    virtual IpHeader decode(const pcap_pkthdr* pcapHdr, const u_char* packet);

    // Return printable name of this decoder for debugging.
    virtual QString name() const;

};

#endif /* RAWPACKETDECODER_H_ */
