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

#include <QtCore/QMutexLocker>
#include <QtNetwork/QNetworkInterface>

#include <sys/time.h>
#include <pcap/pcap.h>

#include "PcapThread.h"
#include "IpEndpointPair.h"
#include "FlowMetrics.h"
#include "DataLinkPacketDecoder.h"
#include "CommonTypes.h"

const int PcapThread::DEFAULT_TIMEOUT_SECS = 30;
const int PcapThread::SNAPLEN = 128;

PcapThread::PcapThread(QObject* parent, const QString& device, const QString& customFilter) :
    QThread(parent), _mutex(QMutex::Recursive), _device(device), _customFilter(customFilter),
    _timeoutSecs(DEFAULT_TIMEOUT_SECS), _pcapHandle(NULL), _dataLinkDecoder(NULL), _networkInterface(NULL) {

    // Do we have a network interface?
    QNetworkInterface iface = QNetworkInterface::interfaceFromName(device);
    if (iface.isValid()) {
        _networkInterface = new QNetworkInterface(iface);
        _ipDecoder.setLocalAddresses(_networkInterface->addressEntries());
    }

    // Update last ping to now.
    timeval tv;
    ::gettimeofday(&tv, NULL);
    _sharedMutables.lastPing = tv.tv_sec;
    _sharedMutables.canceled = false;
}

PcapThread::~PcapThread() {
    delete _networkInterface;
    _networkInterface = NULL;
}


bool PcapThread::keepAlive() {
    QMutexLocker locker(&_mutex);
    bool success = canContinue();
    if (success) {
        timeval tv;
        ::gettimeofday(&tv, NULL);
        _sharedMutables.lastPing = tv.tv_sec;
    }
    return success;
}

void PcapThread::cancel() {
    QMutexLocker locker(&_mutex);
    _sharedMutables.canceled = true;
}

bool PcapThread::canContinue() const {
    QMutexLocker locker(&_mutex);
    return !isExpired() && _sharedMutables.lastError.isEmpty() && !_sharedMutables.canceled;
}

bool PcapThread::isExpired() const {
    QMutexLocker locker(&_mutex);
    time_t expireTime = _sharedMutables.lastPing + _timeoutSecs;
    timeval tv;
    ::gettimeofday(&tv, NULL);
    time_t currTime = tv.tv_sec;
    return expireTime < currTime;
}

void PcapThread::run() {
    Q_ASSERT(_pcapHandle == NULL);
    keepAlive();

    // WARNING: We do not lock the mutex through most of this. Hands off mutable shared state!
    QString initMessage;
    QString captureError;
    _pcapHandle = initializePcap(initMessage);
    if (_pcapHandle) {
        _dataLinkDecoder = DataLinkPacketDecoder::fromLinkType(pcap_datalink(_pcapHandle), _networkInterface);
        if (_dataLinkDecoder) {
            if (!initMessage.isEmpty()) {
                qWarning("[%s]: Packet capture started with warning. (%s)",
                        (const char*)_device.toLatin1(), (const char*)initMessage.toLatin1());
            } else {
                qDebug("[%s]: Packet capture started.", (const char*)_device.toLatin1());
            }
            qDebug("[%s]: Using %s packet decoder.",
                (const char*)_device.toLatin1(), (const char*)_dataLinkDecoder->name().toLatin1());
            // Main capture loop.
            int loopStatus = 0;
            _startupLatch.flip();   // Let other threads know we're up and running.
            while (canContinue() && loopStatus >= 0) {
                loopStatus = pcap_loop(_pcapHandle, -1, PcapThread::packetCallback, reinterpret_cast<u_char*>(this));
            }
            if (loopStatus == -1) {
                captureError = tr("Failed during packet capture. (%1)").arg(pcap_geterr(_pcapHandle));
            }
            delete _dataLinkDecoder;
            _dataLinkDecoder = NULL;
        } else {
            // Don't know how to decode packets for the data link type.
            captureError = tr("Unsupported data link layer type %1.").arg(pcap_datalink(_pcapHandle));
        }
        pcap_close(_pcapHandle);
        _pcapHandle = NULL;
    } else {
        // Failed to get a pcap handle.
        captureError = initMessage;
    }

    // Emit any error and shut down.
    if (!captureError.isEmpty()) {
        _mutex.lock();
        _sharedMutables.lastError = captureError;
        _mutex.unlock();
        captureError = "[" + _device + "]: " + captureError;
        qWarning("%s", (const char*)captureError.toLatin1());
    }
    _startupLatch.flip();   // In case of error, let other threads know we tried to start.
    qDebug("[%s]: Capture thread is shutting down.", (const char*)_device.toLatin1());
}

