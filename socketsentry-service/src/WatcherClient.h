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

#ifndef WATCHERCLIENT_H_
#define WATCHERCLIENT_H_

#include <QtDBus/QtDBus>
#include <QtCore/QObject>

class QString;
class QDBusConnection;
template <class E> class QList;
class CommunicationFlow;

/*
 * Client proxy for Watcher D-Bus service.
 */
class WatcherClient : public QDBusAbstractInterface {
    Q_OBJECT

public:
    static inline const char* staticInterfaceName() {
        return "org.socketsentry.Watcher";
    }
    WatcherClient(const QDBusConnection& connection, QObject *parent = 0);
    ~WatcherClient();

    // Refer to Watcher method declarations for information on these methods.
    QStringList findDevices(QString& error);

    bool getResolveNames();
    void setResolveNames(bool resolveNames);

    bool getOsProcessSortAscending();
    void setOsProcessSortAscending(bool osProcessSortAscending);

    QString getCustomFilter();
    void setCustomFilter(const QString& customFilter);


public slots:
    // Refer to Watcher method declarations for information on these methods.
    Q_NOREPLY void showInterest(const QString& device);

signals:
    // Refer to Watcher method declarations for information on these methods.
    void failure(const QString& device, const QString& error);
    void update(const QString& device, const QList<CommunicationFlow>& flows);
};

#endif /* WATCHERCLIENT_H_ */
