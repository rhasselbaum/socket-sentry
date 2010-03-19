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

#include "EthernetPacketDecoderTest.h"
#include "EthernetPacketDecoder.h"
#include "CommonTypes.h"

#include <QtCore/QByteArray>
#include <QtNetwork/QNetworkInterface>
#include <pcap/pcap.h>

Q_DECLARE_METATYPE(Direction)

EthernetPacketDecoderTest::EthernetPacketDecoderTest() {
}

EthernetPacketDecoderTest::~EthernetPacketDecoderTest() {
}

void EthernetPacketDecoderTest::testDecode() {
    QFETCH(QByteArray, hwAddress);
    QFETCH(QByteArray, packet);
    QFETCH(qlonglong, expectedOffset);
    QFETCH(Direction, expectedDirection);

    QByteArray* hwAddressPtr = NULL;
    if (!hwAddress.isEmpty()) {
        hwAddressPtr = &hwAddress;
    }
    EthernetPacketDecoder decoder(hwAddressPtr);
    const u_char* bytes = (const u_char*)packet.constData();
    pcap_pkthdr pcapHeader;
    pcapHeader.caplen = packet.size();
    IpHeader actual = decoder.decode(&pcapHeader, bytes);

    if (expectedOffset < 0) {
        QCOMPARE((qlonglong)actual.start, (qlonglong)NULL);	// not IP over Ethernet
    } else {
        QCOMPARE((qlonglong)(actual.start - bytes), expectedOffset);
    }
    QCOMPARE(actual.direction, expectedDirection);
}

void EthernetPacketDecoderTest::testDecode_data() {
    QTest::addColumn<QByteArray>("hwAddress");
    QTest::addColumn<QByteArray>("packet");
    QTest::addColumn<qlonglong>("expectedOffset");
    QTest::addColumn<Direction>("expectedDirection");

    QByteArray localMac = QByteArray::fromHex("001c234ea483");
    QByteArray remoteMac = QByteArray::fromHex("00223f0eaf03");
    QByteArray broadcast = QByteArray::fromHex("ffffffffffff");
    QByteArray ipv6Multicast = QByteArray::fromHex("333300000000");
    QByteArray etherTypeIpv4 = QByteArray::fromHex("0800");
    QByteArray etherTypeIpv6 = QByteArray::fromHex("86dd");
    QByteArray etherType8021q = QByteArray::fromHex("8100");
    QByteArray etherTypeUnknown = QByteArray::fromHex("dead");
    QByteArray Ieee8021qPriority = QByteArray::fromHex("0001");
    QByteArray payload = QByteArray::fromHex("00");
    QByteArray empty;

    QByteArray packets;
    packets.append(localMac).append(remoteMac).append(etherTypeIpv4).append(payload);
    QTest::newRow("p2p inbound") << localMac << packets << (qlonglong)14 << INBOUND;

    packets.clear();
    packets.append(remoteMac).append(localMac).append(etherTypeIpv4).append(payload);
    QTest::newRow("p2p outbound") << localMac << packets << (qlonglong)14 << OUTBOUND;

    packets.clear();
    packets.append(remoteMac).append(localMac).append(etherTypeIpv6).append(payload);
    QTest::newRow("ipv6") << localMac << packets << (qlonglong)14 << OUTBOUND;

    packets.clear();
    packets.append(remoteMac).append(localMac).append(etherTypeUnknown).append(payload);
    QTest::newRow("not ip") << localMac << packets << (qlonglong)-1 << UNKNOWN_DIRECTION;

    packets.clear();
    packets.append(localMac).append(remoteMac).append(etherTypeIpv4);
    QTest::newRow("no payload") << localMac << packets << (qlonglong)-1 << UNKNOWN_DIRECTION;

    packets.clear();
    packets.append(localMac).append(remoteMac);
    QTest::newRow("truncated") << localMac << packets << (qlonglong)-1 << UNKNOWN_DIRECTION;

    packets.clear();
    packets.append(broadcast).append(remoteMac).append(etherTypeIpv4).append(payload);
    QTest::newRow("broadcast") << localMac << packets << (qlonglong)14 << INBOUND;

    packets.clear();
    packets.append(ipv6Multicast).append(remoteMac).append(etherTypeIpv6).append(payload);
    QTest::newRow("ipv6 multicast") << localMac << packets << (qlonglong)14 << INBOUND;

    packets.clear();
    packets.append(remoteMac).append(localMac).append(etherType8021q).append(Ieee8021qPriority).append(etherTypeIpv4).append(payload);
    QTest::newRow("802.1q outbound") << localMac << packets << (qlonglong)18 << OUTBOUND;

    packets.clear();
    packets.append(remoteMac).append(localMac).append(etherType8021q).append(Ieee8021qPriority).append(etherTypeIpv6).append(payload);
    QTest::newRow("802.1q ipv6") << localMac << packets << (qlonglong)18 << OUTBOUND;

    packets.clear();
    packets.append(remoteMac).append(localMac).append(etherType8021q).append(Ieee8021qPriority).append(etherTypeIpv6);
    QTest::newRow("802.1q no payload") << localMac << packets << (qlonglong)-1 << UNKNOWN_DIRECTION;

    packets.clear();
    packets.append(remoteMac).append(localMac).append(etherType8021q).append(Ieee8021qPriority);
    QTest::newRow("802.1q truncated") << localMac << packets << (qlonglong)-1 << UNKNOWN_DIRECTION;

    packets.clear();
    packets.append(remoteMac).append(remoteMac).append(etherTypeIpv4).append(payload);
    QTest::newRow("remote chatter") << localMac << packets << (qlonglong)14 << UNKNOWN_DIRECTION;

    packets.clear();
    packets.append(localMac).append(remoteMac).append(etherTypeIpv4).append(payload);
    QTest::newRow("no mac address") << empty << packets << (qlonglong)14 << UNKNOWN_DIRECTION;

}

QTEST_MAIN(EthernetPacketDecoderTest)
