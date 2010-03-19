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

#include "CommunicationFlow.h"

#include <QtNetwork/QHostAddress>
#include <QtCore/QString>
#include <QtDBus/QDBusArgument>

CommunicationFlow::CommunicationFlow() {
    _d = new CommunicationFlowData();
}

CommunicationFlow::CommunicationFlow(const IpEndpointPair& ipEndpointPair, const OsProcess& osProcess,
        const FlowMetrics& flowMetrics, const FlowStatistics& flowStatistics) {
    _d = new CommunicationFlowData();
    _d->ipEndpointPair = ipEndpointPair;
    _d->flowMetrics = flowMetrics;
    _d->flowStatistics = flowStatistics;
    _d->osProcesses.append(osProcess);
}

CommunicationFlow::CommunicationFlow(const IpEndpointPair& ipEndpointPair, const QList<OsProcess>& osProcesses,
        const FlowMetrics& flowMetrics, const FlowStatistics& flowStatistics) {
    _d = new CommunicationFlowData();
    _d->ipEndpointPair = ipEndpointPair;
    _d->flowMetrics = flowMetrics;
    _d->flowStatistics = flowStatistics;
    _d->osProcesses = osProcesses;
}

CommunicationFlow::~CommunicationFlow() {
}

bool CommunicationFlow::operator==(const CommunicationFlow& rhs) const {
    if (_d == rhs._d)
        return true;
    return _d->ipEndpointPair == rhs.getIpEndpointPair()
            && _d->osProcesses == rhs.getOsProcesses()
            && _d->flowMetrics == rhs.getFlowMetrics()
            && _d->flowStatistics == rhs.getFlowStatistics();
}

void CommunicationFlow::combineConnections(const CommunicationFlow& other) {
    _d->flowMetrics.combineConnections(other.getFlowMetrics());
    _d->flowStatistics.combineConnections(other.getFlowStatistics());
}

QString CommunicationFlow::toString() const {
    bool sharedSocket = _d->osProcesses.size() > 1;
    QString result = QString("%1 %2 %3 %4")
        .arg(_d->ipEndpointPair.toString())
        .arg(getFirstOsProcess().toString(sharedSocket))
        .arg(_d->flowMetrics.toString())
        .arg(_d->flowStatistics.toString());
    return result;
}

QDBusArgument& operator<<(QDBusArgument& arg, const CommunicationFlow& obj) {
    arg.beginStructure();
    arg << obj.getIpEndpointPair() << obj.getFlowMetrics() << obj.getFlowStatistics() << obj.getOsProcesses();
    arg.endStructure();
    return arg;
}

const QDBusArgument& operator>>(const QDBusArgument& arg, CommunicationFlow& obj) {
    arg.beginStructure();

    IpEndpointPair ipEndpointPair;
    arg >> ipEndpointPair;
    obj.setIpEndpointPair(ipEndpointPair);

    FlowMetrics flowMetrics;
    arg >> flowMetrics;
    obj.setFlowMetrics(flowMetrics);

    FlowStatistics flowStatistics;
    arg >> flowStatistics;
    obj.setFlowStatistics(flowStatistics);

    QList<OsProcess> osProcesses;
    arg >> osProcesses;
    foreach (OsProcess osProcess, osProcesses) {
        obj.addOsProcess(osProcess);
    }

    arg.endStructure();
    return arg;
}

