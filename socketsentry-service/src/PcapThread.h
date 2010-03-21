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

#ifndef PCAPTHREAD_H_
#define PCAPTHREAD_H_

#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QMutex>

#include "NetworkHistory.h"
#include "InternetProtocolDecoder.h"
#include "IPcapThread.h"
#include "Latch.h"

struct pcap_pkthdr;
typedef struct pcap pcap_t;
class QNetworkInterface;
class DataLinkPacketDecoder;

/*
 * Main implementation of IPcapThread.
 *
 * This class is reentrant and thread-safe. Public methods can be called from any thread.
 */
class PcapThread : public QThread, public IPcapThread {
    Q_OBJECT

public:
    // Create a new capture thread for the given device. Optionally, the caller can pass a non-empty
    // packet filter to restrict captured traffic. The filter must be written in libpcap filter language.
    PcapThread(QObject* parent, const QString& device, const QString& customFilter);
    virtual ~PcapThread();

    // These methods implement behavior described in the interface.
    virtual bool keepAlive();
    virtual void cancel();
    virtual bool fillStatistics(QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& result, QString& error);
    virtual void begin();
    virtual bool isDone() const;
    virtual bool anyTrafficSince(time_t timeSecs) const;
    virtual bool canContinue() const;

protected:
    virtual void run();

private:

    // Create and initialize a new capture handle. The handle is activated before return. If an error occurs,
    // NULL is returned and the string argument is populated with the error. If the handle is created but
    // there is a warning, both the return value and the message argument are initialized. This method
    // has no side-effects on object state. The caller takes responsibility for updating the pcap handle
    // member (and data link decoder).
    pcap_t* initializePcap(QString& message) const;

    // Process a new packet produced by the pcap library.
    void processPacket(const pcap_pkthdr* pcapHeader, const u_char* bytes);

    // True if the thread has been running for too long without a call to keep it alive.
    // Once a thread has expired, it automatically ends the packet capture and shuts down.
    bool isExpired() const;

    // Callback invoked by the pcap library upon successful packet capture.
    static void packetCallback(u_char* obj, const pcap_pkthdr* header, const u_char* bytes);

    // Immutables
    static const int DEFAULT_TIMEOUT_SECS;      // default max time the thread stays alive without a ping
    static const int SNAPLEN;                   // max captured packet length; we only need headers
    const QString _device;                      // the OS device name
    const QString _customFilter;                // if specifiied, it'll be added to the capture filter
    const int _timeoutSecs;                     // max time the thread stays alive without a ping
    const bool _logStats;                       // true if packet capture stats should be logged to debug

    // Mutex for shared mutable state.
    mutable QMutex _mutex;

    // Blocks other threads until this object has completed or failed initialization.
    Latch _startupLatch;

    // These members are shared with multiple threads and mutable. Protect access to them with mutex.
    struct Shared {
        time_t lastPing;                        // last time a client expressed interest in our traffic
        QString lastError;                      // last capture error
        bool canceled;                          // true if the client wants us to shutdown
        NetworkHistory history;                 // rolling history of network traffic
    } _sharedMutables;

    // Unshared (thread private)
    pcap_t* _pcapHandle;                            // capture handle
    const QNetworkInterface* _networkInterface;     // the network interface (if it is known)
    DataLinkPacketDecoder* _dataLinkDecoder;        // decodes data link layer packets
    InternetProtocolDecoder _ipDecoder;             // decodes network layer packets (and a little TCP/UDP)

};

#endif /* PCAPTHREAD_H_ */
