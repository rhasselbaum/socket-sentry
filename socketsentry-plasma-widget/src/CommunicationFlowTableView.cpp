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

#include "CommunicationFlowTableView.h"
#include "CommunicationFlowTableModel.h"
#include "AppletConfiguration.h"

#include <QtGui/QTreeView>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtCore/QPoint>
#include <QtGui/QHeaderView>
#include <Plasma/Theme>

CommunicationFlowTableView::CommunicationFlowTableView(QGraphicsWidget* parent) :
    Plasma::TreeView(parent), _contextMenu(NULL), _columnsSized(false), _appliedColumnHiding(false) {

    QTreeView* nativeTree = nativeWidget();
    nativeTree->setSortingEnabled(true);
    nativeTree->sortByColumn(CommunicationFlowTableModel::ProcessOrProgramColumn, Qt::AscendingOrder);
    nativeTree->setAllColumnsShowFocus(true);
    nativeTree->setMinimumWidth(120);
    nativeTree->setRootIsDecorated(false);
    nativeTree->setFrameShape(QFrame::StyledPanel);
    nativeTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    nativeTree->setSelectionMode(QAbstractItemView::NoSelection);
    nativeTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    nativeTree->setUniformRowHeights(true);
    nativeTree->header()->setResizeMode(QHeaderView::Interactive);
    nativeTree->header()->setCascadingSectionResizes(false);
    updateStyle();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateStyle()));
}

void CommunicationFlowTableView::updateStyle() {
    // Get theme colors
    QColor textColor = Plasma::Theme::defaultTheme()->color( Plasma::Theme::TextColor );
    QColor baseColor = Plasma::Theme::defaultTheme()->color( Plasma::Theme::BackgroundColor );
    QColor buttonColor = Plasma::Theme::defaultTheme()->color( Plasma::Theme::BackgroundColor );
    buttonColor.setAlpha(130);

    QPalette p = palette();
    p.setColor( QPalette::Background, baseColor );
    p.setColor( QPalette::Base, baseColor );
    p.setColor( QPalette::Button, buttonColor );
    p.setColor( QPalette::Foreground, textColor );
    p.setColor( QPalette::Text, textColor );
    p.setColor( QPalette::ButtonText, textColor );
    setPalette(p);

    if(_contextMenu) {
        _contextMenu->setPalette(p);
    }
}

void CommunicationFlowTableView::setModel(QAbstractItemModel* model) {
    Plasma::TreeView::setModel(model);
    // Add context menu actions for columns.
    _contextMenu = new QMenu();
    for (int i = 0; i < model->columnCount(); i++) {
        QAction* columnAction = new QAction(model->headerData(i, Qt::Horizontal,
                CommunicationFlowTableModel::FullNameRole).toString(), this);
        columnAction->setCheckable(true);
        _contextMenu->addAction(columnAction);
    }
    _contextMenu->setPalette(palette());
    nativeWidget()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(nativeWidget(), SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showContextMenu(const QPoint&)));
    nativeWidget()->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(nativeWidget()->header(), SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showContextMenu(const QPoint&)));
}

void CommunicationFlowTableView::showContextMenu(const QPoint& pos) {
    Q_UNUSED(pos);
    if (_contextMenu) {
        // Update checked states of menu items based on the currently visible columns.
        QList<QAction*> menuActions = _contextMenu->actions();
        for (int i = 0; i < menuActions.size(); i++) {
            menuActions[i]->setChecked(!nativeWidget()->isColumnHidden(i));
        }
        // Show the menu.
        QAction* triggered = _contextMenu->exec(QCursor::pos());
        if (triggered) {
            // Toggle the visible state of the selected column.
            int columnIndex = menuActions.indexOf(triggered);
            if (nativeWidget()->isColumnHidden(columnIndex)) {
                // Column was hidden, so show it.
                nativeWidget()->setColumnHidden(columnIndex, false);
                nativeWidget()->resizeColumnToContents(columnIndex);
            } else {
                // Column was visible, so hide it.
                nativeWidget()->setColumnHidden(columnIndex, true);
            }
        }
    }
}

