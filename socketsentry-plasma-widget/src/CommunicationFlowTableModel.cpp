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

#include "CommunicationFlowTableModel.h"
#include "CommunicationFlowItem.h"
#include "CommunicationFlowItemKey.h"
#include "CommunicationFlow.h"
#include "OsProcess.h"
#include "IpEndpointPair.h"
#include "FlowStatistics.h"
#include "AppletConfiguration.h"

#include <QtCore/QVariant>
#include <QtCore/QDebug>
#include <QtCore/QHash>
#include <QtCore/QHashIterator>
#include <QtCore/QSet>
#include <QtCore/QDateTime>
#include <KDE/KLocalizedString>

// Role corrsponding to the data value to be used in sorting.
const int CommunicationFlowTableModel::SortRole = Qt::UserRole + 0;
// Role corresponding to the unabridged name of a section header, which never changes.
const int CommunicationFlowTableModel::FullNameRole = Qt::UserRole + 1;

// Lower bound on the amount of time we keep a communication flow item in the model even if the underlying endpoints
// disappear in flow updates.
const int CommunicationFlowTableModel::MIN_ITEM_AGE_MS = 6000;

// Icon to show when no program icon is available.
const KIcon CommunicationFlowTableModel::DefaultProgramIcon = KIcon("network-wired-activated");

CommunicationFlowTableModel::CommunicationFlowTableModel(QObject* parent) :
    QAbstractItemModel(parent), _showSubdomainLevels(0), _aggregationMode(AppletConfiguration::NoAggregation) {

}

CommunicationFlowTableModel::~CommunicationFlowTableModel() {
}

void CommunicationFlowTableModel::updateCommunicationFlows(const QList<CommunicationFlow>& newFlows) {
    // Index new flows by endpoint pairs.
    QHash<IpEndpointPair, int> newEndpointsTable;
    for (int i = 0; i < newFlows.size(); i++) {
        newEndpointsTable.insert(newFlows[i].getIpEndpointPair(), i);
    }

    // Update or remove existing flows based on what's in the new set.
    QDateTime nowUtc = QDateTime::currentDateTime().toUTC();
    for (int i = 0; i < _allFlowItems.size(); i++) {
        const IpEndpointPair& endpoints = _allFlowItems[i].actual().getIpEndpointPair();
        if (newEndpointsTable.contains(endpoints)) {
            // Update to an existing flow.
            const CommunicationFlow& newFlow = newFlows[newEndpointsTable[endpoints]];
            _allFlowItems[i].update(newFlow);
        } else {
            // This endpoint pair has gone away. Remove it if it meets the age criteria.
            const QDateTime& firstSeenUtc = _allFlowItems[i].getFirstSeenUtc();
            if (nowUtc >= firstSeenUtc.addMSecs(MIN_ITEM_AGE_MS)) {
                // The item is old enough to be removed.
                _allFlowItems.removeAt(i);
                i--;
            } else if (!_allFlowItems[i].isZombie()) {
                // The flow isn't old enough to be removed. Turn it into a zombie.
                _allFlowItems[i].zombify();
            }
        }
        // Finished processing this endpoint pair.
        newEndpointsTable.remove(endpoints);
    }

    // All that should be left in the new endpoints table now are endpoints that weren't in the last update.
    if (!newEndpointsTable.isEmpty()) {
        // Add new flow items for new endpoints.
        QHashIterator<IpEndpointPair, int> i(newEndpointsTable);
        while (i.hasNext()) {
            i.next();
            const CommunicationFlow& newFlow = newFlows[i.value()];
            CommunicationFlowItem newItem(newFlow);
            _allFlowItems.append(newItem);
        }
    }

    QList<CommunicationFlowItem> newAggregatedItems = aggregateFlows();
    replaceAggregatedItems(newAggregatedItems);
    emit flowUpdateCompleted();
}

QList<CommunicationFlowItem> CommunicationFlowTableModel::aggregateFlows() const {
    QList<CommunicationFlowItem> result;
    if (_aggregationMode == AppletConfiguration::NoAggregation) {
        result = _allFlowItems;
    } else {
        // Combine flows with the same aggregation key.
        QHash<CommunicationFlowItemKey, int> resultIndex;
        bool groupByProgram = (_aggregationMode == AppletConfiguration::HostPairProgram);
        foreach (const CommunicationFlowItem& srcItem, _allFlowItems) {
            CommunicationFlowItemKey key = groupByProgram
                ? CommunicationFlowItemKey::fromHostPairAndProgram(srcItem.actual())
                : CommunicationFlowItemKey::fromHostPairAndProcess(srcItem.actual());
            // Have we seen this key before?
            if (resultIndex.contains(key)) {
                // Yes! Combine the source flow with the existing aggregated flow.
                CommunicationFlowItem& destItem = result[resultIndex[key]];
                destItem.combineConnections(srcItem);
            } else {
                // No. Create new aggregated flow item and add to result.
                const CommunicationFlow& srcFlow = srcItem.actual();
                CommunicationFlow destFlow(key.getEndpoints(), key.getOsProcesses(),
                        srcFlow.getFlowMetrics(), srcFlow.getFlowStatistics());
                CommunicationFlowItem destItem(destFlow);
                destItem.setFirstSeenUtc(srcItem.getFirstSeenUtc());
                int newIndex = result.size();
                result << destItem;
                // Add key to the index in case more source flows will be aggregated into this one.
                resultIndex.insert(key, newIndex);
            }
        }
    }
    // Prepare all items for display.
    for (int i = 0; i < result.size(); i++) {
        CommunicationFlowItem& newItem = result[i];
        newItem.prepare();
    }
    return result;
}

