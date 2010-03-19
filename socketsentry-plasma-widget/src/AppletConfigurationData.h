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

#ifndef APPLETCONFIGURATIONDATA_H_
#define APPLETCONFIGURATIONDATA_H_

#include <QtCore/QSharedData>
#include <QtCore/QString>
#include <QtCore/QList>

/*
 * Shared data for application configuration.
 */
class AppletConfigurationData : public QSharedData {
public:
    AppletConfigurationData();
    virtual ~AppletConfigurationData();

    QString selectedDevice;
    int flowTableSortColumn;
    Qt::SortOrder flowTableSortOrder;
    QList<int> flowTableColumnSizes;
    QList<int> flowTableVisualIndices;
    QList<bool> flowTableHiddenColumns;
    bool resolveNames;
    bool osProcessSortAscending;
    int showSubdomainLevels;
    QString customFilter;
    int aggregationMode;

};

#endif /* APPLETCONFIGURATIONDATA_H_ */
