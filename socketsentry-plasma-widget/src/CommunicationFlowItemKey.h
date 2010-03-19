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

#ifndef COMMUNICATIONFLOWITEMKEY_H_
#define COMMUNICATIONFLOWITEMKEY_H_

#include "CommunicationFlowItemKeyData.h"
#include "CommunicationFlow.h"

#include <QtCore/QSharedDataPointer>

class IpEndpointPair;
class OsProcess;
template <class E> class QList;

/*
 * Identifies a unique row in a communication flow table model given a specific aggregation mode.
 */
class CommunicationFlowItemKey {
public:
    CommunicationFlowItemKey(const IpEndpointPair& endpoints, const QList<OsProcess>& osProcesses);
    explicit CommunicationFlowItemKey(const CommunicationFlow& flow);
    virtual ~CommunicationFlowItemKey();
    bool operator==(const CommunicationFlowItemKey& rhs) const;

    IpEndpointPair getEndpoints() const { return _d->endpoints; }
    QList<OsProcess> getOsProcesses() const { return _d->osProcesses; }

    // Factory method that generates a new key based only on the host addresses of the endpoints
    // and all attributes of the OS processes. All other identfying attributes of the endpoints
    // are discarded.
    static CommunicationFlowItemKey fromHostPairAndProcess(const CommunicationFlow& flow);

    // Factory method that generates a new key based only on the host addresses of the endpoints
    // and the program name of the OS processes (multiple processes with the same program name are
    // collapsed). All other identfying attributes are discarded.
    static CommunicationFlowItemKey fromHostPairAndProgram(const CommunicationFlow& flow);

private:
    QSharedDataPointer<CommunicationFlowItemKeyData> _d;
};

#endif /* COMMUNICATIONFLOWITEMKEY_H_ */

// Global hash function required to use the class as a key in QHash containers.
uint qHash(const CommunicationFlowItemKey& key);

