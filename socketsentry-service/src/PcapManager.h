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

#ifndef PCAPMANAGER_H_
#define PCAPMANAGER_H_

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QObject>

#include "IPcapManager.h"

class QString;
class IPcapThread;
class QTimerEvent;
class FlowMetrics;
class FlowStatistics;
class IpEndpointPair;


/*
 * Main implementation of IPcapManager.
 */
class PcapManager : public QObject, public IPcapManager {
    Q_OBJECT

public:
    // New instance.
    PcapManager();

    virtual ~PcapManager();

    // These methods implement behavior described in the interface.
    virtual void showInterest(const QString& device);
    virtual QStringList findAllDevices(QString& error) const;
    virtual QStringList findCurrentDevices() const;
    virtual bool fillStatistics(const QString& device, QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& result,
            QString& error) const;
    virtual void release(const QString& device);
    virtual void releaseAll();
    virtual bool anyTrafficSince(time_t timeSecs) const;
    virtual bool isStopped() const;
    virtual bool isActive(const QString& device) const;
    virtual QString getCustomFilter() const { return _customFilter; }
    virtual void setCustomFilter(const QString& customFilter) {
        if (customFilter != _customFilter) {
            _customFilter = customFilter;
            restartAll();
        }
    }

protected:
    // New instance with the given timer interval. Useful for unit test subclasses.
    PcapManager(int timerIntervalMs, bool skipDeviceValidation);

    // Clean up threads in limbo.
    void timerEvent(QTimerEvent* event);

    // Factory method to create a new IPcapThread instance. May be overridden in a subclass for unit tests (mock threads).
    virtual IPcapThread* createPcapThread(const QString& device, const QString& customFilter);

private:
    // Restart all capture threads, applying the current custom filter (if any).
    void restartAll();

    // Interval between wake-up times when the manager cleans up threads in limbo.
    static const int DEFAULT_TIMER_INTERVAL_MS;
    const int _timerIntervalMs;

    // If true, the manager will not validate device names against the pcap library prior to starting a capture thread.
    // This is useful for unit testing only.
    bool _skipDeviceValidation;

    // List of threads in the process of shutting down, after which the objects may be deleted.
    QList<IPcapThread*> _limbo;

    // Table of capture threads by device. The presence of entries in this table means we have some
    // potential work to do each time the timer wakes us up.
    QHash<QString, IPcapThread*> _threads;

    // Custom packet filter expression applied to all devices if not empty. Must be in pcap syntax.
    QString _customFilter;

};

#endif /* PCAPMANAGER_H_ */
