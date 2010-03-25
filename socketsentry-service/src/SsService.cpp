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
#include <QtCore/QTextStream>

#include <unistd.h>

#include "Watcher.h"
#include "WatcherDBusAdaptor.h"
#include "LogSettings.h"

void printUsage(QTextStream& err) {
    QStringList args = QCoreApplication::arguments();
    Q_ASSERT(args.size() >= 1);
    err << endl << "Usage: " << args[0] << " [--session] [--log <proc|pcap|proc,pcap>]" << endl << endl;
    err << "Specify --session to attach to the session bus instead of the system bus." << endl << endl;
    err << "Specify --log proc to log process corrleation stats" << endl;
    err << "        --log pcap to log packet capture stats" << endl;
    err << "        --log proc,pcap to log both" << endl << endl;
}

// If app was passed the "--log" argument, parse out the comma-separated items to be logged
// and initialize the log setting sinlgeton. Remove the arguments from the list. If the
// "--log" argument was not passed in, then no changes are made.
void initLogOptions(QStringList& appArgs) {
    int idx = appArgs.indexOf("--log");
    if (idx >= 0) {
        appArgs.removeAt(idx);  // consume --log option
        if (appArgs.size() > idx) {
            QString logItemsArg = appArgs[idx];
            appArgs.removeAt(idx);  // consume --log option args
            QStringList requestedLogItems = logItemsArg.split(',');
            bool logProcessCorrelation = requestedLogItems.contains("proc");
            bool logPacketCapture = requestedLogItems.contains("pcap");
            LogSettings settings(logProcessCorrelation, logPacketCapture);
            LogSettings::setInstance(settings);
        }
    }
}

// Usage: ./socksent-service [--session] [--log <proc|pcap|proc,pcap>]
// Use --session to attach to the session bus instead of the system bus.
// Use --log proc to log process corrleation stats
//     --log pcap to log packet capture stats
//     --log proc,pcap to log both
int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();

    qDebug("Socket Sentry Service v%s", PROJECT_VERSION);

    // Initiaze logging, which also removes those options from the argument list.
    // This needs to happen before the Watcher is initalized.
    initLogOptions(args);

    // Consume the "--session" arg, if present.
    QTextStream err(stderr);
    bool useSessionBus = args.contains("--session");
    args.removeOne("--session");

    if (args.size() != 1) {
        // An extra (unrecognized) arg was passed in. Show usage and exit.
        printUsage(err);
        return -1;
    } else {
        // Register adapter with the system bus by default or the session bus if "--session" was specified. The
        // latter may be useful for testing, since registering with the session bus usually requires no explicit
        // D-Bus configuration.
        if (geteuid() != 0) {
            qCritical("The service must be run as root.");
            return -1;
        }
        // Initialize the watcher.
        Watcher Watcher;
        WatcherDBusAdaptor* adaptor = new WatcherDBusAdaptor(&Watcher);
        if (adaptor->openForBusiness(!useSessionBus)) {
            qDebug() << "Logging proc correlations :" << LogSettings::getInstance().logProcessCorrelation();
            qDebug() << "Logging packet captures   :" << LogSettings::getInstance().logPacketCapture();
            qDebug() << "Registered Watcher object with D-Bus. Ready for action!";
            return app.exec();
        } else {
            qCritical("Oops! Failed to register Watcher object with D-Bus. Things to check:");
            qCritical(" 1) Is there another instance already running?");
            qCritical(" 2) Is the D-Bus config file installed? (Usually \"/etc/dbus-1/system.d/org.socketsentry.Watcher.conf\".)");
            return -2;
        }
    }
}
