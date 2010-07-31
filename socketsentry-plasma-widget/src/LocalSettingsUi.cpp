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

#include "LocalSettingsUi.h"
#include "AppletConfiguration.h"
#include "ColumnListWidgetItem.h"

#include <QtCore/QMap>
#include <QtGui/QListWidget>

LocalSettingsUi::LocalSettingsUi(const QStringList& devices, const QStringList& columns, QWidget* parent) :
    QWidget(parent), _columns(columns), _devices(devices) {
    _ui.setupUi(this);
}

LocalSettingsUi::~LocalSettingsUi() {
}

void LocalSettingsUi::readConfiguration(const AppletConfiguration& config) {

    // Set up list of network devices.
    if (!_devices.isEmpty()) {
        _ui.selectedDeviceComboBox->addItems(_devices);
    }
    if (!_devices.contains(config.getSelectedDevice())) {
        // Configured device doesn't exist, but make it an option anyway in case it's simply not
        // present at the moment.
        _ui.selectedDeviceComboBox->addItem(config.getSelectedDevice());
    }
    _ui.selectedDeviceComboBox->setCurrentIndex(_ui.selectedDeviceComboBox->findText(config.getSelectedDevice()));

    // Filter-sort controls toggle.
    _ui.showFilterSortCheckBox->setChecked(config.getShowFilterSortControls());

    // Aggregation selection.
    switch (config.getAggregationMode()) {
    case AppletConfiguration::HostPairProcess:
        _ui.hostPairProcessRadioButton->setChecked(true);
        break;
    case AppletConfiguration::HostPairProgram:
        _ui.hostPairProgramRadioButton->setChecked(true);
        break;
    default:
        _ui.connectionRadioButton->setChecked(true);
    }

    // Subdomain levels selection.
    switch (config.getShowSubdomainLevels()) {
    case 1:
        _ui.subdomainLevels1Button->setChecked(true);
        break;
    case 2:
        _ui.subdomainLevels2Button->setChecked(true);
        break;
    case 3:
        _ui.subdomainLevels3Button->setChecked(true);
        break;
    default:
        _ui.subdomainLevelsAllButton->setChecked(true);
    }

    // Add columns to selector. If column is currently hidden, it is added to the "available" list.
    // Else, it is added to the selected list, which is sorted by visual index.
    const QList<bool>& hiddenColumns = config.getFlowTableHiddenColumns();
    const QList<int>& visualIndices = config.getFlowTableVisualIndices();
    Q_ASSERT(_columns.size() == visualIndices.size() && _columns.size() == hiddenColumns.size());
    QMap<int, ColumnListWidgetItem*> selectedItemsByVisualIdx;
    for (int i = 0; i < _columns.size(); i++) {
        ColumnListWidgetItem* item =  new ColumnListWidgetItem(_columns[i], i);
        if (hiddenColumns[i]) {
            // Column is hidden. Add to available list.
            _ui.columnsSelector->availableListWidget()->addItem(item);
        } else {
            // Column is showing. Order by visual index. We'll add it to the list widget afterwards.
            selectedItemsByVisualIdx.insert(visualIndices[i], item);
        }
    }
    // Add selected columns ordered by visual index.
    const QList<ColumnListWidgetItem*> orderedItems = selectedItemsByVisualIdx.values();
    foreach (ColumnListWidgetItem* item, orderedItems) {
        _ui.columnsSelector->selectedListWidget()->addItem(item);
    }
    _ui.columnsSelector->availableListWidget()->sortItems(Qt::AscendingOrder);

}

void LocalSettingsUi::writeConfiguration(AppletConfiguration& config) {
    // Selected device.
    QString selectedDevice = _ui.selectedDeviceComboBox->currentText();
    if (!selectedDevice.isEmpty()) {
        config.setSelectedDevice(selectedDevice);
    }

    // Filter-sort controls toggle.
    config.setShowFilterSortControls(_ui.showFilterSortCheckBox->isChecked());

    // Aggregation mode.
    if (_ui.hostPairProcessRadioButton->isChecked()) {
        config.setAggregationMode(AppletConfiguration::HostPairProcess);
    } else if (_ui.hostPairProgramRadioButton->isChecked()) {
        config.setAggregationMode(AppletConfiguration::HostPairProgram);
    } else {
        config.setAggregationMode(AppletConfiguration::NoAggregation);
    }

    // Subdomain levels.
    if (_ui.subdomainLevels1Button->isChecked()) {
        config.setShowSubdomainLevels(1);
    } else if (_ui.subdomainLevels2Button->isChecked()) {
        config.setShowSubdomainLevels(2);
    } else if (_ui.subdomainLevels3Button->isChecked()) {
        config.setShowSubdomainLevels(3);
    } else {
        config.setShowSubdomainLevels(0);
    }

    // Get selected column names in order.
    QStringList selectedColumns;
    for (int i = 0; i < _ui.columnsSelector->selectedListWidget()->count(); i++) {
        QListWidgetItem* item = _ui.columnsSelector->selectedListWidget()->item(i);
        selectedColumns << item->text();
    }

    // Set hidden column list.
    QList<bool> hiddenColumns;
    for (int i = 0; i < _columns.size(); i++) {
        hiddenColumns << !selectedColumns.contains(_columns[i]);
    }
    config.setFlowTableHiddenColumns(hiddenColumns);

    // Create list of all columns with gaps for selected columns.
    QStringList orderedColumns;
    for (int i = 0; i < _columns.size(); i++) {
        if (selectedColumns.contains(_columns[i])) {
            // Column is selected. Add placeholder.
            orderedColumns << QString();
        } else {
            orderedColumns << _columns[i];
        }
    }

    // Fill in gaps with selected columns in the order that the user gave us.
    int index = 0;
    while ((index = orderedColumns.indexOf(QString())) >= 0) {
        Q_ASSERT(!selectedColumns.isEmpty());
        orderedColumns[index] = selectedColumns.takeFirst();
    }

    // Now we have the order the columns should appear. Set visual indices.
    QList<int> visualIndices;
    for (int i = 0; i < _columns.size(); i++) {
        int visualIndex = orderedColumns.indexOf(_columns[i]);
        Q_ASSERT(visualIndex >= 0);
        visualIndices << visualIndex;
    }
    config.setFlowTableVisualIndices(visualIndices);

}
