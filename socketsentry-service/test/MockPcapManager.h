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


#ifndef MOCKPCAPMANAGER_H_
#define MOCKPCAPMANAGER_H_

#include "IPcapManager.h"
#include "FlowMetrics.h"
#include "FlowStatistics.h"
#include "IpEndpointPair.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QPair>
#include <gmock/gmock.h>

/*
 * Mock.
 */
class MockPcapManager : public IPcapManager {
public:
    MOCK_METHOD1(showInterest, void(const QString& device));
    MOCK_CONST_METHOD1(findAllDevices, QStringList(QString& error));
    MOCK_CONST_METHOD0(findCurrentDevices, QStringList());
    MOCK_CONST_METHOD3(fillStatistics, bool(const QString& device, QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& result,
            QString& error));
    MOCK_METHOD1(release, void(const QString& device));
    MOCK_METHOD0(releaseAll, void());
    MOCK_CONST_METHOD1(anyTrafficSince, bool(time_t));
    MOCK_CONST_METHOD0(isStopped, bool());
    MOCK_CONST_METHOD1(isActive, bool(const QString& device));
    MOCK_CONST_METHOD0(getCustomFilter, QString());
    MOCK_METHOD1(setCustomFilter, void(const QString& customFilter));
};

#endif /* MOCKPCAPMANAGER_H_ */
