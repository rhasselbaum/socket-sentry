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


#ifndef IPCAPMANAGER_H_
#define IPCAPMANAGER_H_

class QString;
template <class K, class V> class QHash;
template <class F, class S> class QPair;
class FlowMetrics;
class FlowStatistics;
class IpEndpointPair;

/*
 * Manages packet capture threads for a set of devices. Clients express interest in capturing packets on a particular
 * device by calling the "showInterest" method periodically, which either creates a new capture thread or keeps an old
 * one alive. When a capture is in progress, the client can call "fillStatistics" to get current traffic data for the
 * device. The manager also exposes methods to stop capturing on one or all devices ("release"), or it can simply wait
 * for the captures to time out by NOT calling "showInterest" and eventually the manager will clean up the resources for
 * the capture. Note that since the manager spawns any number of threads, clients should NOT delete this object without
 * shutting it down properly to avoid leaks. To do this, one calls "releaseAll" and then polls "isStopped" perioidcally,
 * which returns true when there are no threads. (Of course, if the process is shutting down, we don't worry so much.)
 */
class IPcapManager {
public:
    virtual ~IPcapManager() { }

    // Show interest in a device for a period of time. The manager will begin capturing packets for this device if it
    // is not already doing so, and the client can obtain capture statistics for the device by calling "exportStatistics".
    // If this manager is already capturing packets for the device, then calling this method will continue to keep the
    // packet capture thread alive for a renewed period of time. Clients must show interest periodically in order to
    // get uninterrupted access to statistics. Otherwise, the manager will eventually stop capturing packets for the
    // device. If the caller supplies an invalid device name, no action is taken.
    virtual void showInterest(const QString& device) = 0;

    // Fund the list of network devices that can be watched. Updates error argument if list
    // cannot be obtained due to lack of permission or another problem.
    virtual QStringList findAllDevices(QString& error) const = 0;

    // Return a list of all devices that the manager is currently managing.
    virtual QStringList findCurrentDevices() const = 0;

    // Fill the result structure with the latest metrics and statistics from captured packets on the given device.
    // Returns true on success, false otherwise. If false, the given error argument will be filled with a message explaining
    // the problem.
    virtual bool fillStatistics(const QString& device, QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& result,
            QString& error) const = 0;

    // Returns true if the manager is currently capturing on the device. Returns false if the manager is not managing
    // the device OR the capture has stopped due to an error, expiration, or cancellation. If a client finds that a
    // previously active capture has become inactive, it may call "release" to reset the device, cleaning any error state.
    virtual bool isActive(const QString& device) const = 0;

    // Stop capturing packets for this device and mark resources for cleanup,
    virtual void release(const QString& device) = 0;

    // Stop capturing packets on all devices and mark resources for cleanup.
    virtual void releaseAll() = 0;

    // Check to see if any devices have seen traffic since the specified time.
    virtual bool anyTrafficSince(time_t timeSecs) const = 0;

    // Returns true if the manager is not managing any packet capture threads.
    virtual bool isStopped() const = 0;

    // A custom filter using pcap syntax that is applied across all devices.
    virtual QString getCustomFilter() const = 0;
    virtual void setCustomFilter(const QString& customFilter) = 0;

};

#endif /* IPCAPMANAGER_H_ */
