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

#include "NetworkDeviceWidget.h"
#include "AppletConfiguration.h"
#include "CommunicationFlow.h"
#include "CommunicationFlowTableView.h"
#include "CommunicationFlowTableModel.h"
#include "CommunicationFlowSortFilterProxyModel.h"
#include "ErrorWidget.h"

#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QAbstractButton>
#include <QtGui/QLabel>
#include <QtGui/QGraphicsWidget>
#include <QtCore/QList>
#include <KDE/KLineEdit>

#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/CheckBox>
#include <Plasma/Frame>

NetworkDeviceWidget::NetworkDeviceWidget(QGraphicsItem *parent) :
    QGraphicsWidget(parent), _titleFrame(NULL), _flowView(NULL), _errorWidget(NULL), _topmostLayout(NULL) {

    setMinimumSize(QSizeF(200, 200));

    // Add title frame.
    _topmostLayout = new QGraphicsLinearLayout(Qt::Vertical);
    _titleFrame = new Plasma::Frame();
    _titleFrame->setFrameShadow(Plasma::Frame::Sunken);
    _topmostLayout->addItem(_titleFrame);

    // Create search and sort freeze panel.
    _filterSortWidget = new QGraphicsWidget();
    QGraphicsLinearLayout* filterSortLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    Plasma::Label* filterLabel = new Plasma::Label();
    filterSortLayout->addItem(filterLabel);
    filterLabel->setText(i18n("Search"));
    _filterEdit = new Plasma::LineEdit();
    _filterEdit->setClearButtonShown(true);
    filterSortLayout->addItem(_filterEdit);
    filterSortLayout->addStretch();
    _freezeSortCheck = new Plasma::CheckBox();
    _freezeSortCheck->setText(i18n("Freeze sort"));
    _freezeSortCheck->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);     // needed for KDE 4.4 or label gets cut off
    filterSortLayout->addItem(_freezeSortCheck);
    _filterSortWidget->setLayout(filterSortLayout);
    _topmostLayout->addItem(_filterSortWidget);

    // Add the table view.
    _flowView = new CommunicationFlowTableView();
    connect(this, SIGNAL(configurationChanged(const AppletConfiguration&)),
            _flowView, SLOT(readConfiguration(const AppletConfiguration&)));
    connect(this, SIGNAL(configurationSaveRequested(AppletConfiguration&)),
            _flowView, SLOT(writeConfiguration(AppletConfiguration&)));
    CommunicationFlowSortFilterProxyModel* proxyModel = new CommunicationFlowSortFilterProxyModel(_flowView);
    CommunicationFlowTableModel* sourceModel = new CommunicationFlowTableModel(proxyModel);
    connect(this, SIGNAL(processFlowUpdate(const QList<CommunicationFlow>&)),
            sourceModel, SLOT(updateCommunicationFlows(const QList<CommunicationFlow>&)));
    connect(sourceModel, SIGNAL(flowUpdateCompleted()), _flowView, SLOT(tick()));
    connect(sourceModel, SIGNAL(flowUpdateCompleted()), proxyModel, SLOT(tick()));
    connect(this, SIGNAL(configurationChanged(const AppletConfiguration&)),
            sourceModel, SLOT(readConfiguration(const AppletConfiguration&)));
    proxyModel->setSourceModel(sourceModel);
    connect(_filterEdit->nativeWidget(), SIGNAL(textChanged(const QString&)),
            proxyModel, SLOT(setFilterWildcard(const QString&)));
    _freezeSortCheck->setChecked(proxyModel->getFreezeSort());
    connect(_freezeSortCheck, SIGNAL(toggled(bool)), proxyModel, SLOT(setFreezeSort(bool)));
    connect(proxyModel, SIGNAL(freezeSortStateChanged(bool)),
            (QAbstractButton*)_freezeSortCheck->nativeWidget(), SLOT(setChecked(bool)));
    _flowView->setModel(proxyModel);
    _topmostLayout->addItem(_flowView);
    setLayout(_topmostLayout);

    // Create the error widget, but hidden and not added to the layout.
    _errorWidget = new ErrorWidget();
    _errorWidget->hide();
    connect(_errorWidget, SIGNAL(configButtonClicked()), this, SIGNAL(configurationInterfaceRequested()));

}

QStringList NetworkDeviceWidget::getFullColumnNames() const {
    QStringList result;
    QAbstractItemModel* model = _flowView->model();
    for (int i = 0; i < CommunicationFlowTableModel::ColumnCount; i++) {
        result << model->headerData(i, Qt::Horizontal, CommunicationFlowTableModel::FullNameRole).toString();
    }
    return result;
}