void CommunicationFlowTableModel::replaceAggregatedItems(const QList<CommunicationFlowItem>& newAggregatedItems) {
    // For Qt 4.6+, we could probably just emit begin/end model reset signals, but to be friendly
    // to 4.5 and below, we'll use the row-oriented signals. First, replace as many existing rows as we can.
    int row = 0;
    for (; row < newAggregatedItems.size() && row < _aggregatedFlowItems.size(); row++) {
        _aggregatedFlowItems.replace(row, newAggregatedItems[row]);
    }
    if (row > 0) {
        emit dataChanged(index(0, 0), index(row - 1, ColumnCount - 1));
    }
    if (newAggregatedItems.size() > _aggregatedFlowItems.size()) {
        // Insert additional rows from the update.
        beginInsertRows(QModelIndex(), row, newAggregatedItems.size() - 1);
        _aggregatedFlowItems += newAggregatedItems.mid(row, -1);
        endInsertRows();
    } else if (newAggregatedItems.size() < _aggregatedFlowItems.size()) {
        // Remove old rows.
        beginRemoveRows(QModelIndex(), row, _aggregatedFlowItems.size() - 1);
        _aggregatedFlowItems = _aggregatedFlowItems.mid(0, row);
        endRemoveRows();
    }
}

QVariant CommunicationFlowTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.row() < _aggregatedFlowItems.size() && index.column() < ColumnCount) {
        const CommunicationFlowItem& item = _aggregatedFlowItems[index.row()];
        const CommunicationFlow& actual = item.actual();
        const QString& user = actual.getFirstOsProcess().getUser();
        switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
            case ProcessOrProgramColumn:
                return QString("%1%2")
                        .arg(actual.getFirstOsProcess().getFormattedSummary())
                        .arg(actual.isSharedSocket() ? "*" : "");
            case RateColumn:
                return formatRate(actual.getFlowStatistics().getRecentBytesPerSec());
            case RemoteHostColumn:
                return actual.getIpEndpointPair().getFormattedRemoteEndpoint(_showSubdomainLevels);
            case TransportColumn:
                return actual.getIpEndpointPair().getTransportName();
            case LocalEndpointColumn:
                return actual.getIpEndpointPair().getLocalEndpoint();
            case RemoteEndpointColumn:
                return actual.getIpEndpointPair().getRemoteEndpoint();
            case NumConnections:
                return item.getNumConnections();
            case UserColumn:
                return user.isEmpty() ? "-" : QString("%1%2").arg(user).arg(actual.isSharedSocket() ? "*" : "");
            case InRate:
                return formatRate(actual.getFlowStatistics().getRecentBytesInPerSec());
            case OutRate:
                return formatRate(actual.getFlowStatistics().getRecentBytesOutPerSec());
            case RecentPeakRate:
                return formatRate(actual.getFlowStatistics().getPeakBytesPerSec());
            case FirstSeen:
                return item.getFirstSeenUtc().toLocalTime().toString("hh:mm:ss d-MMM");
            default:
                return QVariant();
            }
        case CommunicationFlowTableModel::SortRole:
            switch (index.column()) {
            case DeviceStateColumn:
                return (int)item.getDeviceState();
            case ProcessOrProgramColumn:
                return actual.getFirstOsProcess().getFormattedSummary();
            case RateColumn:
                return actual.getFlowStatistics().getRecentBytesPerSec();
            case RemoteHostColumn:
                return actual.getIpEndpointPair().getFormattedRemoteEndpoint();
            case TransportColumn:
                return actual.getIpEndpointPair().getTransportName();
            case LocalEndpointColumn:
                return actual.getIpEndpointPair().getLocalEndpoint();
            case RemoteEndpointColumn:
                return actual.getIpEndpointPair().getRemoteEndpoint();
            case NumConnections:
                return item.getNumConnections();
            case UserColumn:
                return actual.getFirstOsProcess().getUser();
            case InRate:
                return actual.getFlowStatistics().getRecentBytesInPerSec();
            case OutRate:
                return actual.getFlowStatistics().getRecentBytesOutPerSec();
            case RecentPeakRate:
                return actual.getFlowStatistics().getPeakBytesPerSec();
            case FirstSeen:
                return item.getFirstSeenUtc();
            default:
                return QVariant();
            }
        case Qt::DecorationRole:
            switch (index.column()) {
            case DeviceStateColumn:
                return item.getDeviceStateIcon();
            case ProcessOrProgramColumn:
                return item.getProgramIcon().isNull() ? DefaultProgramIcon : item.getProgramIcon();
            default:
                return QVariant();
            }
        case Qt::ToolTipRole:
            return createToolTipText(index.row());
        case Qt::FontRole:
            return item.getFont();
        default:
            return QVariant();
        }
    }
    return QVariant();
}

