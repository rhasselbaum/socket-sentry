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

#ifndef OSPROCESSDATA_H_
#define OSPROCESSDATA_H_

#include <QtCore/QSharedData>
#include <QtCore/QString>
#include <QtCore/QDateTime>

/*
 * Shared data for OsProcess.
 */
class OsProcessData : public QSharedData {
public:
    OsProcessData();
    virtual ~OsProcessData();

    quint32 pid;
    QString program;
    QString user;
    QDateTime startTime;
};

#endif /* OSPROCESSDATA_H_ */
