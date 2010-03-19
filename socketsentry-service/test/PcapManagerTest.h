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

#ifndef PCAPMANAGERTEST_H_
#define PCAPMANAGERTEST_H_

#include <QtCore/QObject>

/*
 * Unit test for PcapManager.
 */
class PcapManagerTest : public QObject {
    Q_OBJECT

public:
    PcapManagerTest();
    virtual ~PcapManagerTest();

    // Timer interval to use in the PcapManager under test.
    static const int TIMER_INTERVAL_MS;

private slots:
    // Test thread lifecycle management.
    void testThreadManagement();

    // Test query for traffic since a given time.
    void testAnyTrafficSince();

    // Test that manager queries threads for their status on demand.
    void testActiveThreadProbe();
};

#endif /* PCAPMANAGERTEST_H_ */
