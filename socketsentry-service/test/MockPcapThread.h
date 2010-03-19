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


#ifndef MOCKPCAPTHREAD_H_
#define MOCKPCAPTHREAD_H_

#include "IPcapThread.h"
#include "IpEndpointPair.h"
#include "FlowMetrics.h"
#include "FlowStatistics.h"

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QPair>
#include <gmock/gmock.h>

/*
 * Mock.
 */
class MockPcapThread : public IPcapThread {
public:
    MOCK_METHOD0(keepAlive, bool());
    MOCK_METHOD0(cancel, void());
    MOCK_METHOD2(fillStatistics, bool(QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& result, QString& error));
    MOCK_METHOD0(begin, void());
    MOCK_CONST_METHOD0(isDone, bool());
    MOCK_CONST_METHOD0(canContinue, bool());
    MOCK_CONST_METHOD1(anyTrafficSince, bool(time_t));

};

#endif /* MOCKPCAPTHREAD_H_ */
