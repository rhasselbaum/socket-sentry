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

#include "AppletConfiguration.h"
#include <KDE/KConfigGroup>

// Setting keys.
const QString AppletConfiguration::SELECTED_DEVICE = "selectedDevice";
const QString AppletConfiguration::FLOW_TABLE_COLUMN_SIZES = "flowTableColumnSizes";
const QString AppletConfiguration::FLOW_TABLE_HIDDEN_COLUMNS = "flowTableHiddenColumns";
const QString AppletConfiguration::FLOW_TABLE_VISUAL_INDICES = "flowTableVisualIndices";
const QString AppletConfiguration::FLOW_TABLE_SORT_COLUMN = "flowTableSortColumn";
const QString AppletConfiguration::FLOW_TABLE_SORT_ORDER = "flowTableSortOrder";
const QString AppletConfiguration::RESOLVE_NAMES = "resolveNames";
const QString AppletConfiguration::OS_PROCESS_SORT_ASCENDING = "osProcessSortAscending";
const QString AppletConfiguration::CUSTOM_FILTER = "customFilter";
const QString AppletConfiguration::SHOW_SUBDOMAIN_LEVELS = "showSubdomainLevels";
const QString AppletConfiguration::AGGREGATION_MODE = "aggregationMode";

// Name of the default pseudo-device, which captures traffic on all interfaces.
const QString AppletConfiguration::DEFAULT_DEVICE_NAME = "any";

AppletConfiguration::AppletConfiguration() {
    _d = new AppletConfigurationData();
}

AppletConfiguration::~AppletConfiguration() {
}

void AppletConfiguration::importConfig(const KConfigGroup& localConfig, const KConfigGroup& globalConfig) {
    // Local config.
    _d->selectedDevice = localConfig.readEntry(SELECTED_DEVICE, DEFAULT_DEVICE_NAME);
    _d->flowTableColumnSizes = localConfig.readEntry(FLOW_TABLE_COLUMN_SIZES, QList<int>());
    _d->flowTableHiddenColumns = localConfig.readEntry(FLOW_TABLE_HIDDEN_COLUMNS, QList<bool>());
    _d->flowTableVisualIndices = localConfig.readEntry(FLOW_TABLE_VISUAL_INDICES, QList<int>());
    _d->flowTableSortColumn = localConfig.readEntry(FLOW_TABLE_SORT_COLUMN, -1);
    _d->flowTableSortOrder = localConfig.readEntry(FLOW_TABLE_SORT_ORDER, true) ? Qt::AscendingOrder : Qt::DescendingOrder;
    _d->showSubdomainLevels = localConfig.readEntry(SHOW_SUBDOMAIN_LEVELS, 2);

    // Read and validate aggregation mode ordinal.
    int aggregationModeOrdinal = localConfig.readEntry(AGGREGATION_MODE, (int)HostPairProcess);
    if (aggregationModeOrdinal >= 0 && aggregationModeOrdinal < AggregationModeCount) {
        _d->aggregationMode = aggregationModeOrdinal;
    } else {
        _d->aggregationMode = NoAggregation;
    }

    // Global config.
    _d->resolveNames = globalConfig.readEntry(RESOLVE_NAMES, false);
    _d->osProcessSortAscending = globalConfig.readEntry(OS_PROCESS_SORT_ASCENDING, true);
    _d->customFilter = globalConfig.readEntry(CUSTOM_FILTER, QString());
}

void AppletConfiguration::exportConfig(KConfigGroup& localConfig, KConfigGroup& globalConfig) const {
    // Local config.
    localConfig.writeEntry(SELECTED_DEVICE, _d->selectedDevice);
    localConfig.writeEntry(FLOW_TABLE_COLUMN_SIZES, _d->flowTableColumnSizes);
    localConfig.writeEntry(FLOW_TABLE_HIDDEN_COLUMNS, _d->flowTableHiddenColumns);
    localConfig.writeEntry(FLOW_TABLE_VISUAL_INDICES, _d->flowTableVisualIndices);
    localConfig.writeEntry(FLOW_TABLE_SORT_COLUMN, _d->flowTableSortColumn);
    localConfig.writeEntry(FLOW_TABLE_SORT_ORDER, (_d->flowTableSortOrder == Qt::AscendingOrder));
    localConfig.writeEntry(SHOW_SUBDOMAIN_LEVELS, _d->showSubdomainLevels);
    localConfig.writeEntry(AGGREGATION_MODE, _d->aggregationMode);

    // Global config.
    globalConfig.writeEntry(RESOLVE_NAMES, _d->resolveNames);
    globalConfig.writeEntry(OS_PROCESS_SORT_ASCENDING, _d->osProcessSortAscending);
    globalConfig.writeEntry(CUSTOM_FILTER, _d->customFilter);
}
