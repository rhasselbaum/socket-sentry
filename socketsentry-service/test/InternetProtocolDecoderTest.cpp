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

#include "InternetProtocolDecoderTest.h"
#include "InternetProtocolDecoder.h"
#include "CommonTypes.h"
#include "IpEndpointPair.h"

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtNetwork/QNetworkAddressEntry>

Q_DECLARE_METATYPE(Direction)
Q_DECLARE_METATYPE(QList<QNetworkAddressEntry>)

InternetProtocolDecoderTest::InternetProtocolDecoderTest() {
}

InternetProtocolDecoderTest::~InternetProtocolDecoderTest() {
}

void InternetProtocolDecoderTest::testDecode() {
    QFETCH(QList<QNetworkAddressEntry>, localAddresses);
    QFETCH(Direction, linkLayerDirection);
    QFETCH(QByteArray, packet);
    QFETCH(IpEndpointPair, expectedEndpoints);
    QFETCH(Direction, expectedDirection);

    InternetProtocolDecoder decoder;
    decoder.setLocalAddresses(localAddresses);

    const u_char* bytes = (const u_char*)packet.constData();
    IpEndpointPair actualEndpoints;
    Direction actualDirection = decoder.decode(linkLayerDirection, packet.size(), bytes, actualEndpoints);
    QCOMPARE(expectedDirection, actualDirection);
    QCOMPARE(expectedEndpoints, actualEndpoints);
}

void InternetProtocolDecoderTest::testDecodeIpv4() {
    testDecode();	// test framework will feed in IPv4 test data
}

void InternetProtocolDecoderTest::testDecodeIpv6() {
    testDecode();	// test framework will feed in IPv6 test data
}

void InternetProtocolDecoderTest::addCommonTestColumns() {
    // Columns used for all tests.
    QTest::addColumn<QList<QNetworkAddressEntry> >("localAddresses");
    QTest::addColumn<Direction>("linkLayerDirection");
    QTest::addColumn<QByteArray>("packet");
    QTest::addColumn<IpEndpointPair>("expectedEndpoints");
    QTest::addColumn<Direction>("expectedDirection");
}

