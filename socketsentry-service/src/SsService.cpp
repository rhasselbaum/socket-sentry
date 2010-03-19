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

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QStringList>

#include <unistd.h>

#include "Watcher.h"
#include "WatcherDBusAdaptor.h"

int main(int argc, char* argv[]) {
    if (geteuid() != 0) {
        qCritical("The service must be run as root.");
        return -1;
    }
    QCoreApplication app(argc, argv);
    Watcher Watcher;
    WatcherDBusAdaptor* adaptor = new WatcherDBusAdaptor(&Watcher);
    // Register adapter with the system bus by default or the session bus if "--session" was specified. The
    // latter may be useful for testing, since registering with the session bus usually requires no explicit
    // D-Bus configuration.
    QStringList args = app.arguments();
    if (adaptor->openForBusiness(!args.contains("--session"))) {
        qDebug() << "Registered Watcher object with D-Bus. Ready for action!";
        return app.exec();
    } else {
        qCritical("Oops! Failed to register Watcher object with D-Bus. Things to check:");
        qCritical(" 1) Is there another instance already running?");
        qCritical(" 2) Is the D-Bus config file installed? (Usually \"/etc/dbus-1/system.d/org.socketsentry.Watcher.conf\".)");
        return -2;
    }

}
