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

#ifndef WATCHERDBUSADAPTOR_H_
#define WATCHERDBUSADAPTOR_H_

#include "Watcher.h"

#include <QtDBus/QDBusAbstractAdaptor>
#include <QtCore/QStringList>

class QDBusMessage;
template <class E> class QList;
class QStringList;
class CommunicationFlow;

/*
 * D-Bus adaptor for the Watcher interface.
 */
class WatcherDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.socketsentry.Watcher")
    Q_PROPERTY(bool resolveNames READ getResolveNames WRITE setResolveNames)
    Q_PROPERTY(bool osProcessSortAscending READ getOsProcessSortAscending WRITE setOsProcessSortAscending)
    Q_PROPERTY(QString customFilter READ getCustomFilter WRITE setCustomFilter)

public:
    WatcherDBusAdaptor(Watcher* parent);
    virtual ~WatcherDBusAdaptor();

    // Register data object meta-types, the service, and object with D-Bus and return true on success. If the
    // system bus argument is true, the adapter registers with the system bus. Else, it registers with the current
    // user's session bus.
    bool openForBusiness(bool systemBus);

public slots:
    // True if this watcher should do network name resolution. This may be expensive.
    bool getResolveNames() const { return _parent->getResolveNames(); }
    Q_NOREPLY void setResolveNames(bool resolveNames) { _parent->setResolveNames(resolveNames); }

    // If true, each list of OS processes sharing an IP endpoint pair socket is sorted in ascending order by start
    // time and PID. If false, the order is reversed.
    bool getOsProcessSortAscending() const { return _parent->getOsProcessSortAscending(); }
    Q_NOREPLY void setOsProcessSortAscending(bool osProcessSortAscending) { _parent->setOsProcessSortAscending(osProcessSortAscending); }

    // A custom pcap filter applied across all devices.
    QString getCustomFilter() const { return _parent->getCustomFilter(); }
    Q_NOREPLY void setCustomFilter(const QString& customFilter) { _parent->setCustomFilter(customFilter); }

    // Mirrors the public slots of Watcher, but takes a D-Bus message argument to relay errors back to the client.
    Q_NOREPLY void showInterest(const QString& device);
    QStringList findDevices(const QDBusMessage &msg) const;

signals:
    // Traffic update for the specified device.
    void update(const QString& device, const QList<CommunicationFlow>& flows);

    // Indicates a failure watching the given device. After a failure signal, there will be no further updates until
    // a client shows interest in the device again.
    void failure(const QString& device, const QString& error);

private:
    Watcher* _parent;
};

#endif /* WATCHERDBUSADAPTOR_H_ */
