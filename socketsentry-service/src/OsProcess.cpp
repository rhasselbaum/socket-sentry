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

#include "OsProcess.h"

#include <QtDBus/QDBusArgument>

OsProcess::OsProcess() {
    _d = new OsProcessData();
}

OsProcess::OsProcess(const quint32 pid, const QString& program, const QString& user, const QDateTime& startTime) {
    _d = new OsProcessData();
    _d->pid = pid;
    _d->program = program;
    _d->user = user;
    _d->startTime = startTime.toUTC();
}

OsProcess::OsProcess(const QString& program) {
    _d = new OsProcessData();
    _d->program = program;
}

OsProcess::~OsProcess() {
}

QString OsProcess::getFormattedSummary() const {
    if (!_d->program.isEmpty() && _d ->pid > 0) {
        // All known.
        return QString("%1 (%2)").arg(_d->program).arg(_d->pid);
    } else if (_d ->pid > 0) {
        // PID only.
        return QString(_d->pid);
    } else if (!_d->program.isEmpty()) {
        // Program only.
        return _d->program;
    } else {
        // Nothing known.
        return "?";
    }
}


bool OsProcess::operator==(const OsProcess& rhs) const {
    if(_d == rhs._d)
        return true;
    return _d->pid == rhs.getPid()
            && _d->program == rhs.getProgram()
            && _d->user == rhs.getUser()
            && _d->startTime == rhs.getStartTime();
}

QString OsProcess::toString(bool special) const {
    return QString("%1/%2%3 as %4 ctime %5")
            .arg(_d->pid)
            .arg(_d->program)
            .arg(special ? "*" : "")
            .arg(_d->user)
            .arg(_d->startTime.toLocalTime().toString("h:mm:ss"));
}


uint qHash(const OsProcess& key) {
    return qHash(key.getPid())
            ^ qHash(key.getProgram())
            ^ qHash(key.getUser())
            ^ qHash(key.getStartTime().toTime_t());
}

QDBusArgument& operator<<(QDBusArgument& arg, const OsProcess& obj) {
    arg.beginStructure();
    arg << obj.getPid() << obj.getProgram() << obj.getUser() << obj.getStartTime().toUTC().toTime_t();
    arg.endStructure();
    return arg;
}

const QDBusArgument& operator>>(const QDBusArgument& arg, OsProcess& obj) {
    arg.beginStructure();

    quint32 pid;
    arg >> pid;
    obj.setPid(pid);

    QString program;
    arg >> program;
    obj.setProgram(program);

    QString user;
    arg >> user;
    obj.setUser(user);

    uint startTimeT;
    arg >> startTimeT;
    QDateTime startTime;
    startTime.setTimeSpec(Qt::UTC);
    startTime.setTime_t(startTimeT);
    obj.setStartTime(startTime);

    arg.endStructure();
    return arg;
}