NetworkDeviceWidget::~NetworkDeviceWidget() {
    // If either of these widgets is not currently in the layout, we own them and need to
    // delete them. Else, the layout will take care of them.
    if (!_errorWidget->isVisible()) {
        delete _errorWidget;
        _errorWidget = NULL;
    }
    if (!_flowView->isVisible()) {
        delete _flowView;
        _flowView = NULL;
    }

    // If the filter-sort widget is not in the layout, we own it and need to delete it.
    // Else, the layout will take care of it.
    if (!_filterSortWidget->isVisible()) {
        delete _filterSortWidget;
        _filterSortWidget = NULL;
    }
}

void NetworkDeviceWidget::updateTitle(bool defaultDeviceSelected) {
    if (_titleFrame) {
        if (_deviceName.isEmpty() || defaultDeviceSelected) {
            _titleFrame->setText(i18n("Network traffic"));
        } else {
            _titleFrame->setText(i18n("Network traffic on %1").arg(_deviceName));
        }
        _titleFrame->update();
    }
}

void NetworkDeviceWidget::readConfiguration(const AppletConfiguration& newConfig) {
    if (_deviceName != newConfig.getSelectedDevice()) {
        _deviceName = newConfig.getSelectedDevice();
        updateTitle(newConfig.isDefaultDevice());
    }
    setFilterSortVisible(newConfig.getShowFilterSortControls());
    emit configurationChanged(newConfig);
}

void NetworkDeviceWidget::deviceUpdated(const QString& deviceName, const QList<CommunicationFlow>& allFlows) {
    if (deviceName == _deviceName) {
        if (!_flowView->isVisible()) {
            swapMainWidgets();  // show the flow view
        }
        _errorWidget->setText(QString());    // clear any error
        emit processFlowUpdate(allFlows);
     }
}

void NetworkDeviceWidget::deviceFailed(const QString& deviceName, const QString& error) {
    if (deviceName == _deviceName) {
        if (!_errorWidget->isVisible()) {
            swapMainWidgets();  // show the error label
        }
        _errorWidget->setText(error);    // update error text
    }
}

void NetworkDeviceWidget::setFilterSortVisible(bool visible) {
    if (visible != isFilterSortVisible()) {
        if (visible) {
            // Show the widget.
            Q_ASSERT(findLayoutItem(_filterSortWidget) == -1);      // should be hidden right now
            // Place it after the title widget.
            int widgetIndex = findLayoutItem(_titleFrame) + 1;
            Q_ASSERT(widgetIndex > 0);
            _topmostLayout->insertItem(widgetIndex, _filterSortWidget);
            _filterSortWidget->show();
        } else {
            // Turn off filter and freeze sort.
            _filterEdit->setText("");
            _freezeSortCheck->setChecked(false);
            // Hide thw widget.
            int widgetIndex = findLayoutItem(_filterSortWidget);
            Q_ASSERT(widgetIndex >= 0);
            _filterSortWidget->hide();
            _topmostLayout->removeAt(widgetIndex);
        }
    } // Else, nothing to do.
}

int NetworkDeviceWidget::findLayoutItem(QGraphicsLayoutItem* item) const {
    // Weird that Qt doesn't provide a method like this in the layout class. Am I missing something?
    int result = -1;
    for (int i = 0; i < _topmostLayout->count(); i++) {
        if (_topmostLayout->itemAt(i) == item) {
            result = i;
            break;
        }
    }
    return result;
}

void NetworkDeviceWidget::swapMainWidgets() {
    // Ideally, we'd use a QStackedLayout here, but that's not available to QGraphicsWidgets (yet).
    // So we swap widgets by removing one from the layout and adding the other. Crude, but it works.
    // This is a toggle, so whichever widget is currently visible is the one moving to the background.
    QGraphicsWidget* newForegroundWidget = NULL;
    QGraphicsWidget* newBackgroundWidget = NULL;
    if (!_flowView->isVisible()) {
        newForegroundWidget = _flowView;
        newBackgroundWidget = _errorWidget;
    } else {
        newForegroundWidget = _errorWidget;
        newBackgroundWidget = _flowView;
    }

    // Hide and remove new background widget.
    int widgetIndex = findLayoutItem(newBackgroundWidget);
    Q_ASSERT(widgetIndex >= 0);
    newBackgroundWidget->hide();
    _topmostLayout->removeAt(widgetIndex);
    // Show and add the new foreground object.
    _topmostLayout->insertItem(widgetIndex, newForegroundWidget);
    newForegroundWidget->show();
}
