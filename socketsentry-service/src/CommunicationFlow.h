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

#ifndef COMMUNICATIONFLOW_H_
#define COMMUNICATIONFLOW_H_

#include "CommunicationFlowData.h"
#include "OsProcess.h"

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QList>

class QDBusArgument;

/*
 * Details a bidirectional connection or group of connections between endpoints at a point in time. A flow includes
 * an endpoint pair that identifies the parties, a list of OS processes or programs responsible for the communication,
 * and a set of metrics and derived statistics that describe the traffic.
 *
 * Usually, only one process or program is associated with a flow. However, in the case of sockets shared by multiple
 * processes, the flow records them all in an order determined by the application. The first element in the list is the
 * best guess at which one is responsible for the traffic, so there is a special getter for it.
 *
 * Communication flows can represent a single connection or they can aggregate many connections. To aggregate
 * a new connection into this flow item, call its "combineConnections" method.
 *
 * This type can be used as a D-Bus argument or return value.
 */
class CommunicationFlow {
public:
    CommunicationFlow();
    CommunicationFlow(const IpEndpointPair& ipEndpointPair, const OsProcess& osProcess,
            const FlowMetrics& flowMetrics, const FlowStatistics& flowStatistics);
    CommunicationFlow(const IpEndpointPair& ipEndpointPair, const QList<OsProcess>& osProcesses,
            const FlowMetrics& flowMetrics, const FlowStatistics& flowStatistics);
    virtual ~CommunicationFlow();

    // Two flows are equal if their component parts are equal.
    bool operator==(const CommunicationFlow& rhs) const;

    // The endpoint pair responsible for the communication flow.
    IpEndpointPair getIpEndpointPair() const { return _d->ipEndpointPair; }
    void setIpEndpointPair(const IpEndpointPair& ipEndpointPair) { _d->ipEndpointPair = ipEndpointPair; }

    // The measured traffic metrics.
    FlowMetrics getFlowMetrics() const { return _d->flowMetrics; }
    void setFlowMetrics(const FlowMetrics& flowMetrics) { _d->flowMetrics = flowMetrics; }

    // The derived traffic statistics.
    FlowStatistics getFlowStatistics() const { return _d->flowStatistics; }
    void setFlowStatistics(const FlowStatistics& flowStatistics) { _d->flowStatistics = flowStatistics; }

    // The OS processes responsible for the communication. Usually, there's just one unless it's a
    // shared socket. If only the program names are given, then each element represents a group of
    // processes running the same program.
    QList<OsProcess> getOsProcesses() const { return _d->osProcesses;}

    // Add another process to the process list.
    void addOsProcess(const OsProcess& osProcess) { _d->osProcesses.append(osProcess); }

    // The first process in the list, which is our best guess at which one is responsible for traffic.
    OsProcess getFirstOsProcess() const {
        return (_d->osProcesses.isEmpty()) ? OsProcess() : _d->osProcesses[0];
    }

    // True if there is more than one process in the process list.
    bool isSharedSocket() const { return _d->osProcesses.size() > 1; }

    // Combine one or more additional connections from another flow into this flow. The metrics and statistics
    // of the other flow are merged into this flow.
    void combineConnections(const CommunicationFlow& other);

    // Return a string suitable for debugging, but not for end-user consumption.
    virtual QString toString() const;

private:
    QSharedDataPointer<CommunicationFlowData> _d;
};

// DBUS argument marshalling.
QDBusArgument& operator<<(QDBusArgument& argument, const CommunicationFlow& obj);
// DBUS argument unmarshalling.
const QDBusArgument& operator>>(const QDBusArgument& argument, CommunicationFlow& obj);

// Make visible as a D-Bus data type.
Q_DECLARE_METATYPE(CommunicationFlow)
Q_DECLARE_METATYPE(QList<CommunicationFlow>)

#endif /* COMMUNICATIONFLOW_H_ */
