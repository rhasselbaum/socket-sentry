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

#include "CookedPacketDecoderTest.h"
#include "CookedPacketDecoder.h"
#include "CommonTypes.h"

#include <QtCore/QByteArray>
#include <pcap/pcap.h>

Q_DECLARE_METATYPE(Direction)

CookedPacketDecoderTest::CookedPacketDecoderTest() {
}

CookedPacketDecoderTest::~CookedPacketDecoderTest() {
}

void CookedPacketDecoderTest::testDecode() {
    QFETCH(QByteArray, packet);
    QFETCH(qlonglong, expectedOffset);
    QFETCH(Direction, expectedDirection);

    CookedPacketDecoder decoder;
    const u_char* bytes = (const u_char*)packet.constData();
    pcap_pkthdr pcapHeader;
    pcapHeader.caplen = packet.size();
    IpHeader actual = decoder.decode(&pcapHeader, bytes);

    if (expectedOffset < 0) {
        QCOMPARE((qlonglong)actual.start, (qlonglong)NULL);	// not IP
    } else {
        QCOMPARE((qlonglong)(actual.start - bytes), expectedOffset);
    }
    QCOMPARE(expectedDirection, actual.direction);
}

void CookedPacketDecoderTest::testDecode_data() {
    QTest::addColumn<QByteArray>("packet");
    QTest::addColumn<qlonglong>("expectedOffset");
    QTest::addColumn<Direction>("expectedDirection");

    QByteArray host = QByteArray::fromHex("0000");
    QByteArray broadcast = QByteArray::fromHex("0001");
    QByteArray multicast = QByteArray::fromHex("0002");
    QByteArray otherHost = QByteArray::fromHex("0003");
    QByteArray outgoing = QByteArray::fromHex("0004");
    QByteArray unknownPacketType = QByteArray::fromHex("feef");
    QByteArray filler = QByteArray(12, '\0');	// packet data not used by decoder
    QByteArray etherTypeIpv4 = QByteArray::fromHex("0800");
    QByteArray etherTypeIpv6 = QByteArray::fromHex("86dd");
    QByteArray etherTypeUnknown = QByteArray::fromHex("dead");
    QByteArray payload = QByteArray::fromHex("00");

    QByteArray packets;
    packets.append(host).append(filler).append(etherTypeIpv4).append(payload);
    QTest::newRow("p2p inbound") << packets << (qlonglong)16 << INBOUND;

    packets.clear();
    packets.append(outgoing).append(filler).append(etherTypeIpv4).append(payload);
    QTest::newRow("p2p outbound") << packets << (qlonglong)16 << OUTBOUND;

    packets.clear();
    packets.append(broadcast).append(filler).append(etherTypeIpv4).append(payload);
    QTest::newRow("broadcast") << packets << (qlonglong)16 << INBOUND;

    packets.clear();
    packets.append(multicast).append(filler).append(etherTypeIpv4).append(payload);
    QTest::newRow("multicast") << packets << (qlonglong)16 << INBOUND;

    packets.clear();
    packets.append(otherHost).append(filler).append(etherTypeIpv4).append(payload);
    QTest::newRow("remote chatter") << packets << (qlonglong)16 << UNKNOWN_DIRECTION;

    packets.clear();
    packets.append(unknownPacketType).append(filler).append(etherTypeIpv4).append(payload);
    QTest::newRow("unknown packet type") << packets << (qlonglong)16 << UNKNOWN_DIRECTION;

    packets.clear();
    packets.append(host).append(filler).append(etherTypeIpv6).append(payload);
    QTest::newRow("ipv6") << packets << (qlonglong)16 << INBOUND;

    packets.clear();
    packets.append(host).append(filler).append(etherTypeUnknown).append(payload);
    QTest::newRow("not ip") << packets << (qlonglong)-1 << UNKNOWN_DIRECTION;

    packets.clear();
    packets.append(host).append(filler).append(etherTypeIpv4);
    QTest::newRow("no payload") << packets << (qlonglong)-1 << UNKNOWN_DIRECTION;

    packets.clear();
    packets.append(host).append(filler);
    QTest::newRow("truncated") << packets << (qlonglong)-1 << UNKNOWN_DIRECTION;

}

QTEST_MAIN(CookedPacketDecoderTest)
