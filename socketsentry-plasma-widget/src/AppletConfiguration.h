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

#ifndef APPLETCONFIGURATION_H_
#define APPLETCONFIGURATION_H_

#include "AppletConfigurationData.h"
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>

class KConfigGroup;

/*
 * Manages the storage and retrieval of local and global configuration settings for the applet.
 */
class AppletConfiguration {
public:
    AppletConfiguration();
    virtual ~AppletConfiguration();

    // Connection aggregation strategy for grouping multiple connections into one data row. Insert new values
    // before AggregationModeCount but don't change the order of existing values since this setting is persisted.
    enum AggregationMode {
        NoAggregation,
        HostPairProcess,
        HostPairProgram,
        AggregationModeCount
    };

    // The device this applet instance is monitoring.
    QString getSelectedDevice() const { return _d->selectedDevice; }
    void setSelectedDevice(const QString& selectedDevice) { _d->selectedDevice = selectedDevice; }

    // Size of each column in the flow table. Callers should check the return value against the expected number of
    // table columns and discard the setting here if they don't match. This could happen if no sizes have been
    // persisted or a new applet version changes the number of columns.
    QList<int> getFlowTableColumnSizes() const { return _d->flowTableColumnSizes; }
    void setFlowTableColumnSizes(const QList<int>& columnSizes) { _d->flowTableColumnSizes = columnSizes; }

    // Visual index of each logical column in the flow table. Callers should check the return value against the
    // expected number of table columns and discard the setting here if they don't match. This could happen if
    // no indices have been persisted or a new applet version changes the number of columns.
    QList<int> getFlowTableVisualIndices() const { return _d->flowTableVisualIndices; }
    void setFlowTableVisualIndices(const QList<int>& visualIndices) { _d->flowTableVisualIndices = visualIndices; }

    // Hidden status of each column. Callers should check the return value against the expected number of
    // table columns and discard the setting here if they don't match. This could happen if no data have been
    // persisted or a new applet version changes the number of columns.
    QList<bool> getFlowTableHiddenColumns() const { return _d->flowTableHiddenColumns; }
    void setFlowTableHiddenColumns(const QList<bool>& hiddenColumns) { _d->flowTableHiddenColumns = hiddenColumns; }

    // Flow table sort column. Callers should check to see if the column is in range before applying. This could
    // happen if no value has been persisted or a new applet version changes the number of columns.
    int getFlowTableSortColumn() const { return _d->flowTableSortColumn; }
    void setFlowTableSortColumn(int sortColumn) { _d->flowTableSortColumn = sortColumn; }

    // Flow table sort order.
    Qt::SortOrder getFlowTableSortOrder() const { return _d->flowTableSortOrder; }
    void setFlowTableSortOrder(Qt::SortOrder sortOrder) { _d->flowTableSortOrder = sortOrder; }

    // True if the engine should resolve host names. Else, false.
    bool getResolveNames() const { return _d->resolveNames; }
    void setResolveNames(bool resolveNames) { _d->resolveNames = resolveNames; }

    // True if shared sockets should be attributed to the oldest process (sort oldest-to-newest).
    // False if shared sockets should be attributed to the newest process (sort newest-to-oldest).
    bool getOsProcessSortAscending() const { return _d->osProcessSortAscending; }
    void setOsProcessSortAscending(bool osProcessSortAscending) { _d->osProcessSortAscending = osProcessSortAscending; }

    // Custom filter text in pcap syntax that is applied across devices.
    QString getCustomFilter() const { return _d->customFilter; }
    void setCustomFilter(QString customFilter) { _d->customFilter = customFilter; }

    // The number of subdomain levels to display when name resolution is enabled. Non-positive values
    // mean no limit.
    int getShowSubdomainLevels() const { return _d->showSubdomainLevels; }
    void setShowSubdomainLevels(int showSubdomainLevels) { _d->showSubdomainLevels = showSubdomainLevels; }

    // Connection aggregation strategy for grouping multiple connections into one data row.
    AggregationMode getAggregationMode() const { return (AggregationMode)_d->aggregationMode; }
    void setAggregationMode(AggregationMode aggregationMode) { _d->aggregationMode = (int)aggregationMode; }

    // Read configuration settings from the applet's local and global configuration groups and store them in this object.
    // If some settings are missing, defaults are applied for those instead.
    void importConfig(const KConfigGroup& localConfig, const KConfigGroup& globalConfig);

    // Write configuration settings from this object to the applet's local and global configuration groups.
    void exportConfig(KConfigGroup& localConfig, KConfigGroup& globalConfig) const;

private:
    QSharedDataPointer<AppletConfigurationData> _d;

    // Setting keys.
    static const QString SELECTED_DEVICE;
    static const QString FLOW_TABLE_COLUMN_SIZES;
    static const QString FLOW_TABLE_HIDDEN_COLUMNS;
    static const QString FLOW_TABLE_VISUAL_INDICES;
    static const QString FLOW_TABLE_SORT_COLUMN;
    static const QString FLOW_TABLE_SORT_ORDER;
    static const QString RESOLVE_NAMES;
    static const QString OS_PROCESS_SORT_ASCENDING;
    static const QString CUSTOM_FILTER;
    static const QString SHOW_SUBDOMAIN_LEVELS;
    static const QString AGGREGATION_MODE;
};

#endif /* APPLETCONFIGURATION_H_ */
