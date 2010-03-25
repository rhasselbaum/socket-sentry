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
#include <QtDBus/QDBusConnection>
#include <QtCore/QTextStream>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>

#include "WatcherClient.h"
#include "WatcherClientConsolePrinter.h"

void printUsage(QTextStream& err) {
    QStringList args = QCoreApplication::arguments();
    Q_ASSERT(args.size() >= 1);
    err << endl << "Usage: " << args[0] << " [--session] <device> [<device> ...]" << endl;
    err << "       " << "Listens to traffic on named device(s)." << endl << endl;
    err << "   Or: " << args[0] << " [--session] --eavesdrop" << endl;
    err << "       " << "Listens to service signals without subscribing to devices." << endl << endl;
    err << "Use --session to connect to service over the session bus instead of the system bus." << endl;
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();

    qDebug("Socket Sentry Test Client v%s", PROJECT_VERSION);

    Q_ASSERT(args.size() >= 1);
    args.takeFirst();   // remove program name

    // Set up client.
    QDBusConnection bus = args.contains("--session") ? QDBusConnection::sessionBus() : QDBusConnection::systemBus();
    WatcherClient watcher(bus, 0);
    // Try to communicate.
    QString error;
    QStringList devices = watcher.findDevices(error);
    QTextStream err(stderr);
    if (!error.isEmpty()) {
        err << "ERROR: " << error << endl;
        err << "The service has encountered a problem or is not running." << endl;
        printUsage(err);
        return -1;
    } else {
        // Found service. Looks OK.
        WatcherClientConsolePrinter* printer = new WatcherClientConsolePrinter(&watcher);
        if (args.contains("--eavesdrop")) {
            // Eavesdrop only; don't subscribe.
            return app.exec();
        } else {
            // Subscribe to devices.
            QRegExp devicePattern("^[a-zA-Z].*");
            QStringList deviceArgs = args.filter(devicePattern);
            if (deviceArgs.isEmpty()) {
                // No devices selected. Show help.
                err << "Service supports these devices: " << devices.join(" ").toLatin1().constData() << endl;
                printUsage(err);
                delete printer;
                printer = NULL;
                return -1;
            } else {
                // Start watching.
                for (int i = 0; i < deviceArgs.size(); i++) {
                    printer->addDevice(deviceArgs[i]);
                }
                return app.exec();
            }
        }
    }
}

