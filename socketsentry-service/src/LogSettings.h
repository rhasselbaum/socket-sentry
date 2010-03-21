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

#ifndef LOGSETTINGS_H_
#define LOGSETTINGS_H_

/*
 * A simple container of service log settings. This singleton is NOT thread-safe and should only be accessed directly
 * from the main thread.
 */
class LogSettings {
public:
    LogSettings(bool logProcessCorrelation, bool logPacketCapture);
    virtual ~LogSettings();

    // True if the service should log connection-process correlation stats.
    bool logProcessCorrelation() const { return _logProcessCorrelation; }

    // True if the service should log packet capture stats.
    bool logPacketCapture() const { return _logPacketCapture; }

    // Get the singleton instance.
    static const LogSettings& getInstance() { return INSTANCE; }

    // Set the singleton instance.
    static void setInstance(const LogSettings& instance) { INSTANCE = instance; }

private:
    bool _logProcessCorrelation;
    bool _logPacketCapture;

    // Singleton instance.
    static LogSettings INSTANCE;
};

#endif /* LOGSETTINGS_H_ */
