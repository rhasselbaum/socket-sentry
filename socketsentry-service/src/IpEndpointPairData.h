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

#ifndef IPENDPOINTPAIRDATA_H_
#define IPENDPOINTPAIRDATA_H_

#include "CommonTypes.h"

#include <QtCore/QSharedData>
#include <QtNetwork/QHostAddress>
#include <QtCore/QString>

/*
 * Shared data for IpEndpointPair.
 */
class IpEndpointPairData : public QSharedData {
public:
    IpEndpointPairData();
    virtual ~IpEndpointPairData();

    QHostAddress localAddr;
    ushort localPort;
    QHostAddress remoteAddr;
    QString remoteHostName;
    ushort remotePort;
    L4Protocol transport;
};

#endif /* IPENDPOINTPAIRDATA_H_ */
