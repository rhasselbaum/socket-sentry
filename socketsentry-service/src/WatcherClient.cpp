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

#include "WatcherClient.h"

#include "CommunicationFlow.h"
#include "IpEndpointPair.h"
#include "OsProcess.h"
#include "FlowMetrics.h"
#include "FlowStatistics.h"

#include <QtDBus/QDBusReply>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QStringList>

WatcherClient::WatcherClient(const QDBusConnection& connection, QObject* parent)
    : QDBusAbstractInterface("org.socketsentry.Watcher", "/Watcher", staticInterfaceName(), connection, parent) {

    qDBusRegisterMetaType<IpEndpointPair>();
    qDBusRegisterMetaType<FlowMetrics>();
    qDBusRegisterMetaType<FlowStatistics>();
    qDBusRegisterMetaType<OsProcess>();
    qDBusRegisterMetaType<CommunicationFlow>();
    qDBusRegisterMetaType<QList<CommunicationFlow> >();
    qDBusRegisterMetaType<QList<OsProcess> >();

    // Tickle the service to make sure it's up. Without this, automatic relay of signals doesn't
    // seem to work starting with Qt 4.6 / KDE 4.4. (Older versions worked fine.)
    call("ping");
}

WatcherClient::~WatcherClient() {
}

void WatcherClient::showInterest(const QString& device) {
    QList<QVariant> argumentList;
    argumentList << qVariantFromValue(device);
    callWithArgumentList(QDBus::NoBlock, QLatin1String("showInterest"), argumentList);
}

QStringList WatcherClient::findDevices(QString& error) {
    QDBusReply<QStringList> reply = call("findDevices");
    if(reply.isValid()) {
        return reply.value();
    } else {
        error = reply.error().message();
        QStringList empty;
        return empty;
    }
}

bool WatcherClient::getResolveNames() {
    QDBusReply<bool> reply = call("getResolveNames");
    return reply.value();
}

void WatcherClient::setResolveNames(bool resolveNames) {
    QList<QVariant> argumentList;
    argumentList << qVariantFromValue(resolveNames);
    callWithArgumentList(QDBus::NoBlock, QLatin1String("setResolveNames"), argumentList);
}

bool WatcherClient::getOsProcessSortAscending() {
    QDBusReply<bool> reply = call("getOsProcessSortAscending");
    return reply.value();
}

void WatcherClient::setOsProcessSortAscending(bool osProcessSortAscending) {
    QList<QVariant> argumentList;
    argumentList << qVariantFromValue(osProcessSortAscending);
    callWithArgumentList(QDBus::NoBlock, QLatin1String("setOsProcessSortAscending"), argumentList);
}

QString WatcherClient::getCustomFilter() {
    QDBusReply<QString> reply = call("getCustomFilter");
    return reply.value();
}

void WatcherClient::setCustomFilter(const QString& customFilter) {
    QList<QVariant> argumentList;
    argumentList << qVariantFromValue(customFilter);
    callWithArgumentList(QDBus::NoBlock, QLatin1String("setCustomFilter"), argumentList);
}
