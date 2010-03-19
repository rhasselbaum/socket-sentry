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

#ifndef WATCHERCLIENTCONSOLEPRINTER_H_
#define WATCHERCLIENTCONSOLEPRINTER_H_

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QString>

class QTimerEvent;
class WatcherClient;
template <class E> class QList;
class CommunicationFlow;

/*
 * A simple wrapper around a Watcher D-Bus client that listens for signals emitted from the Watcher and prints them to the
 * standard output stream. Clients of this printer can register interest in any number of devices by calling the "addDevice"
 * method. The printer will automatically renew interest as needed to receive continuous updates from the service unless
 * or until "removeDevice" is called. It may continue to receive (and print) signals for removed devices for a short time until
 * interest times out at the service.
 */
class WatcherClientConsolePrinter : public QObject {
    Q_OBJECT

public:
    WatcherClientConsolePrinter(WatcherClient* parent);
    virtual ~WatcherClientConsolePrinter();

    // Add a device to the watch set.
    void addDevice(const QString& deviceName) {
        _devices.insert(deviceName);
        emit renew(deviceName);
    }

    // Remove a device from the watch set.
    void removeDevice(const QString& deviceName) { _devices.remove(deviceName); }

signals:
    void renew(const QString& device);

protected:
    // Renew inteest in devices.
    void timerEvent(QTimerEvent* event);

private slots:
    // Traffic update for the specified device.
    void printUpdate(const QString& device, const QList<CommunicationFlow>& flows);

    // Indicates a failure watching the given device. After a failure signal, there will be no further updates until
    // a client shows interest in the device again.
    void printFailure(const QString& device, const QString& error);

private:
    // Devvices we're interested in.
    QSet<QString> _devices;

    // The interval between device interest renewals.
    static const int RENEWAL_INTERVAL_MS;

};

#endif /* WATCHERCLIENTCONSOLEPRINTER_H_ */
