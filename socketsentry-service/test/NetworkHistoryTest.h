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

#ifndef NETWORKHISTORYTEST_H_
#define NETWORKHISTORYTEST_H_

#include <QtTest/QtTest>

template <class T, class V> class QHash;
template <class F, class S> class QPair;
class FlowMetrics;
class FlowStatistics;
class IpEndpointPair;

/*
 * Unit test for NetworkHistory.
 */
class NetworkHistoryTest : public QObject {
    Q_OBJECT

public:
    NetworkHistoryTest();
    virtual ~NetworkHistoryTest();

private:
    // Verify that a single export result contains the expected contents
    void verifySingleResult(const QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& result,
            const IpEndpointPair& endpoints, const FlowMetrics& metrics);

private slots:
    void testExportStatistics();
    void testRecordAndRoll();
};

#endif /* NETWORKHISTORYTEST_H_ */
