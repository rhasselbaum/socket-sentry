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

#ifndef COMMUNICATIONFLOWDATA_H_
#define COMMUNICATIONFLOWDATA_H_

#include "IpEndpointPair.h"
#include "FlowMetrics.h"
#include "FlowStatistics.h"

#include <QtCore/QSharedData>
#include <QtCore/QList>

class OsProcess;

/*
 * Shared data for CommunicationFlow.
 */
class CommunicationFlowData : public QSharedData {
public:
    CommunicationFlowData();
    virtual ~CommunicationFlowData();

    IpEndpointPair ipEndpointPair;
    QList<OsProcess> osProcesses;
    FlowMetrics flowMetrics;
    FlowStatistics flowStatistics;
};

#endif /* COMMUNICATIONFLOWDATA_H_ */
