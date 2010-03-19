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

#ifndef INTERNETPROTOCOLDECODERTEST_H_
#define INTERNETPROTOCOLDECODERTEST_H_

#include <QtTest/QtTest>

/*
 * Unit test for InternetProtocolDecoder.
 */
class InternetProtocolDecoderTest : public QObject {
    Q_OBJECT

public:
    InternetProtocolDecoderTest();
    virtual ~InternetProtocolDecoderTest();

private slots:
    void testDecodeIpv6();
    void testDecodeIpv6_data();
    void testDecodeIpv4();
    void testDecodeIpv4_data();

private:
    // Handles data-driven tests for both IPv4 and IPv6. Only the data differs.
    void testDecode();

    // Adds common data-driven test columns used in all tests.
    void addCommonTestColumns();
};

#endif /* INTERNETPROTOCOLDECODERTEST_H_ */