void InternetProtocolDecoderTest::testDecodeIpv4_data() {
    addCommonTestColumns();
    // Host addresses.
    QHostAddress localIpAddr("192.168.168.131");
    QHostAddress altLocalIpAddr("10.0.0.1");
    QHostAddress remoteIpAddr("147.129.226.1");
    QHostAddress broadcastAddr("192.168.168.255");
    QHostAddress multicastAddr("224.0.0.1");
    // Local address list as reported by the OS.
    QNetworkAddressEntry localAddressEntry;
    localAddressEntry.setIp(localIpAddr);
    localAddressEntry.setBroadcast(broadcastAddr);
    QList<QNetworkAddressEntry> localAddresses;
    localAddresses.append(localAddressEntry);

    // IP version ID and header length
    QByteArray ipv4NoOptions = QByteArray::fromHex("45");
    QByteArray ipv4WithOptions = QByteArray::fromHex("46");
    QByteArray ipv4BadLength = QByteArray::fromHex("4F");
    QByteArray ipv5 = QByteArray::fromHex("55");	// crazy talk

    QByteArray filler(8, '\0');    // not read by decoder

    // Transports and checksum (decoder ignores checksum)
    QByteArray tcpAndChecksum = QByteArray::fromHex("060000");
    QByteArray udpAndChecksum = QByteArray::fromHex("110000");
    QByteArray badTransportAndChecksum = QByteArray::fromHex("410000");

    QByteArray options(4, '\0');   // not read by decoder

    // Sources and destinations
    QByteArray local = QByteArray::fromHex("c0a8a883");     // 192.168.168.131
    QByteArray altLocal = QByteArray::fromHex("0a000001");  // 10.0.0.1
    QByteArray broadcast = QByteArray::fromHex("c0a8a8ff"); // 192.168.168.255
    QByteArray multicast = QByteArray::fromHex("e0000001"); // 224.0.0.1
    QByteArray remote = QByteArray::fromHex("9381e201"); // 147.129.226.1

    // Ports
    QByteArray port443 = QByteArray::fromHex("01bb");
    QByteArray port1234 = QByteArray::fromHex("04d2");

    QByteArray packet;
    IpEndpointPair expectedEndpoints;
    packet.append(ipv4NoOptions).append(filler).append(tcpAndChecksum).append(remote).append(local).append(port443).append(port1234);
    expectedEndpoints = IpEndpointPair(localIpAddr, 1234, remoteIpAddr, 443, TCP);
    QTest::newRow("p2p inbound") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << INBOUND;

    packet.clear();
    packet.append(ipv4NoOptions).append(filler).append(tcpAndChecksum).append(local).append(remote).append(port1234).append(port443);
    expectedEndpoints = IpEndpointPair(localIpAddr, 1234, remoteIpAddr, 443, TCP);
    QTest::newRow("p2p outbound") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << OUTBOUND;

    packet.clear();
    packet.append(ipv4NoOptions).append(filler).append(tcpAndChecksum).append(remote).append(altLocal).append(port443).append(port1234);
    QTest::newRow("unknown hosts") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;
    expectedEndpoints = IpEndpointPair(altLocalIpAddr, 1234, remoteIpAddr, 443, TCP);
    QTest::newRow("link layer gives direction") << localAddresses << INBOUND << packet << expectedEndpoints << INBOUND;

    packet.clear();
    packet.append(ipv4NoOptions).append(filler).append(udpAndChecksum).append(local).append(remote).append(port1234).append(port443);
    expectedEndpoints = IpEndpointPair(localIpAddr, 1234, remoteIpAddr, 443, UDP);
    QTest::newRow("udp") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << OUTBOUND;

    packet.clear();
    packet.append(ipv4NoOptions).append(filler).append(badTransportAndChecksum).append(local).append(remote).append(port1234).append(port443);
    QTest::newRow("unknown transport") << localAddresses << INBOUND << packet << IpEndpointPair() << UNKNOWN_DIRECTION;

    packet.clear();
    packet.append(ipv4NoOptions).append(filler).append(udpAndChecksum).append(local).append(multicast).append(port1234).append(port443);
    expectedEndpoints = IpEndpointPair(localIpAddr, 1234, multicastAddr, 443, UDP);
    QTest::newRow("multicast outbound") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << OUTBOUND;

    packet.clear();
    packet.append(ipv4NoOptions).append(filler).append(udpAndChecksum).append(multicast).append(local).append(port1234).append(port443);
    expectedEndpoints = IpEndpointPair(localIpAddr, 443, multicastAddr, 1234, UDP);
    QTest::newRow("multicast inbound") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << INBOUND;

    packet.clear();
    packet.append(ipv4NoOptions).append(filler).append(udpAndChecksum).append(local).append(broadcast).append(port1234).append(port443);
    expectedEndpoints = IpEndpointPair(localIpAddr, 1234, broadcastAddr, 443, UDP);
    QTest::newRow("broadcast outbound") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << OUTBOUND;

    packet.clear();
    packet.append(ipv4NoOptions).append(filler).append(udpAndChecksum).append(broadcast).append(local).append(port1234).append(port443);
    expectedEndpoints = IpEndpointPair(localIpAddr, 443, broadcastAddr, 1234, UDP);
    QTest::newRow("broadcast inbound") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << INBOUND;

    packet.clear();
    packet.append(ipv4WithOptions).append(filler).append(tcpAndChecksum).append(remote).append(local).append(options).append(port443).append(port1234);
    expectedEndpoints = IpEndpointPair(localIpAddr, 1234, remoteIpAddr, 443, TCP);
    QTest::newRow("ipv4 options") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << INBOUND;

    packet.clear();
    packet.append(ipv4BadLength).append(filler).append(tcpAndChecksum).append(remote).append(local).append(port443).append(port1234);
    QTest::newRow("bad header length") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;

    packet.clear();
    packet.append(ipv5).append(filler).append(tcpAndChecksum).append(remote).append(local).append(port443).append(port1234);
    QTest::newRow("bad ip version") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;

    packet.clear();
    packet.append(ipv4NoOptions).append(filler).append(tcpAndChecksum).append(remote).append(local).append(port443);
    QTest::newRow("truncated transport header") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;

    packet.clear();
    packet.append(ipv4NoOptions).append(filler).append(tcpAndChecksum).append(remote).append(local);
    QTest::newRow("missing transport header") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;

    packet.clear();
    packet.append(ipv4NoOptions).append(filler).append(tcpAndChecksum).append(remote);
    QTest::newRow("truncated ip header") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;


}
void InternetProtocolDecoderTest::testDecodeIpv6_data() {
    addCommonTestColumns();
    // Host addresses.
    QHostAddress localIpAddr("2001::1");
    QHostAddress altLocalIpAddr("2001::2");
    QHostAddress remoteIpAddr("2001:50::55");
    // Local address list as reported by the OS.
    QNetworkAddressEntry localAddressEntry;
    localAddressEntry.setIp(localIpAddr);
    QList<QNetworkAddressEntry> localAddresses;
    localAddresses.append(localAddressEntry);

    // Start packet data.
    QByteArray versionAndFiller = QByteArray::fromHex("600000000000");  // decoder ignores most of header after version
    QByteArray hopLimit(1, '\0');  // not used

    // Header types
    QByteArray headerTcp = QByteArray::fromHex("06");
    QByteArray headerUdp = QByteArray::fromHex("11");
    QByteArray headerHopByHop = QByteArray::fromHex("00");
    QByteArray headerRouting = QByteArray::fromHex("2B");
    QByteArray headerDstOpts = QByteArray::fromHex("3C");
    QByteArray headerUnknown = QByteArray::fromHex("41");
    QByteArray headerFragment = QByteArray::fromHex("2C");	//fixed length

    // Extension header lengths and filler
    QByteArray noMoreOctetsAndFiller(7, '\0');
    QByteArray oneMoreOctetAndFiller(QByteArray::fromHex("01").append(QByteArray(14, '\0')));

    // Sources and destinations
    QByteArray local = QByteArray::fromHex("20010000000000000000000000000001");  // 2001::1
    QByteArray altLocal = QByteArray::fromHex("20010000000000000000000000000002");  // 2001::2
    QByteArray remote = QByteArray::fromHex("20010050000000000000000000000055"); // 2001:50::55

    // Ports
    QByteArray port443 = QByteArray::fromHex("01bb");
    QByteArray port1234 = QByteArray::fromHex("04d2");

    QByteArray packet;
    IpEndpointPair expectedEndpoints;
    packet.append(versionAndFiller).append(headerTcp).append(hopLimit).append(remote).append(local).append(port443).append(port1234);
    expectedEndpoints = IpEndpointPair(localIpAddr, 1234, remoteIpAddr, 443, TCP6);
    QTest::newRow("p2p inbound") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << INBOUND;

    packet.clear();
    packet.append(versionAndFiller).append(headerTcp).append(hopLimit).append(local).append(remote).append(port1234).append(port443);
    expectedEndpoints = IpEndpointPair(localIpAddr, 1234, remoteIpAddr, 443, TCP6);
    QTest::newRow("p2p outbound") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << OUTBOUND;

    packet.clear();
    packet.append(versionAndFiller).append(headerTcp).append(hopLimit).append(remote).append(altLocal).append(port443).append(port1234);
    QTest::newRow("unknown hosts") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;
    expectedEndpoints = IpEndpointPair(altLocalIpAddr, 1234, remoteIpAddr, 443, TCP6);
    QTest::newRow("link layer gives direction") << localAddresses << INBOUND << packet << expectedEndpoints << INBOUND;

    packet.clear();
    packet.append(versionAndFiller).append(headerUdp).append(hopLimit).append(local).append(remote).append(port1234).append(port443);
    expectedEndpoints = IpEndpointPair(localIpAddr, 1234, remoteIpAddr, 443, UDP6);
    QTest::newRow("udp") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << OUTBOUND;

    packet.clear();
    packet.append(versionAndFiller).append(headerUnknown).append(hopLimit).append(local).append(remote).append(port1234).append(port443);
    QTest::newRow("unknown header") << localAddresses << INBOUND << packet << IpEndpointPair() << UNKNOWN_DIRECTION;

    packet.clear();
    packet.append(versionAndFiller).append(headerHopByHop).append(hopLimit).append(local).append(remote)
            .append(headerUdp).append(noMoreOctetsAndFiller)
            .append(port1234).append(port443);
    expectedEndpoints = IpEndpointPair(localIpAddr, 1234, remoteIpAddr, 443, UDP6);
    QTest::newRow("small extension header") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << OUTBOUND;

    packet.clear();
    packet.append(versionAndFiller).append(headerHopByHop).append(hopLimit).append(local).append(remote)
            .append(headerUdp).append(oneMoreOctetAndFiller)
            .append(port1234).append(port443);
    expectedEndpoints = IpEndpointPair(localIpAddr, 1234, remoteIpAddr, 443, UDP6);
    QTest::newRow("bigger extension header") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << OUTBOUND;

    packet.clear();
    packet.append(versionAndFiller).append(headerHopByHop).append(hopLimit).append(local).append(remote)
            .append(headerFragment).append(noMoreOctetsAndFiller)
            .append(headerRouting).append(noMoreOctetsAndFiller)
            .append(headerDstOpts).append(oneMoreOctetAndFiller)
            .append(headerUdp).append(noMoreOctetsAndFiller)
            .append(port1234).append(port443);
    expectedEndpoints = IpEndpointPair(localIpAddr, 1234, remoteIpAddr, 443, UDP6);
    QTest::newRow("all extension headers") << localAddresses << UNKNOWN_DIRECTION << packet << expectedEndpoints << OUTBOUND;

    packet.clear();
    packet.append(versionAndFiller).append(headerUdp).append(hopLimit).append(local).append(remote).append(port1234);
    QTest::newRow("truncated transport header") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;

    packet.clear();
    packet.append(versionAndFiller).append(headerUdp).append(hopLimit).append(local).append(remote);
    QTest::newRow("missing transport header") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;

    packet.clear();
    packet.append(versionAndFiller).append(headerHopByHop).append(hopLimit).append(local).append(remote)
            .append(headerUdp)
            .append(port1234).append(port443);
    QTest::newRow("truncated extension header") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;

    packet.clear();
    packet.append(versionAndFiller).append(headerHopByHop).append(hopLimit).append(local).append(remote)
            .append(headerUdp);
    QTest::newRow("uncastable extension header") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;

    packet.clear();
    packet.append(versionAndFiller).append(headerHopByHop).append(hopLimit).append(local).append(remote)
            .append(headerFragment)
            .append(port1234).append(port443);
    QTest::newRow("truncated fragment header") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;

    packet.clear();
    packet.append(versionAndFiller).append(headerUdp).append(hopLimit).append(local);
    QTest::newRow("truncated ip header") << localAddresses << UNKNOWN_DIRECTION << packet << IpEndpointPair() << UNKNOWN_DIRECTION;

}

QTEST_MAIN(InternetProtocolDecoderTest)
