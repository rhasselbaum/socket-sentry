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

#include "PcapManager.h"

#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QTimerEvent>
#include <QtCore/QMutableListIterator>
#include <QtCore/QHashIterator>

#include "PcapThread.h"
#include "IpEndpointPair.h"

#include <pcap/pcap.h>
#include <sys/types.h>


// Default interval between wake-up times when the manager cleans up threads in limbo.
const int PcapManager::DEFAULT_TIMER_INTERVAL_MS = 10000;

PcapManager::PcapManager() :
    _timerIntervalMs(DEFAULT_TIMER_INTERVAL_MS), _skipDeviceValidation(false) {
    startTimer(_timerIntervalMs);
}

PcapManager::PcapManager(int timerIntervalMs, bool skipDeviceValidation) :
    _timerIntervalMs(timerIntervalMs), _skipDeviceValidation(skipDeviceValidation) {
    startTimer(_timerIntervalMs);
}

PcapManager::~PcapManager() {
}

void PcapManager::timerEvent(QTimerEvent* event) {
    // Clean up threads in limbo if they're finished.
    QMutableListIterator<IPcapThread*> i(_limbo);
    while (i.hasNext()) {
        IPcapThread* thread = i.next();
        if (thread->isDone()) {
            delete thread;
            thread = NULL;
            i.remove();
        }
    }
}

void PcapManager::showInterest(const QString& device) {
    QString error;
    QStringList devices;
    if (!_threads.contains(device) && !_skipDeviceValidation) {
        devices = findAllDevices(error);
    }
    if (_skipDeviceValidation || _threads.contains(device) || devices.contains(device)) {
        bool createNew = true;
        if (_threads.contains(device)) {
            // We already have a capture thread for this device.
            IPcapThread* thread = _threads[device];
            if (thread->keepAlive()) {
                // Thread is still alive and kicking.
                createNew = false;
            } else {
                // Thread is shutting down. Queue it for cleanup and we'll start a new one.
                release(device);
            }
        }
        if (createNew) {
            // We need a new capture thread for this device.
            IPcapThread* thread = createPcapThread(device, _customFilter);
            _threads.insert(device, thread);
            thread->begin();
        }
    } else if (!devices.contains(device)) {
        qWarning("Ingored request to manage unknown device: %s", device.toLatin1().constData());
    }
}

void PcapManager::release(const QString& device) {
    if (_threads.contains(device)) {
        // Cancel the thread and move it to limbo. We'll clean it up when it's finished.
        IPcapThread* thread = _threads[device];
        thread->cancel();
        _threads.remove(device);
        _limbo.append(thread);
    }
}

bool PcapManager::anyTrafficSince(time_t timeSecs) const {
    QHashIterator<QString, IPcapThread*> i(_threads);
    while (i.hasNext()) {
        i.next();
        IPcapThread* thread = i.value();
        if (thread->anyTrafficSince(timeSecs)) return true;
    }
    return false;
}

void PcapManager::releaseAll() {
    QMutableHashIterator<QString, IPcapThread*> i(_threads);
    while (i.hasNext()) {
        i.next();
        IPcapThread* thread = i.value();
        thread->cancel();
        i.remove();
        _limbo.append(thread);
    }
}

bool PcapManager::isActive(const QString& device) const {
    if (_threads.contains(device)) {
        IPcapThread* thread = _threads[device];
        return thread->canContinue();
    }
    return false;
}

bool PcapManager::fillStatistics(const QString& device, QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& result,
        QString& error) const {

    if (_threads.contains(device)) {
        IPcapThread* thread = _threads[device];
        return thread->fillStatistics(result, error);
    } else {
        error = tr("Not capturing packets on device %1").arg(device);
        return false;
    }
}

void PcapManager::restartAll() {
    QStringList currentDevices = _threads.keys();
    releaseAll();
    foreach (QString device, currentDevices) {
        IPcapThread* thread = createPcapThread(device, _customFilter);
        _threads.insert(device, thread);
        thread->begin();
    }
}

QStringList PcapManager::findAllDevices(QString& error) const {
    char errorBuf[PCAP_ERRBUF_SIZE];
    pcap_if_t* interfaces = NULL;
    QStringList result;
    int rc = ::pcap_findalldevs(&interfaces, errorBuf);
    if (rc) {
        error = tr("Can't obtain device list: %1").arg(errorBuf);
    } else {
        if (!interfaces) {
            error = tr("No network devices found.");
        } else {
            pcap_if_t* iface = interfaces;
            while (iface) {
                result.append(iface->name);
                iface = iface->next;
            }
            if (result.isEmpty()) {
                error = tr("Although some network devices were found, none are supported for monitoring.");
            }
        }
    }

    // Clean up.
    if (interfaces) {
        ::pcap_freealldevs(interfaces);
        interfaces = NULL;
    }

    if (!error.isEmpty()) {
        qWarning("Problem finding devices: %s", error.toLatin1().constData());
    } else {
        qDebug("Found devices: %s", result.join(" ").toLatin1().constData());
    }

    return result;
}

QStringList PcapManager::findCurrentDevices() const {
    return _threads.keys();
}

IPcapThread* PcapManager::createPcapThread(const QString& device, const QString& customFilter) {
    return new PcapThread(0, device, customFilter);
}

bool PcapManager::isStopped() const {
    return _threads.isEmpty() && _limbo.isEmpty();
}
