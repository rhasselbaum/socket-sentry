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

#ifndef COOKEDPACKETDECODER_H_
#define COOKEDPACKETDECODER_H_

#include "DataLinkPacketDecoder.h"

/*
 * Knows how to decode Linux cooked packets such as those generated from the "any" pseudo-device.
 */
class CookedPacketDecoder : public DataLinkPacketDecoder {
public:
    CookedPacketDecoder();
    virtual ~CookedPacketDecoder();

    // Decode the given data link packet to determine the IP header descriptor.
    virtual IpHeader decode(const pcap_pkthdr* pcapHdr, const u_char* packet);

    // Return printable name of this decoder for debugging.
    virtual QString name() const;
};

#endif /* COOKEDPACKETDECODER_H_ */
