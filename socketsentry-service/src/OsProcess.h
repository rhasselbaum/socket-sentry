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

#ifndef OSPROCESS_H_
#define OSPROCESS_H_

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

#include "OsProcessData.h"

class QDBusArgument;

/*
 * A grouping of program name, PID, and other attributes that uniquely identify an operating system process. Optionally,
 * some attributes may be left unspecified in which case this object represents a group of processes that share certain
 * characteristics such as program name.
 */
class OsProcess {
public:
    OsProcess();
    OsProcess(const quint32 pid, const QString& program, const QString& user, const QDateTime& startTime);
    OsProcess(const QString& program);
    virtual ~OsProcess();
    bool operator==(const OsProcess& rhs) const;

    quint32 getPid() const { return _d->pid; }
    QString getProgram() const { return _d->program; }
    QString getUser() const { return _d->user; }
    QDateTime getStartTime() const { return _d->startTime; }

    void setPid(quint32 pid) { _d->pid = pid; }
    void setProgram(QString program) { _d->program = program; }
    void setUser(QString user) { _d->user = user; }
    void setStartTime(const QDateTime& startTime) { _d->startTime = startTime.toUTC(); }

    // Return program and PID combined if both are known. If only the PID or only the program name is known, then the
    // known part is returned by itself. Else, '?' is returned.
    QString getFormattedSummary() const;

    // Format as a string suitable for debug output, but not end-user consumption. If "special" is true, the program
    // name is embellished with an asterisk to indicate it is special in some way.
    virtual QString toString(bool special = false) const;

private:
    QSharedDataPointer<OsProcessData> _d;
};

// Global hash function required to use the class as a key in QHash containers.
uint qHash(const OsProcess& key);

// DBUS argument marshalling.
QDBusArgument& operator<<(QDBusArgument& argument, const OsProcess& obj);
// DBUS argument unmarshalling.
const QDBusArgument& operator>>(const QDBusArgument& argument, OsProcess& obj);

// Make visible as a D-Bus data type.
Q_DECLARE_METATYPE(OsProcess)
Q_DECLARE_METATYPE(QList<OsProcess>)

#endif /* OSPROCESS_H_ */