QString CommunicationFlowTableModel::createToolTipText(int row) const {
    QString result;
    if (row >= 0 && row < _aggregatedFlowItems.size()) {
        // Append all column vlaues.
        for (int i = 0; i < ColumnCount; i++) {
            QModelIndex index = createIndex(row, i, 0);
            QVariant displayVariant = data(index, Qt::DisplayRole);
            if (!displayVariant.isNull()) {
                QString columnValue = displayVariant.value<QString>();
                QString heading = headerData(i, Qt::Horizontal, Qt::DisplayRole).value<QString>();
                QString lineItem("%1:\t%2");
                result += lineItem.arg(heading).arg(columnValue);
                if (i < ColumnCount - 1) {
                    result += '\n';
                }
            }
        }

        if (_aggregatedFlowItems[row].actual().isSharedSocket()) {
            // Multiple processes share the socket. Show info about all known ones.
            result += i18n("\n\n* Connection shared with:\n");
            const QList<OsProcess>& osProcesses = _aggregatedFlowItems[row].actual().getOsProcesses();
            for (int i = 1; i < osProcesses.size(); i++) {
                result += QString("%1\t%2")
                    .arg(headerData(ProcessOrProgramColumn, Qt::Horizontal, Qt::DisplayRole).value<QString>())
                    .arg(osProcesses[i].getFormattedSummary());
                if (i < osProcesses.size() - 1) {
                    result += '\n';
                }
            }
        }
    }

    return result;
}

QVariant CommunicationFlowTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(orientation);
    if (role == Qt::DisplayRole) {
        switch(section) {
        case DeviceStateColumn:
            return i18n("State");
        case ProcessOrProgramColumn:
            return (_aggregationMode == AppletConfiguration::HostPairProgram) ? i18n("Program") : i18n("Process");
        case RateColumn:
            return i18n("Rate");
        case RemoteHostColumn:
            return i18n("Host");
        case TransportColumn:
            return i18n("Proto");
        case LocalEndpointColumn:
            return i18n("Local end");
        case RemoteEndpointColumn:
            return i18n("Remote end");
        case NumConnections:
            return i18n("Conns");
        case UserColumn:
            return i18n("User");
        case InRate:
            return i18n("In rate");
        case OutRate:
            return i18n("Out rate");
        case RecentPeakRate:
            return i18n("Recent peak");
        case FirstSeen:
            return i18n("First seen");
        default:
            return QVariant();
        }
    } else if (role == FullNameRole) {
        switch(section) {
        case TransportColumn:
            return i18n("Protocol");
        case ProcessOrProgramColumn:
            return i18n("Process/Program");
        case NumConnections:
            return i18n("Connections");
        default:
            return headerData(section, orientation, Qt::DisplayRole);
        }
    } else {
        return QVariant();
    }
}

QString CommunicationFlowTableModel::formatRate(qlonglong bytesPerSec) const {
    // A map would be cleaner here, but not as efficient.
    const qlonglong oneKb = (qlonglong)1024;
    const qlonglong oneMb = (qlonglong)1024 * 1024;
    const qlonglong oneGb = (qlonglong)1024 * 1024 * 1024;
    const qlonglong oneTb = (qlonglong)1024 * 1024 * 1024 * 1024;

    if (bytesPerSec >= oneTb) {
        return i18n("%L1 TB/s").arg((double)bytesPerSec / (double)oneTb, 0, 'f', 1);
    } else if (bytesPerSec >= oneGb) {
        return i18n("%L1 GB/s").arg((double)bytesPerSec / (double)oneGb, 0, 'f', 1);
    } else if (bytesPerSec >= oneMb) {
        return i18n("%L1 MB/s").arg((double)bytesPerSec / (double)oneMb, 0, 'f', 1);
    } else if (bytesPerSec >= oneKb) {
        return i18n("%L1 KB/s").arg((double)bytesPerSec / (double)oneKb, 0, 'f', 1);
    } else {
        return i18n("%L1 B/s").arg(bytesPerSec);
    }
}

void CommunicationFlowTableModel::readConfiguration(const AppletConfiguration& newConfig) {
    _showSubdomainLevels = newConfig.getShowSubdomainLevels();
    _aggregationMode = newConfig.getAggregationMode();
    headerDataChanged(Qt::Horizontal, 0, ColumnCount - 1);  // header names change based on aggregation mode
}
