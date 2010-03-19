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

#include "WatcherClientConsolePrinter.h"
#include "WatcherClient.h"
#include "CommunicationFlow.h"

#include <QtCore/QTimerEvent>
#include <QtCore/QSetIterator>
#include <QtCore/QList>
#include <QtCore/QListIterator>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>


// The interval between device interest renewals.
const int WatcherClientConsolePrinter::RENEWAL_INTERVAL_MS = 5000;

WatcherClientConsolePrinter::WatcherClientConsolePrinter(WatcherClient* parent) :
    QObject(parent) {

    connect(this, SIGNAL(renew(const QString&)), parent, SLOT(showInterest(const QString&)));
    connect(parent, SIGNAL(update(const QString&, const QList<CommunicationFlow>&)),
            this, SLOT(printUpdate(const QString&, const QList<CommunicationFlow>&)));
    connect(parent, SIGNAL(failure(const QString&, const QString&)),
            this, SLOT(printFailure(const QString&, const QString&)));
    startTimer(RENEWAL_INTERVAL_MS);
}

WatcherClientConsolePrinter::~WatcherClientConsolePrinter() {
}

void WatcherClientConsolePrinter::timerEvent(QTimerEvent* event) {
    QSetIterator<QString> i(_devices);
    while (i.hasNext()) {
        emit renew(i.next());
    }
}

void WatcherClientConsolePrinter::printUpdate(const QString& device, const QList<CommunicationFlow>& flows) {
    QTextStream out(stdout);
    QString prefix = QString("[%1]: ").arg(device);
    QDateTime now = QDateTime::currentDateTime();
    out << prefix << "-- Signal at " << now.toString("yyyy-MMM-dd hh:mm:ss.zzz") << endl;
    QListIterator<CommunicationFlow> i(flows);
    while (i.hasNext()) {
        out << prefix << i.next().toString() << endl;
    }
    out.flush();
}

void WatcherClientConsolePrinter::printFailure(const QString& device, const QString& error) {
    QTextStream out(stdout);
    out << QString("[%1]: -- ERROR: %2").arg(device).arg(error) << endl;
    out.flush();
}
