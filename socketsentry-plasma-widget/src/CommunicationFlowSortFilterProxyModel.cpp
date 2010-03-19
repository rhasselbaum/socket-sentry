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

#include "CommunicationFlowSortFilterProxyModel.h"
#include "CommunicationFlowTableModel.h"

#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QDebug>


CommunicationFlowSortFilterProxyModel::CommunicationFlowSortFilterProxyModel(QObject* parent) :
    QSortFilterProxyModel(parent), _freezeSort(false), _lastSortedColumn(0), _lastSortedOrder(Qt::AscendingOrder) {
    setFilterKeyColumn(-1);
    setSortRole(CommunicationFlowTableModel::SortRole);
}

CommunicationFlowSortFilterProxyModel::~CommunicationFlowSortFilterProxyModel() {
}

bool CommunicationFlowSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right ) const {
    QVariant leftData = sourceModel()->data(left, sortRole());
    QVariant rightData = sourceModel()->data(right, sortRole());
    if (leftData == rightData) {
        // Break tie.
        QList<QPair<CommunicationFlowTableModel::Column, bool> > tieBreakers;
        QPair<CommunicationFlowTableModel::Column, bool> process(CommunicationFlowTableModel::ProcessOrProgramColumn, true);
        QPair<CommunicationFlowTableModel::Column, bool> rate(CommunicationFlowTableModel::RateColumn, false);
        QPair<CommunicationFlowTableModel::Column, bool> remoteHost(CommunicationFlowTableModel::RemoteHostColumn, true);
        tieBreakers << process << rate << remoteHost;
        return lessThan(left.row(), right.row(), tieBreakers);
    } else {
        // Delegate to base class.
        return QSortFilterProxyModel::lessThan(left, right);
    }
}

void CommunicationFlowSortFilterProxyModel::sort(int column, Qt::SortOrder order) {
    QSortFilterProxyModel::sort(column, order);
    _lastSortedColumn = column;
    _lastSortedOrder = order;
    setFreezeSort(false);
}

void CommunicationFlowSortFilterProxyModel::setFreezeSort(bool freezeSort) {
    if (_freezeSort != freezeSort) {
        _freezeSort = freezeSort;
        emit freezeSortStateChanged(_freezeSort);
    }
}

void CommunicationFlowSortFilterProxyModel::tick() {
    if (!_freezeSort) {
        QSortFilterProxyModel::sort(_lastSortedColumn, _lastSortedOrder);
    }
    if (!filterRegExp().pattern().isEmpty()) {
        // Reapply filter.
        setFilterRegExp(filterRegExp().pattern());
    }
}

bool CommunicationFlowSortFilterProxyModel::lessThan(int leftRow, int rightRow,
        QList<QPair<CommunicationFlowTableModel::Column, bool> >& sortCriteria) const {

    if (sortCriteria.isEmpty()) {
        // Nothing to sort by.
        return false;
    } else {
        QPair<CommunicationFlowTableModel::Column, bool> criterion = sortCriteria.takeFirst();
        QModelIndex leftIndex = sourceModel()->index(leftRow, criterion.first);
        QModelIndex rightIndex = sourceModel()->index(rightRow, criterion.first);
        QVariant leftData = sourceModel()->data(leftIndex, sortRole());
        QVariant rightData = sourceModel()->data(rightIndex, sortRole());
        if (leftData == rightData) {
            // Recursive call to break tie. Sort criteria list has already been reduced.
            return lessThan(leftRow, rightRow, sortCriteria);
        }
        else {
            // No tie. Delegate to base class.
            if ((sortOrder() == Qt::AscendingOrder && criterion.second)
                || (sortOrder() == Qt::DescendingOrder && !criterion.second)) {
                // Required sort order of this column agrees with the model's current sort order.
                return QSortFilterProxyModel::lessThan(leftIndex, rightIndex);
            } else {
                // Required sort order of this column is opposite the model's current sort order, so swap the arguments.
                return QSortFilterProxyModel::lessThan(rightIndex, leftIndex);
            }
        }
    }
}
