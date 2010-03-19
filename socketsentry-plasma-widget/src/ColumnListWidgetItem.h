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

#ifndef COLUMNLISTWIDGETITEM_H_
#define COLUMNLISTWIDGETITEM_H_

#include <QtGui/QListWidgetItem>
#include <QtCore/QString>
class QListWidget;

/*
 * A list widget item representing a column in a communication flow table model. This subclass overrides the
 * less than operator of the base class to facilitate sorting by default column position rather than name.
 */
class ColumnListWidgetItem : public QListWidgetItem {
public:
    ColumnListWidgetItem(const QString& columnName, int columnPosition, QListWidget* parent = 0);
    virtual ~ColumnListWidgetItem();

    // The default position of the named column.
    int getColumnPosition() const { return _columnPosition; }
    void setColumnPosition(int columnPosition) { _columnPosition = columnPosition; }

    // Return true if the column position of the other item is less than this item's column position.
    // Default to base class implementation if the other item is not a ColumnListWidgetItem.
    virtual bool operator<(const QListWidgetItem& other) const;

private:
    int _columnPosition;
};

#endif /* COLUMNLISTWIDGETITEM_H_ */
