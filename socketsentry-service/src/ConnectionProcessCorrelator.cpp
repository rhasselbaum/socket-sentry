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

#include "ConnectionProcessCorrelator.h"

#include "OsProcess.h"
#include "IpEndpointPair.h"
#include "OsProcess.h"
#include "CommonTypes.h"
#include "LogSettings.h"

#include <netinet/in.h>
#include <pwd.h>

#include <QtCore/QHash>
#include <QtCore/QHashIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QListIterator>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>

// The following regex matches a line of data from /proc/net/tcp* or /proc/net/udp* and captures (in order)
//   1) Local address in hex.
//   2) Local port in hex.
//   3) Remote address in hex.
//   4) Remote port in hex.
//   5) Connection state.
//   6) Inode (socket identifier).
//
// Example data (TCP over IPv4):
//  sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode ref pointer drops
//  49: 00000000:0044 00000000:0000 07 00000000:00000000 00:00000000 00000000     0        0 6703 2 ffff880123d0c000 0
//  55: 0100007F:1F4A 00000000:0000 07 00000000:00000000 00:00000000 00000000  1000        0 8625 2 ffff88012545cd00 0
//  84: 00000000:CE67 00000000:0000 07 00000000:00000000 00:00000000 00000000  1000        0 8643 2 ffff88012545d040 0
//  86: 00000000:14E9 00000000:0000 07 00000000:00000000 00:00000000 00000000   105        0 5628 2 ffff88012545c000 0
// 126: 00000000:8D11 00000000:0000 07 00000000:00000000 00:00000000 00000000   105        0 5629 2 ffff88012545c340 0
//
// Note that the only difference between IPv4 and v6 in these tables is the size of the addresses.
const QString ConnectionProcessCorrelator::CONN_ENTRY_PATTERN("\\s*\\d+\\: (\\w{8,32})\\:(\\w{4}) (\\w{8,32})\\:(\\w{4}) (\\w{2}) (?:[\\w:]+\\s+){5}(\\w+)[^\\r\\n]*");
// Regex matches a socket symlink target in /proc/<pid>/fd and captures the inode number.
const QString ConnectionProcessCorrelator::SOCKET_SYMLINK_PATTERN("socket\\:\\[(\\d+)\\]");
// Regex matches the process name in a /proc/<pid>/status file and captures the name.
const QString ConnectionProcessCorrelator::PROC_NAME_PATTERN("Name\\:\\s*(\\S.+)");
// Regex matches the process owner uid in a /proc/<pid>/status file and captures the uid.
const QString ConnectionProcessCorrelator::PROC_UID_PATTERN("Uid\\:\\s*(\\d+)");

// Hex string corresponding to a socket in LISTEN state (not a real connection).
const QString ConnectionProcessCorrelator::LISTEN_STATE = "0A";

// Paths to connection tables and socket file descriptors in the /proc filesystem.
const QString ConnectionProcessCorrelator::DEFAULT_TCP4_PATH("/proc/net/tcp");
const QString ConnectionProcessCorrelator::DEFAULT_TCP6_PATH("/proc/net/tcp6");
const QString ConnectionProcessCorrelator::DEFAULT_UDP4_PATH("/proc/net/udp");
const QString ConnectionProcessCorrelator::DEFAULT_UDP6_PATH("/proc/net/udp6");
// Path to the root of the /proc filesystem.
const QString ConnectionProcessCorrelator::PROC_PATH("/proc");
// Template path to the file descrptor directory of a process.
const QString ConnectionProcessCorrelator::FD_PATH_TEMPLATE("/proc/%1/fd");
// // Template path to the status pseudo-file of a process.
const QString ConnectionProcessCorrelator::STATUS_FILE_TEMPLATE("/proc/%1/status");

// IP address lengths.
const uint ConnectionProcessCorrelator::IPV4_ADDR_LEN = 4;
const uint ConnectionProcessCorrelator::IPV6_ADDR_LEN = 16;

ConnectionProcessCorrelator::ConnectionProcessCorrelator() :
    _connEntryRegex(CONN_ENTRY_PATTERN), _digitsRegex("\\d+"), _socketSymLinkRegex(SOCKET_SYMLINK_PATTERN),
    _procNameRegex(PROC_NAME_PATTERN), _procUidRegex(PROC_UID_PATTERN), _tcp4Path(DEFAULT_TCP4_PATH),
    _tcp6Path(DEFAULT_TCP6_PATH), _udp4Path(DEFAULT_UDP4_PATH), _udp6Path(DEFAULT_UDP6_PATH),
    _logStats(LogSettings::getInstance().logProcessCorrelation()) {
}

