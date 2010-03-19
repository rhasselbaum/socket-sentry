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

#ifndef COMMUNICATIONFLOWITEMKEYDATA_H_
#define COMMUNICATIONFLOWITEMKEYDATA_H_
\
#include "OsProcess.h"
#include "IpEndpointPair.h"

#include <QtCore/QSharedData>
#include <QtCore/QList>

/*
 * Shared data for CommunicationFlowItemKey.
 */
class CommunicationFlowItemKeyData : public QSharedData {
public:
    CommunicationFlowItemKeyData();
    virtual ~CommunicationFlowItemKeyData();

    IpEndpointPair endpoints;
    QList<OsProcess> osProcesses;
};

#endif /* COMMUNICATIONFLOWITEMKEYDATA_H_ */
