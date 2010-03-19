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

#ifndef COOKEDPACKETDECODERTEST_H_
#define COOKEDPACKETDECODERTEST_H_

#include <QtTest/QtTest>

/*
 * Unit test for cooked packet decoder.
 */
class CookedPacketDecoderTest : public QObject {
    Q_OBJECT

public:
    CookedPacketDecoderTest();
    virtual ~CookedPacketDecoderTest();

private slots:
    void testDecode();
    void testDecode_data();
};

#endif /* COOKEDPACKETDECODERTEST_H_ */