ConnectionProcessCorrelator::~ConnectionProcessCorrelator() {
}

bool ConnectionProcessCorrelator::correlate(QHash<IpEndpointPair, QList<OsProcess> >& result, QString& error) const {

    // Get current connections by inode.
    QHash<int, IpEndpointPair> endpointsByInode;
    bool ok = findInodes(_tcp4Path, TCP, endpointsByInode, error)
            && findInodes(_tcp6Path, TCP6, endpointsByInode, error)
            && findInodes(_udp4Path, UDP, endpointsByInode, error)
            && findInodes(_udp6Path, UDP6, endpointsByInode, error);
    if (!ok) return false;
    // Find related sockets and processes.
    ok = findProcessSockets(endpointsByInode, result, error);
    if (!ok) return false;
    // Sometimes the kernel gives us IPv4 addresses mapped into IPv6 address space. Create IPv4 mirror entires
    // for these endpoint pairs.
    addMappedIpv4MirrorConnections(result);
    return true;
}

void ConnectionProcessCorrelator::addMappedIpv4MirrorConnections(QHash<IpEndpointPair, QList<OsProcess> >& result) const {
    // Can't add to the hash while we're iterating over it, so we need to do this in two steps.
    QList<const IpEndpointPair*> mappedPairs;
    QHashIterator<IpEndpointPair, QList<OsProcess> > hashIter(result);
    while (hashIter.hasNext()) {
        hashIter.next();
        const IpEndpointPair& endpoints = hashIter.key();
        if (endpoints.isIpv4MappedAs6()) {
            mappedPairs.append(&endpoints);
        }
    }

    QListIterator<const IpEndpointPair*> listIter(mappedPairs);
    while (listIter.hasNext()) {
        const IpEndpointPair* mappedPair = listIter.next();
        const QList<OsProcess>& osProcesses = result[*mappedPair];
        result.insert(mappedPair->demoteIpv6To4(), osProcesses);
    }

}

bool ConnectionProcessCorrelator::findInodes(const QString& filename, const L4Protocol protocol,
        QHash<int, IpEndpointPair>& result, QString& error) const {

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString contents = in.readAll();
        if (!contents.isNull()) {
            int pos = 0;
            // Loop over each line of connection data, which matches our regex.
            while ((pos = _connEntryRegex.indexIn(contents, pos)) != -1) {
                pos += _connEntryRegex.matchedLength();		// advance no matter what

                // Parse line of connection data to get the fields we're interested in.
                QString state = _connEntryRegex.cap(StateCaptureGroup);
                if (state.compare(LISTEN_STATE, Qt::CaseInsensitive) == 0) continue;	// not a real connection

                // Decode local and remote address and ports from hex strings.
                QHostAddress localAddr = convertIpAddress(_connEntryRegex.cap(LocalAddressCaptureGroup));
                if (localAddr.isNull()) continue;
                QHostAddress remoteAddr = convertIpAddress(_connEntryRegex.cap(RemoteAddressCaptureGroup));
                if (remoteAddr.isNull()) continue;
                bool ok = false;
                ushort localPort = _connEntryRegex.cap(LocalPortCaptureGroup).toUShort(&ok, 16);  // hex to bin
                if (!ok) continue;
                ushort remotePort = _connEntryRegex.cap(RemotePortCaptureGroup).toUShort(&ok, 16);  // hex to bin
                if (!ok) continue;

                // And now the inode key.
                int inode = _connEntryRegex.cap(InodeCaptureGroup).toInt(&ok, 10);  // dec to bin
                if (!ok || inode == 0) continue;

                // All checks out. Add to the result.
                IpEndpointPair& endpoints = result[inode];
                endpoints.setLocalAddr(localAddr);
                endpoints.setLocalPort(localPort);
                endpoints.setRemoteAddr(remoteAddr);
                endpoints.setRemotePort(remotePort);
                endpoints.setTransport(protocol);

                //
            }
        }
        return true;
    } else {
        error = QObject::tr("Can't access file %1").arg(filename);
        return false;
    }
}

