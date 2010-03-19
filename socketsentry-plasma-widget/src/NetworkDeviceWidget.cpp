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

#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QAbstractButton>
#include <QtGui/QLabel>
#include <QtCore/QList>
#include <KDE/KLineEdit>

#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/CheckBox>
#include <Plasma/Frame>


NetworkDeviceWidget::NetworkDeviceWidget(QGraphicsItem *parent) :
    QGraphicsWidget(parent), _titleFrame(NULL), _mainWidgetIndex(-1), _flowView(NULL), _errorLabel(NULL), _topmostLayout(NULL) {

    // Add title frame.
    _topmostLayout = new QGraphicsLinearLayout(Qt::Vertical);
    _titleFrame = new Plasma::Frame();
    _titleFrame->setFrameShadow(Plasma::Frame::Sunken);
    _topmostLayout->addItem(_titleFrame);

    // Create search and sort freeze panel.
    QGraphicsWidget* filterSortWidget = new QGraphicsWidget();
    QGraphicsLinearLayout* filterSortLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    Plasma::Label* filterLabel = new Plasma::Label();
    filterSortLayout->addItem(filterLabel);
    filterLabel->setText(i18n("Search"));
    Plasma::LineEdit* filterEdit = new Plasma::LineEdit();
    filterSortLayout->addItem(filterEdit);
    filterSortLayout->addStretch();
    Plasma::CheckBox* freezeSortCheck = new Plasma::CheckBox();
    freezeSortCheck->setText(i18n("Freeze sort"));
    filterSortLayout->addItem(freezeSortCheck);
    filterSortWidget->setLayout(filterSortLayout);
    _topmostLayout->addItem(filterSortWidget);

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
    connect(filterEdit->nativeWidget(), SIGNAL(textChanged(const QString&)),
            proxyModel, SLOT(setFilterWildcard(const QString&)));
    freezeSortCheck->setChecked(proxyModel->getFreezeSort());
    connect(freezeSortCheck, SIGNAL(toggled(bool)), proxyModel, SLOT(setFreezeSort(bool)));
    connect(proxyModel, SIGNAL(freezeSortStateChanged(bool)),
            (QAbstractButton*)freezeSortCheck->nativeWidget(), SLOT(setChecked(bool)));
    _flowView->setModel(proxyModel);
    _topmostLayout->addItem(_flowView);
    _mainWidgetIndex = _topmostLayout->count() - 1;
    setLayout(_topmostLayout);

    //  Add the error label, parented by this widget, but hidden and not added to the layout.
    _errorLabel = new Plasma::Label();
    _errorLabel->nativeWidget()->setWordWrap(true);
    _errorLabel->setAlignment(Qt::AlignHCenter);
    _errorLabel->hide();

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
    if (!_errorLabel->isVisible()) {
        delete _errorLabel;
        _errorLabel = NULL;
    }
    if (!_flowView->isVisible()) {
        delete _flowView;
        _flowView = NULL;
    }
}

void NetworkDeviceWidget::updateTitle() {
    if (_titleFrame) {
        if (_deviceName.isEmpty() || _deviceName == "any") {
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
        updateTitle();
    }
    emit configurationChanged(newConfig);
}

void NetworkDeviceWidget::deviceUpdated(const QString& deviceName, const QList<CommunicationFlow>& allFlows) {
    if (deviceName == _deviceName) {
        _errorLabel->setText(QString());    // clear any error
        if (!_flowView->isVisible()) {
            swapMainWidgets();  // show the flow view
        }
        emit processFlowUpdate(allFlows);
     }
}

void NetworkDeviceWidget::deviceFailed(const QString& deviceName, const QString& error) {
    if (deviceName == _deviceName) {
        _errorLabel->setText(error);    // update error text
        if (!_errorLabel->isVisible()) {
            swapMainWidgets();  // show the error label
        }
    }
}

void NetworkDeviceWidget::swapMainWidgets() {
    // Ideally, we'd use a QStackedLayout here, but that's not available to QGraphicsWidgets (yet).
    // So we swap widgets by removing one from the layout and adding the other. Crude, but it works.
    // This is a toggle, so whichever widget is currently visible is the one moving to the background.
    QGraphicsWidget* newForegroundWidget = NULL;
    QGraphicsWidget* newBackgroundWidget = NULL;
    if (!_flowView->isVisible()) {
        newForegroundWidget = _flowView;
        newBackgroundWidget = _errorLabel;
    } else {
        newForegroundWidget = _errorLabel;
        newBackgroundWidget = _flowView;
    }
    // Hide and remove new background widget.
    newBackgroundWidget->hide();
    Q_ASSERT(_topmostLayout->itemAt(_mainWidgetIndex) == newBackgroundWidget);
    _topmostLayout->removeAt(_mainWidgetIndex);
    // Show and add the new foreground object.
    _topmostLayout->insertItem(_mainWidgetIndex, newForegroundWidget);
    newForegroundWidget->show();
}
