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

#include "SocketSentryDataEngine.h"
#include "WatcherClient.h"
#include "CommunicationFlow.h"

#include <QtDBus/QDBusConnection>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QSetIterator>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include <QtCore/QList>

// The interval between subscription renewals.
const int SocketSentryDataEngine::RENEWAL_INTERVAL_MS = 10000;

SocketSentryDataEngine::SocketSentryDataEngine(QObject* parent, const QVariantList& args) :
    Plasma::DataEngine(parent, args), _watcherClient(NULL) {

    connect(this, SIGNAL(generalErrorDetected(const QString&)), this, SLOT(generalFailure(const QString&)));

}

SocketSentryDataEngine::~SocketSentryDataEngine() {

}

void SocketSentryDataEngine::init() {
    // Initialize watcher client and connect to system bus.
    Q_ASSERT(!_watcherClient);
    QDBusConnection bus = QDBusConnection::systemBus();
    _watcherClient = new WatcherClient(bus, this);
    connect(_watcherClient, SIGNAL(update(const QString&, const QList<CommunicationFlow>&)),
            this, SLOT(deviceUpdate(const QString&, const QList<CommunicationFlow>&)));
    connect(_watcherClient, SIGNAL(failure(const QString&, const QString&)),
            this, SLOT(deviceFailure(const QString&, const QString&)));

    // Timer-based automatic subscription renewal for devices.
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(renewSubscriptions()));
    timer->start(RENEWAL_INTERVAL_MS);
}

void SocketSentryDataEngine::renewSubscriptions() {
    // Remove subscriptions for sources no longer in use.
    QSet<QString> activeSources = DataEngine::sources().toSet();    // call base class to get ACTIVE sources

    // Renew remaining subscriptions.
    QSetIterator<QString> i(activeSources);
    while (i.hasNext()) {
        _watcherClient->showInterest(i.next());
    }
}

bool SocketSentryDataEngine::sourceRequestEvent(const QString& device) {
    _watcherClient->showInterest(device);
    setData(device, DataEngine::Data());
    return true;
}

void SocketSentryDataEngine::deviceFailure(const QString& device, const QString& error) {
    // Is this device in our active list? If so, accept the data. Else, reject as unsolicited.
    if (Plasma::DataEngine::sources().contains(device)) {
        setData(device, I18N_NOOP("data"), QVariant());
        setData(device, I18N_NOOP("error"), error);
    }
}

void SocketSentryDataEngine::deviceUpdate(const QString& device, const QList<CommunicationFlow>& flows) {
    // Is this device in our active list? If so, accept the data. Else, reject as unsolicited.
    if (Plasma::DataEngine::sources().contains(device)) {
        QVariant variants = QVariant::fromValue(flows);
        setData(device, I18N_NOOP("data"), variants);
        setData(device, I18N_NOOP("error"), QVariant());
    }
}

void SocketSentryDataEngine::generalFailure(const QString& error) {
    setData(I18N_NOOP("status"), I18N_NOOP("error"), error);
}

QStringList SocketSentryDataEngine::sources() const {
    QString watcherError;
    QStringList result = _watcherClient->findDevices(watcherError);
    if (result.isEmpty() && !watcherError.isEmpty()) {
        // Signal error detected, which sets the "status" source with the error.
        // We can't do that directly here b/c this method must be const.
        emit generalErrorDetected(watcherError);
    }
    return result;
}

K_EXPORT_PLASMA_DATAENGINE(socketsentry, SocketSentryDataEngine)