pcap_t* PcapThread::initializePcap(QString& message) const {
    char errbuf[PCAP_ERRBUF_SIZE];
    const int READ_TIMEOUT = 333;	// wake up every 1/3 second
    bool successful = false;
    pcap_t* pcapHandle = pcap_create(_device.toAscii(), errbuf);
    if (pcapHandle) {
        // Set capture options.
        pcap_set_snaplen(pcapHandle, SNAPLEN);		// just headers
        pcap_set_timeout(pcapHandle, READ_TIMEOUT);

        // We only want TCP/UDP and user's custom criteria (if any).
        bpf_program filterProg;
        QString filterText("(tcp or udp)");
        if (_customFilter.length() > 0) {
            filterText += " and (" + _customFilter + ")";
        }

        // Activate.
        int pcapFailed = pcap_activate(pcapHandle);
        if (!pcapFailed || pcapFailed == PCAP_WARNING) {
            if (pcapFailed == PCAP_WARNING) {
                message = pcap_geterr(pcapHandle);
            }
            // Apply the filter here. This has to be done after activation.
            pcapFailed = pcap_compile(pcapHandle, &filterProg, filterText.toAscii(), 1, 0);
            if (!pcapFailed) {
                pcapFailed = pcap_setfilter(pcapHandle, &filterProg);
            }
            if (!pcapFailed) {
                // DONE!
                successful = true;
            } else {
                // Failed to apply filter.
                message = tr("Can't apply filter \"%1\". (%2)").arg(filterText).arg(pcap_geterr(pcapHandle));
            }
        } else if (pcapFailed == PCAP_ERROR_IFACE_NOT_UP) {
            // Common case of iface being offline.
            message = tr("Can't activate packet capture because device is offline.");
        } else {
            // General failure to activate.
            message = tr("Can't activate packet capture. (%1)").arg(pcap_geterr(pcapHandle));
        }

        if (!successful) {
            pcap_close(pcapHandle);
            pcapHandle = NULL;
        }
    } else {
        // Failed to get a pcap handle.
        message = tr("Can't initialize packet capture. (%1)").arg(errbuf);
    }
    return pcapHandle;
}

void PcapThread::processPacket(const pcap_pkthdr* pcapHeader, const u_char* bytes) {
    if (!canContinue()) {
        pcap_breakloop(_pcapHandle);
    } else {
        Q_ASSERT(_dataLinkDecoder);
        // Decode data link layer packet to determine start of IP packet and directionality (if applicable).
        IpHeader ipHeader = _dataLinkDecoder->decode(pcapHeader, bytes);
        if (ipHeader.start && ipHeader.start < bytes + pcapHeader->caplen) {
            // We seem to have a valid IP packet. Decode the packet to determine endpoints and directionality.
            int ipCapLen = bytes + pcapHeader->caplen - ipHeader.start;
            IpEndpointPair endpoints;
            Direction direction = _ipDecoder.decode(ipHeader.direction, ipCapLen, ipHeader.start,  endpoints);
            if (direction != UNKNOWN_DIRECTION) {
                // Found a valid IP packet. Accumulate metrics and add to history.
                FlowMetrics metrics;
                if (direction == INBOUND) {
                    metrics.setBytesIn(pcapHeader->len);
                    metrics.setPacketsIn(1);
                } else {
                    metrics.setBytesOut(pcapHeader->len);
                    metrics.setPacketsOut(1);
                }
                time_t capTime = pcapHeader->ts.tv_sec;
                _mutex.lock();
                _sharedMutables.history.record(endpoints, metrics, capTime);
                _mutex.unlock();
                qDebug("[%s]: %s %s %d bytes", _device.toLatin1().constData(),
                        (direction == INBOUND) ? "In " : "Out",
                        endpoints.toString().toLatin1().constData(),
                        pcapHeader->len);
            } // Else, couldn't decode IP packet. Oh well.
        } // Else, not an IP packet. Oh well.
    }
}

bool PcapThread::fillStatistics(QHash<IpEndpointPair, QPair<FlowMetrics, FlowStatistics> >& result, QString& error) {
    _startupLatch.wait();   // Wait for startup in case there's an error we need to pick up.
    QMutexLocker locker(&_mutex);
    if(!_sharedMutables.lastError.isEmpty()) {
        error = _sharedMutables.lastError;
        return false;
    } else {
        timeval tv;
        ::gettimeofday(&tv, NULL);
        _sharedMutables.history.exportStatistics(result, tv.tv_sec);
        return true;
    }
}

void PcapThread::packetCallback(u_char* obj, const pcap_pkthdr* header, const u_char* bytes) {
    PcapThread* pcapThread = reinterpret_cast<PcapThread*>(obj);
    pcapThread->processPacket(header, bytes);
}

void PcapThread::begin() {
    start();
}

bool PcapThread::isDone() const {
    return isFinished();
}

bool PcapThread::anyTrafficSince(time_t timeSecs) const {
    QMutexLocker locker(&_mutex);
    return _sharedMutables.history.anyTrafficSince(timeSecs);
}
