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

#include "CommunicationFlowItemKey.h"

#include <QtCore/QList>

CommunicationFlowItemKey::CommunicationFlowItemKey(const IpEndpointPair& endpoints,
        const QList<OsProcess>& osProcesses) {
    _d = new CommunicationFlowItemKeyData();
    _d->endpoints = endpoints;
    _d->osProcesses = osProcesses;
}

CommunicationFlowItemKey::CommunicationFlowItemKey(const CommunicationFlow& flow) {
    _d = new CommunicationFlowItemKeyData();
    _d->endpoints = flow.getIpEndpointPair();
    _d->osProcesses = flow.getOsProcesses();
}

CommunicationFlowItemKey::~CommunicationFlowItemKey() {
}

CommunicationFlowItemKey CommunicationFlowItemKey::fromHostPairAndProcess(const CommunicationFlow& flow) {
    const IpEndpointPair& flowEndpoints = flow.getIpEndpointPair();
    IpEndpointPair keyEndpoints(flowEndpoints.getLocalAddr(), flowEndpoints.getRemoteAddr());
    keyEndpoints.setRemoteHostName(flowEndpoints.getRemoteHostName());
    CommunicationFlowItemKey result(keyEndpoints, flow.getOsProcesses());
    return result;
}

CommunicationFlowItemKey CommunicationFlowItemKey::fromHostPairAndProgram(const CommunicationFlow& flow) {
    const IpEndpointPair& flowEndpoints = flow.getIpEndpointPair();
    IpEndpointPair keyEndpoints(flowEndpoints.getLocalAddr(), flowEndpoints.getRemoteAddr());
    keyEndpoints.setRemoteHostName(flowEndpoints.getRemoteHostName());
    QList<OsProcess> keyOsProcesses;
    const QList<OsProcess>& flowOsProcesses = flow.getOsProcesses();
    foreach (const OsProcess& flowOsProcess, flowOsProcesses) {
        OsProcess keyOsProcess(flowOsProcess.getProgram());
        if (!keyOsProcesses.contains(keyOsProcess)) {
            keyOsProcesses << keyOsProcess;
        }
    }
    CommunicationFlowItemKey result(keyEndpoints, keyOsProcesses);
    return result;
}

bool CommunicationFlowItemKey::operator==(const CommunicationFlowItemKey& rhs) const {
    if(_d == rhs._d)
        return true;
    return _d->endpoints == rhs.getEndpoints()
            && _d->osProcesses == rhs.getOsProcesses();
}

uint qHash(const CommunicationFlowItemKey& key) {
    uint result = qHash(key.getEndpoints());
    foreach (const OsProcess& osProcess, key.getOsProcesses()) {
        result = result ^ qHash(osProcess);
    }
    return result;
}
