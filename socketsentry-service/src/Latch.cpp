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

#include "Latch.h"

#include <QtCore/QMutexLocker>

Latch::Latch() :
    _flipped(false) {
}

Latch::~Latch() {
}

void Latch::wait() {
    QMutexLocker locker(&_mutex);
    while (!_flipped) {
        _flippedCondition.wait(&_mutex);
    }
}

void Latch::flip() {
    QMutexLocker locker(&_mutex);
    _flipped = true;
    _flippedCondition.wakeAll();
}
