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

#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusMetaType>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QList>

#include "Watcher.h"
#include "WatcherDBusAdaptor.h"
#include "CommunicationFlow.h"
#include "IpEndpointPair.h"
#include "OsProcess.h"
#include "FlowMetrics.h"
#include "FlowStatistics.h"


WatcherDBusAdaptor::WatcherDBusAdaptor(Watcher* parent) :
    QDBusAbstractAdaptor(parent) {
    this->_parent = parent;
}

WatcherDBusAdaptor::~WatcherDBusAdaptor() {
}

bool WatcherDBusAdaptor::openForBusiness(bool systemBus) {
    qDBusRegisterMetaType<IpEndpointPair>();
    qDBusRegisterMetaType<FlowMetrics>();
    qDBusRegisterMetaType<FlowStatistics>();
    qDBusRegisterMetaType<OsProcess>();
    qDBusRegisterMetaType<CommunicationFlow>();
    qDBusRegisterMetaType<QList<CommunicationFlow> >();
    qDBusRegisterMetaType<QList<OsProcess> >();
    QDBusConnection dbus = systemBus ? QDBusConnection::systemBus() : QDBusConnection::sessionBus();
    if (!dbus.registerObject("/Watcher", _parent)) return false;
    if (!dbus.registerService("org.socketsentry.Watcher")) return false;
    setAutoRelaySignals(true);
    return true;
}

QStringList WatcherDBusAdaptor::findDevices(const QDBusMessage &msg) const {
    QString error;
    QStringList result = _parent->findDevices(error);
    if(error.length() > 0) {
        QDBusMessage reply = msg.createErrorReply("org.socketsentry.Failure", error);
        QDBusConnection::systemBus().send(reply);
    }
    return result;
}

void WatcherDBusAdaptor::showInterest(const QString& device) {
    _parent->showInterest(device);
}