QHostAddress ConnectionProcessCorrelator::convertIpAddress(const QString& address) const {
    QHostAddress result;

    // Kernel gives us IP addresses in host byte order. QHostAddress expects IPv4 addresses in host byte order but
    // IPv6 addresses in network byte order. So we need to handle each case separately.
    QByteArray bytes = QByteArray::fromHex(address.toAscii());
    if (bytes.size() == IPV4_ADDR_LEN) {
        result.setAddress(*(const quint32*)bytes.constData());
    } else if (bytes.size() == IPV6_ADDR_LEN) {
        Q_ASSERT(bytes.size() % 4 == 0);
        // Copy the input address as 32-bit words, converting to network byte order as we go.
        const int words = IPV6_ADDR_LEN / 4;
        quint32 netBytes[words];
        const quint32* hostBytes = (const quint32*)bytes.constData();
        for (int i = 0; i < words; i++) {
            netBytes[i] = htonl(hostBytes[i]);
        }
        result.setAddress((quint8*)netBytes);
    }

    // Discard address if it is the ANY address.
    if (result == QHostAddress::Any || result == QHostAddress::AnyIPv6) {
        result.clear();
    }

    return result;
}

bool ConnectionProcessCorrelator::findProcessSockets(const QHash<int, IpEndpointPair>& inodeConnections,
        QHash<IpEndpointPair, QList<OsProcess> >& result, QString& error) const {

    if (inodeConnections.isEmpty()) return true;     // no connections to be mapped

    QDir procDir(PROC_PATH);
    QFileInfoList procEntries = procDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (!procEntries.isEmpty()) {
        // Iterate over all process pseudo-directories.
        QListIterator<QFileInfo> procIter(procEntries);
        while (procIter.hasNext()) {
            const QFileInfo& procEntry = procIter.next();
            const QString& procEntryFileName = procEntry.fileName();
            const QDateTime& procStartTime = procEntry.created();
            if (_digitsRegex.exactMatch(procEntryFileName)) {
                // This directory represents a process.
                quint32 pid = procEntryFileName.toUInt();
                if (pid > 0) {
                    // Find all file descriptors for this process.
                    QDir fdDir(FD_PATH_TEMPLATE.arg(procEntryFileName));
                    QFileInfoList fdEntries = fdDir.entryInfoList();
                    QListIterator<QFileInfo> fdIter(fdEntries);
                    OsProcess osProcess;
                    while (fdIter.hasNext()) {
                        // Is the file descriptor a socket?
                        const QFileInfo& fdInfo = fdIter.next();
                        QString symLinkTarget = fdInfo.symLinkTarget();
                        if (_socketSymLinkRegex.indexIn(symLinkTarget) >= 0) {
                            // File descriptor is a socket. Grab the inode number (first capture group).
                            int inode = _socketSymLinkRegex.cap(1).toInt();
                            if (inode > 0 && inodeConnections.contains(inode)) {
                                // It's a socket AND we know the matching endpoint pair. Add a result entry.
                                const IpEndpointPair& endpoints = inodeConnections[inode];
                                // Have the process details already been fetched?
                                if (osProcess.getPid() == 0) {
                                    // No. Fill in the details.
                                    populateProcess(pid, procStartTime, osProcess);
                                }
                                QList<OsProcess>& osProcesses = result[endpoints];     // adds an entry if needed
                                osProcesses.append(osProcess);
                            }
                        }
                    }
                }
            }
        }

        if (_logStats) {
            qDebug() << "Mapped" << result.size() << "out of" << inodeConnections.size() << "connection(s) to OS processes.";
        }

        // Fill out the result for any inodes that weren't found. Any connections not already mapped to a
        // process gets mapped to an "empty" one here.
        QHashIterator<int, IpEndpointPair> connectionsIter(inodeConnections);
        while (connectionsIter.hasNext()) {
            connectionsIter.next();
            const IpEndpointPair& endpoints = connectionsIter.value();
            if (! result.contains(endpoints)) {
                QList<OsProcess> osProcesses;
                osProcesses.append(OsProcess());
                result.insert(endpoints, osProcesses);
            }
        }
        return true;
    } else {
        error = QObject::tr("Can't access directory %1").arg(PROC_PATH);
        return false;
    }
}

void ConnectionProcessCorrelator::populateProcess(quint32 pid, const QDateTime& startTime, OsProcess& result) const {
    result.setPid(pid);
    result.setStartTime(startTime);
    QFile file(STATUS_FILE_TEMPLATE.arg(pid));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine();
        bool foundName = false;
        bool foundUid = false;
        while (!line.isNull() && (!foundName || !foundUid)) {
            if (!foundName && _procNameRegex.indexIn(line) >= 0) {
                result.setProgram(_procNameRegex.cap(1));
                foundName = true;
            } else if (!foundUid && _procUidRegex.indexIn(line) >= 0) {
                uid_t uid = _procUidRegex.cap(1).toUInt();
                passwd* userRec = ::getpwuid(uid);
                if (userRec) {
                    result.setUser(userRec->pw_name);
                }
                foundUid = true;
            }
            line = in.readLine();
        }
    }
}
