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

#ifndef LATCH_H_
#define LATCH_H_

#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

/*
 * A simple latch that supports blocking one or more threads until another thread has passed a critical point
 * in its execution. After a latch is created, threads may call "await" to block until the latch is flipped.
 * When another thread calls "flip", all waiting threads will wake up and resume execution. If the latch is
 * already flipped when a thread calls "await", the call returns immediately. Calling "flip" more than once has
 * no effect.
 */
class Latch {
public:
    Latch();
    virtual ~Latch();

    // Block until the latch is flipped. If the latch is already flipped, return immediatrely.
    void wait();

    // Flip the latch, resuming any waiting threads. Calling this more than once has no effect.
    void flip();

private:
    bool _flipped;
    QMutex _mutex;
    QWaitCondition _flippedCondition;
};

#endif /* LATCH_H_ */
