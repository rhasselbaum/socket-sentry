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

#ifndef COMMUNICATIONFLOWITEMDATA_H_
#define COMMUNICATIONFLOWITEMDATA_H_

#include <QtCore/QSharedData>

#include "CommunicationFlow.h"
#include <KDE/KIcon>
#include <QtGui/QFont>
#include <QtCore/QDateTime>

/*
 * Shared data for CommunicationFlowItem.
 */
class CommunicationFlowItemData : public QSharedData {
public:
    CommunicationFlowItemData();
    virtual ~CommunicationFlowItemData();

    // The communication flow peer.
    CommunicationFlow actual;
    // The number of connections represented within an aggregated communication flow. Default is 1.
    int numConnections;
    // Font that should be used to display this item.
    QFont font;
    // The device state of this flow.
    int deviceState;
    // Icon representing the current network device state of the flow.
    const KIcon* deviceStateIcon;
    // Icon representing the running program.
    KIcon programIcon;
    // The date and time when this flow was first seen by this model.
    QDateTime firstSeenUtc;
    // Returns true if this item is a zombie. See "zombify".
    bool zombie;

};

#endif /* COMMUNICATIONFLOWITEMDATA_H_ */
