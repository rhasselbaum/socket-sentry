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


#ifndef IPCAPTHREAD_H_
#define IPCAPTHREAD_H_

template <class K, class V> class QHash;
template <class F, class S> class QPair;
class IpEndpointPair;
class FlowMetrics;
class FlowStatistics;
class QString;

/*
 * Defines the interface of A thread that captures packets from a device to construct a summaries of
 * traffic statistcs over time. Once started, the thread captures packets on the device and keeps a
 * rolling history of network activity until it encounters an error, it is canceled, or a timeout occurs.
 * Clients can keep the thread running indefinitely by calling its "keep alive" method periodically,
 * which resets the timeout. If a timeout does occur, this thread is considered "expired" and
 * should eventually shut itself down. Once this thread has expired, been canceled, or encounters an
 * error, it cannot be reused.
 *
 * Implementations must be reentrant and thread-safe. Public methods can be called from any thread.
 */
class IPcapThread {
public:
    virtual ~IPcapThread() { }

    // Request to keep this thread alive. Returns true if the request succeeds. Returns false if the thread
    // is already in the process of shutting down in which case the client should create a new instance instead.
    virtual bool keepAlive() = 0;

    // Stop the packet capture and shut down without waiting for the timeout. This call works asynchronously.
    // The thread may continue processing for some time, but will eventually shut down.
    virtual void cancel() = 0;

    // Fill the result structure with the latest metrics and statistics from captured packets. Returns true
    // on success, false otherwise. If false, the given error argument will be filled with a message explaining
    // the problem.
    virtual bool fillStatistics(QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& result, QString& error) = 0;

    // Start the thread running. (Synonym for non-virtual method QThread::start.)
    virtual void begin() = 0;

    // Has the thread finished running? (Synonym for non-virtual method QThread::isFinished.)
    virtual bool isDone() const = 0;

    // Check to see if the device has seen traffic since the specified time.
    virtual bool anyTrafficSince(time_t timeSecs) const = 0;

    // True only if the thread has not expired, has no error, and has not been canceled.
    // When the thread can no longer continue, it automatically ends the packet capture and
    // shuts down.
    virtual bool canContinue() const = 0;

};

#endif /* IPCAPTHREAD_H_ */
