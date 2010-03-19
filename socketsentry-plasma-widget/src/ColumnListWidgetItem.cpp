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

#include "ColumnListWidgetItem.h"

ColumnListWidgetItem::ColumnListWidgetItem(const QString& columnName, int columnPosition, QListWidget* parent) :
    QListWidgetItem(columnName, parent), _columnPosition(columnPosition) {
}

ColumnListWidgetItem::~ColumnListWidgetItem() {
}

bool ColumnListWidgetItem::operator<(const QListWidgetItem& other) const {
    const ColumnListWidgetItem* rhs = dynamic_cast<const ColumnListWidgetItem*>(&other);
    if (rhs) {
        return _columnPosition < rhs->getColumnPosition();
    } else {
        return QListWidgetItem::operator<(other);
    }
}
