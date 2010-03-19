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

#ifndef COMMUNICATIONFLOWTABLEMODEL_H_
#define COMMUNICATIONFLOWTABLEMODEL_H_

#include "AppletConfiguration.h"

#include <KDE/KIcon>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>
#include <QtCore/QObject>

class CommunicationFlowItem;
class CommunicationFlow;
class AppletConfiguration;

/*
 * Model for a tabular display of communication flows.
 */
class CommunicationFlowTableModel : public QAbstractItemModel {
    Q_OBJECT

public:
    CommunicationFlowTableModel(QObject* parent = 0);
    virtual ~CommunicationFlowTableModel();

    // All indexes are enabled, but can't be selected or edited.
    virtual Qt::ItemFlags flags(const QModelIndex& index) const {
        Q_UNUSED(index);
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    // Row count is equal to the current number of communication flow items.
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const {
        return parent.isValid() ? 0 : _aggregatedFlowItems.size();
    }

    // Column count.
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const {
        Q_UNUSED(parent);
        return parent.isValid() ? 0 : ColumnCount;
    }

    // Return an index for the given row, column, and parent index.
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const {
        Q_UNUSED(parent);
        return createIndex(row, column, 0);
    }

    // Return an index for the parent of the given index.
    QModelIndex parent(const QModelIndex& index ) const {
        Q_UNUSED(index);
        return QModelIndex();
    }

    // Return header data for this model.
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    // Return the data at the given index and role. For the display role, the value corresponding to each
    // column given in the CommunicationFlowColumn enum is returned. All columns have a display value
    // except the first column ("device state") which has no text.
    //
    // For the decoration role, the first column maps to an icon representing the current device
    // state. The process column maps to a program icon. All other rows/columns are invalid.
    //
    // For the tooltip role, all columns map to descriptive text that gives detail about the flow, which
    // is helpful when columns are hidden.
    //
    // For the font role, all columns map to a weight that is proportional to the ratio of
    // the current transfer rate to the peak rate.
    //
    // For the sort role, which is a custom role, each column maps to a value that can be used for
    // sorting rows on that column.
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole ) const;

    // Role corrsponding to the data value to be used in sorting.
    static const int SortRole;

    // Role corresponding to the unabridged name of a section header, which never changes.
    static const int FullNameRole;

    // The enum of columns in this model.
    enum Column {
        DeviceStateColumn = 0,
        ProcessOrProgramColumn,
        RateColumn,
        RemoteHostColumn,
        TransportColumn,
        LocalEndpointColumn,
        RemoteEndpointColumn,
        NumConnections,
        UserColumn,
        InRate,
        OutRate,
        RecentPeakRate,
        FirstSeen,
        ColumnCount
    };

signals:
    // Emitted after the model has successfully updated itself from a new set of communication flows.
    void flowUpdateCompleted();

public slots:
    // Receive a new set of flows and incorporate them into the model. Each flow must have a unique endpoint pair,
    // which are matched against endpoint pairs of this model's current list of all non-aggregated flow items.
    // For endpoints that are present in the current items and the update, the current item is updated in place.
    // Current endpoints that are not present in the update are either removed or turned into zombies depending
    // on their age. (Zombies are very short-lived connections that persist longer in the model so that the user
    // gets a chance to see them.) Finally, endpoints that are in the update but not in the current list of flows
    // are added to the end.
    //
    // Once the new non-aggregated list of flows is computed for all endpoint pairs in the update, the second
    // step is to aggregate this data according to the current aggregation mode. The output of the aggregation
    // becomes the publicly exposed rows and columns of this model. We invoke standard row insert/update/remove
    // signals/methods mandated by the base class contract to let our consumers know about these changes. And
    // then finally, we emit flowUpdateCompleted to let others know that the entire update cycle has finished.
    void updateCommunicationFlows(const QList<CommunicationFlow>& newFlows);

    // Updates the model to reflect configuration changes.
    void readConfiguration(const AppletConfiguration& newConfig);

private:
    // Lower bound on the amount of time we keep a communication flow item in the model even if the underlying endpoints
    // disappear in flow updates.
    static const int MIN_ITEM_AGE_MS;

    // Create descriptive text about the flow specified by the row suitable for use in a tooltip.
    QString createToolTipText(int row) const;

    // Returns a bytes-per-second value as a human readable string.
    QString formatRate(qlonglong bytesPerSec) const;

    // Replace the aggregated flow items of this model with the new list We invoke standard row insert/update/remove
    // signals/methods mandated by the base class contract to let our consumers know about these changes.
    void replaceAggregatedItems(const QList<CommunicationFlowItem>& newAggregatedItems);

    // Aggregate this model's current list of non-aggregated communication flow items using the current aggregation mode
    // and return the result. If the aggregation mode is NoAggregation, then no aggregation takes place. Otherwise,
    // the list of non-aggregated communication flows are grouped by the criteria named in the aggregation mode and
    // metrics and statistics are combined for connections in each group. Before returning, the method invokes "prepare"
    // on each item in the result, which prepares it for rendering in the view.
    QList<CommunicationFlowItem> aggregateFlows() const;

    // The latest flow items, non-aggregated.
    QList<CommunicationFlowItem> _allFlowItems;

    // The latest flow items, aggregated. These are the public items exposed by this model.
    QList<CommunicationFlowItem> _aggregatedFlowItems;

    // The number of subdomain levels to show for host names. If not positive, then the number is unlimited.
    int _showSubdomainLevels;

    // The current connection aggregation mode.
    AppletConfiguration::AggregationMode _aggregationMode;

    // Icon to show when no program icon is available.
    static const KIcon DefaultProgramIcon;

};

#endif /* COMMUNICATIONFLOWTABLEMODEL_H_ */
