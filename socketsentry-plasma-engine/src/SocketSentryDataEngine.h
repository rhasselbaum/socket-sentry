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

#ifndef SOCKETSENTRYDATAENGINE_H_
#define SOCKETSENTRYDATAENGINE_H_

#include "WatcherClient.h"

#include <Plasma/DataEngine>
#include <QtCore/QObject>
#include <QtCore/QSet>

class QVariant;
template <class E> class QList;
typedef QList<QVariant> QVariantList;
class QString;
class CommunicationFlow;
class QStringList;

/*
 * Plasma data engine for Socket Sentry. Serves as a bridge between widgets and the privileged Watcher service.
 * This engine pushes data updates to clients asynchronously as signals are emitted from the Watcher service,
 * so there is no need to poll. It also exposes global properties that mirror those of the service.
 */
class SocketSentryDataEngine : public Plasma::DataEngine {
    Q_OBJECT
    Q_PROPERTY(bool resolveNames READ getResolveNames WRITE setResolveNames)
    Q_PROPERTY(bool osProcessSortAscending READ getOsProcessSortAscending WRITE setOsProcessSortAscending)
    Q_PROPERTY(QString customFilter READ getCustomFilter WRITE setCustomFilter)

public:
    SocketSentryDataEngine(QObject* parent, const QVariantList& args);
    virtual ~SocketSentryDataEngine();

    // Set up the watcher client and timer-based automatic subscription renewals.
    virtual void init();

    // Override the base class to return the list of all supported capture devices. If the sources list cannot be
    // obtained due to an erorr, a source called "status" can be queried giving a single "error" entry with an
    // error message.
    virtual QStringList sources() const;

    // True if this watcher should do network name resolution. This may be expensive.
    bool getResolveNames() const {
        Q_ASSERT(_watcherClient);
        return _watcherClient->getResolveNames();
    }
    void setResolveNames(bool resolveNames) {
        Q_ASSERT(_watcherClient);
        _watcherClient->setResolveNames(resolveNames);
    }

    // If true, each list of OS processes sharing an IP endpoint pair socket is sorted in ascending order by start
    // time and PID. If false, the order is reversed.
    bool getOsProcessSortAscending() const {
        Q_ASSERT(_watcherClient);
        return _watcherClient->getOsProcessSortAscending();
    }
    void setOsProcessSortAscending(bool osProcessSortAscending) {
        Q_ASSERT(_watcherClient);
        _watcherClient->setOsProcessSortAscending(osProcessSortAscending);
    }

    // A custom pcap filter applied across all devices.
    QString getCustomFilter() const {
        Q_ASSERT(_watcherClient);
        return _watcherClient->getCustomFilter();
    }
    void setCustomFilter(const QString& customFilter) {
        Q_ASSERT(_watcherClient);
        _watcherClient->setCustomFilter(customFilter);
    }

signals:
    // Signaled when the service returns a general error such as a failure to retrieve a device list.
    void generalErrorDetected(const QString& error) const;

public slots:
    // Invoked when the Watcher fails to monitor traffic on a device. Sets the "error" key on the matching source
    // in this engine and removes the device from the current subscription list.
    void deviceFailure(const QString& device, const QString& error);

    // Invoked when the Watcher fails to communicate with the service or the service encounters an error. Sets the
    // "error" key on the special "status" source in this engine.
    void generalFailure(const QString& error);

    // Called when the Watcher signals an update on a device. Sets the matching source data in this engine.
    void deviceUpdate(const QString& device, const QList<CommunicationFlow>& flows);

    // Update subscription set from active sources and renew all subscriptions.
    void renewSubscriptions();

protected:
    // Express new interest in a device.
    virtual bool sourceRequestEvent(const QString& device);

private:
    // The D-Bus service client.
    WatcherClient* _watcherClient;

    // The interval between subscription renewals.
    static const int RENEWAL_INTERVAL_MS;
};

#endif /* SOCKETSENTRYDATAENGINE_H_ */
