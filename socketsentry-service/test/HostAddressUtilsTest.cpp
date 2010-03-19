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

#include "HostAddressUtilsTest.h"
#include "HostAddressUtils.h"

#include <QtNetwork/QHostAddress>

HostAddressUtilsTest::HostAddressUtilsTest() {
}

HostAddressUtilsTest::~HostAddressUtilsTest() {
}

void HostAddressUtilsTest::testIsIpv4MappedAs6() {
    QHostAddress ipv6As6("fe80::1");
    QHostAddress ipv4As6("::ffff:0a0a:0a0a");
    QVERIFY(HostAddressUtils::isIpv4MappedAs6(ipv4As6));
    QVERIFY(!HostAddressUtils::isIpv4MappedAs6(ipv6As6));
}

void HostAddressUtilsTest::testDemoteIpv6To4() {
    QHostAddress ipv4MappedAs6("::ffff:c0a8:a883");
    QHostAddress expectedDemotion("192.168.168.131");
    QHostAddress actualDemotion = HostAddressUtils::demoteIpv6To4(ipv4MappedAs6);
    QCOMPARE(actualDemotion, expectedDemotion);

    QHostAddress origIpv4("10.20.1.1");
    QHostAddress expectedIpv4("10.20.1.1");
    QHostAddress actualIpv4 = HostAddressUtils::demoteIpv6To4(origIpv4);
    QCOMPARE(actualIpv4, expectedIpv4);

}

QTEST_MAIN(HostAddressUtilsTest)
