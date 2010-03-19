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


#ifndef MOCKCONNECTIONPROCESSCORRELATOR_H_
#define MOCKCONNECTIONPROCESSCORRELATOR_H_

#include "IConnectionProcessCorrelator.h"

#include "OsProcess.h"
#include "IpEndpointPair.h"

#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <gmock/gmock.h>

/*
 * Mock.
 */
class MockConnectionProcessCorrelator : public IConnectionProcessCorrelator {
public:
    MOCK_CONST_METHOD2(correlate, bool(QHash<IpEndpointPair, QList<OsProcess> >& result, QString& error));
};

#endif /* MOCKCONNECTIONPROCESSCORRELATOR_H_ */