void CommunicationFlowTableView::tick() {
    // Resize columns to fit contents if they have not been sized yet.
    if (!_columnsSized && model()->rowCount(QModelIndex()) > 0) {
        for (int i = 0; i < model()->columnCount(); i++) {
            nativeWidget()->resizeColumnToContents(i);
        }
        _columnsSized = true;
    }
    if (!_appliedColumnHiding) {
        // Only certain columns are visible by default.
        for (int i = 0; i < CommunicationFlowTableModel::ColumnCount; i++) {
            switch (i) {
            case CommunicationFlowTableModel::DeviceStateColumn:
            case CommunicationFlowTableModel::ProcessOrProgramColumn:
            case CommunicationFlowTableModel::RateColumn:
            case CommunicationFlowTableModel::RemoteHostColumn:
                break;
            default:
                nativeWidget()->setColumnHidden(i, true);
            }
        }
        _appliedColumnHiding = true;
    }
}

void CommunicationFlowTableView::readConfiguration(const AppletConfiguration& newConfig) {
    // Assign column sizes.
    QList<int> columnSizes = newConfig.getFlowTableColumnSizes();
    if (columnSizes.size() == model()->columnCount()) {
        for (int i = 0; i < model()->columnCount(); i++) {
            nativeWidget()->setColumnWidth(i, columnSizes[i]);
        }
        _columnsSized = true;
    }
    // Hide columns.
    QList<bool> hiddenColumns = newConfig.getFlowTableHiddenColumns();
    if (hiddenColumns.size() == model()->columnCount()) {
        for (int i = 0; i < model()->columnCount(); i++) {
            bool wasHidden = nativeWidget()->isColumnHidden(i);
            nativeWidget()->setColumnHidden(i, hiddenColumns[i]);
            if (wasHidden && !nativeWidget()->isColumnHidden(i)) {
                // Column is becoming visible. Adjust size based on contents.
                nativeWidget()->resizeColumnToContents(i);
            }
        }
        _appliedColumnHiding = true;
    }
    // Reorder columns by new visual index.
    QList<int> visualIndices = newConfig.getFlowTableVisualIndices();
    if (visualIndices.size() == model()->columnCount()) {
        for (int newVisualIndex = 0; newVisualIndex < visualIndices.size(); newVisualIndex++) {
            int logicalIndex = visualIndices.indexOf(newVisualIndex);
            Q_ASSERT(logicalIndex >= 0);
            int oldVisualIndex = nativeWidget()->header()->visualIndex(logicalIndex);
            if (newVisualIndex != oldVisualIndex) {
                nativeWidget()->header()->moveSection(oldVisualIndex, newVisualIndex);
            }
        }
    }
    // Set sort column.
    int sortColumn = newConfig.getFlowTableSortColumn();
    Qt::SortOrder sortOrder = newConfig.getFlowTableSortOrder();
    if (sortColumn >= 0 && sortColumn < model()->columnCount()) {
        nativeWidget()->sortByColumn(sortColumn, sortOrder);
    }

}

void CommunicationFlowTableView::writeConfiguration(AppletConfiguration& config) {
    QList<int> columnSizes;
    QList<bool> hiddenColumns;
    QList<int> visualIndices;
    for (int i = 0; i < model()->columnCount(); i++) {
        columnSizes << nativeWidget()->columnWidth(i);
        hiddenColumns << nativeWidget()->isColumnHidden(i);
        visualIndices << nativeWidget()->header()->visualIndex(i);
    }
    config.setFlowTableColumnSizes(columnSizes);
    config.setFlowTableHiddenColumns(hiddenColumns);
    config.setFlowTableVisualIndices(visualIndices);
    config.setFlowTableSortColumn(nativeWidget()->header()->sortIndicatorSection());
    config.setFlowTableSortOrder(nativeWidget()->header()->sortIndicatorOrder());
}

CommunicationFlowTableView::~CommunicationFlowTableView() {
}
