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

#ifndef COMMUNICATIONFLOWSORTFILTERPROXYMODEL_H_
#define COMMUNICATIONFLOWSORTFILTERPROXYMODEL_H_

#include "CommunicationFlowTableModel.h"

#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QObject>

template <class E> class QList;
template <class F, class S> class QPair;
class QAbstractItemModel;


/*
 * A sort-filter proxy model that understands how to sort and filter a CommunicationFlowTableModel and remembers
 * the most recent sort column and order.
 */
class CommunicationFlowSortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    CommunicationFlowSortFilterProxyModel(QObject* parent = 0);
    virtual ~CommunicationFlowSortFilterProxyModel();

    // Get the column used for the most recent sort. This value is never negative.
    int getLastSortedColumn() const { return _lastSortedColumn; }

    // Get the order used for the most recent sort..
    Qt::SortOrder getLastSortedOrder() const { return _lastSortedOrder; }

    // If true, this model will not automatically re-sort on a tick signal.
    bool getFreezeSort() const { return _freezeSort; }

    // Implements sorting based on the sort role just like the base class implementation, but ties are broken
    // using the following additional columns: process/program (ascending), rate (descending), remote address
    // and port (ascending).
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

    // Sort the contents of the model and record the most recent sort column and order. If the "freeze sort"
    // flag is set, calling this method "unfreezes" it.
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

signals:
    // Emitted whenever the freeze sort flag state changes.
    void freezeSortStateChanged(bool sortFrozen);

public slots:
    // Invoked after the source model has completed one update cycle. Re-sorts this model unless "freeze sort" is set.
    void tick();

    // If true, this model will not automatically re-sort on a tick signal. This method will emit freezeSortStateChanged
    // if the state changes.
    void setFreezeSort(bool freezeSort);

private:
    // Implements sorting based on an ordered list of columns (sort criteria). Columns are expressed as pairs of
    // one dentifier and boolean that is true if the sort by that column should be asending. Columns beyond the
    // first in the list are considered only in case of a tie. The method removes columns from the list as they
    // are processed.
    bool lessThan(int leftRow, int rightRow,
            QList<QPair<CommunicationFlowTableModel::Column, bool> >& sortCriteria) const;

    // If true, this model will not automatically re-sort on a tick signal.
    bool _freezeSort;

    // The most recent sort column.
    int _lastSortedColumn;

    // The most recent sort order.
    Qt::SortOrder _lastSortedOrder;
};

#endif /* COMMUNICATIONFLOWSORTFILTERPROXYMODEL_H_ */
