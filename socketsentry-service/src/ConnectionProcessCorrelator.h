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

#ifndef CONNECTIONPROCESSCORRELATOR_H_
#define CONNECTIONPROCESSCORRELATOR_H_

#include "CommonTypes.h"
#include "IConnectionProcessCorrelator.h"
#include "UserNameResolver.h"

#include <QtCore/QRegExp>

template <class K, class V> class QHash;
template <class E> class QList;
class IpEndpointPair;
class OsProcess;
class QHostAddress;
class QString;
class QDateTime;

/*
 * Production implementation of IConnectionProcessCorrelator.
 */
class ConnectionProcessCorrelator : public IConnectionProcessCorrelator {
public:
    ConnectionProcessCorrelator();
    virtual ~ConnectionProcessCorrelator();

    // This method implements behavior described in the interface.
    virtual bool correlate(QHash<IpEndpointPair, QList<OsProcess> >& result, QString& error);

private:
    // Regex matches a line of data from /proc/net/tcp* and /proc/net/udp* and captures important fields.
    static const QString CONN_ENTRY_PATTERN;
    // Regex matches a socket symlink target and captures the inode number.
    static const QString SOCKET_SYMLINK_PATTERN;
    // Regex matches the process name in a /proc/<pid>/status file and captures the name.
    static const QString PROC_NAME_PATTERN;
    // Regex matches the process owner uid in a /proc/<pid>/status file and captures the uid.
    static const QString PROC_UID_PATTERN;

    // Hex string corresponding to a socket in LISTEN state (not a real connection).
    static const QString LISTEN_STATE;

    // Capture groups in connection data regular expressions.
    enum CaptureGroup {
        LocalAddressCaptureGroup = 1,
        LocalPortCaptureGroup,
        RemoteAddressCaptureGroup,
        RemotePortCaptureGroup,
        StateCaptureGroup,
        InodeCaptureGroup
    };

    // Paths to connection tables and socket file descriptors in the /proc filesystem.
    static const QString DEFAULT_TCP4_PATH;
    static const QString DEFAULT_TCP6_PATH;
    static const QString DEFAULT_UDP4_PATH;
    static const QString DEFAULT_UDP6_PATH;
    // Path to the root of the /proc filesystem.
    static const QString PROC_PATH;
    // Template path to the file descrptor directory of a process.
    static const QString FD_PATH_TEMPLATE;
    // Template path to the status pseudo-file of a process.
    static const QString STATUS_FILE_TEMPLATE;

    // Regex object corresponding to V4_REGEX and V6_REGEX.
    const QRegExp _connEntryRegex;
    // Decimal digits.
    const QRegExp _digitsRegex;
    // Regex object corresponding to SOCKET_SYMLINK_PATTERN.
    const QRegExp _socketSymLinkRegex;
    // Regex object corresponding to PROC_NAME_PATTERN.
    const QRegExp _procNameRegex;
    // Regex object corresponding to PROC_UID_PATTERN.
    const QRegExp _procUidRegex;

    // Paths to connection data in the /proc filesystem.
    const QString _tcp4Path;
    const QString _tcp6Path;
    const QString _udp4Path;
    const QString _udp6Path;

    // True if correlation stats should be logged to debug. False, otherwise.
    const bool _logStats;

    // Address lengths in bytes.
    static const uint IPV4_ADDR_LEN;
    static const uint IPV6_ADDR_LEN;

    // Read IP connection data from the /proc filesystem (or equivalent) and create a map of inodes representing socket
    // file descriptors to IP endpoint pairs. The mappings are added to the result argument. The caller supplies the
    // filename of the connection table "file" and the transport protocol. Returns true on success or false if it was
    // not able to read the file. On failure, the error argument is populated with the error message.
    bool findInodes(const QString& filename, const L4Protocol protocol, QHash<int, IpEndpointPair>& result, QString& error) const;

    // Find socket file descriptors of all processes and match them to endpoint pairs via inodes in the supplied
    // connections table. For each entry in the inode-connections table, a new entry is added to the result table
    // with the matching OS processes. The list of processes for each endpoint pair in the result will contain
    // at least one element. If the inode cannot be matched to an OS process, an "empty" OS process is
    // added with pid = 0. If multiple processes share the same socket, entries will be added to the list for
    // all of them in no particular order. The method returns true on success or false if it is unable to read any
    // process data. If false is returned, the error argument will be populated with a message.
    bool findProcessSockets(const QHash<int, IpEndpointPair>& inodeConnections, QHash<IpEndpointPair,
            QList<OsProcess> >& result, QString& error);

    // Given a process ID and start time, fill in an OS process object with details. This is best effort. If the process
    // details cannot be fetched from the OS, only the PID and start time are initialized (copied into) in the result.
    void populateProcess(quint32 pid, const QDateTime& dateTime, OsProcess& result);

    // Scan the input hash table for IPv6 endpoint pairs that actually represent IPv4 hosts mapped into the IPv6 address space.
    // For each such connection, add an equivalent IPv4 connection to the result correlated to the same processes.
    void addMappedIpv4MirrorConnections(QHash<IpEndpointPair, QList<OsProcess> >& result) const;

    // Convert an IP address string from the kernel's connection table into a QHostAddress. Addresses are hex strings
    // with each 32-bit word in host byte order. If the string address cannot be converted for any reason or the
    // address corresponds to the ANY address, then the returned host is set to the default address and its
    // "isNull" method returns true.
    QHostAddress convertIpAddress(const QString& address) const;

    // Process owner name resolver.
    UserNameResolver _userNameResolver;

};

#endif /* CONNECTIONPROCESSCORRELATOR_H_ */
