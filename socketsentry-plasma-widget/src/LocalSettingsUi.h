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

#ifndef WIDGETSETTINGSUI_H_
#define WIDGETSETTINGSUI_H_

#include <QtGui/QWidget>
#include <QtCore/QStringList>

#include "ui_LocalSettings.h"

class AppletConfiguration;

/*
 * Settings for the local widget instance.
 */
class LocalSettingsUi : public QWidget {
    Q_OBJECT

public:
    LocalSettingsUi(const QStringList& devices, const QStringList& columns, QWidget* parent = 0);
    virtual ~LocalSettingsUi();

    // Initialize controls from the given applet configuration and list of available network devices and table columns.
    void readConfiguration(const AppletConfiguration& config);

    // Transfer control values to the given applet configuration object.
    void writeConfiguration(AppletConfiguration& config);

private:
    // The UI.
    Ui::LocalSettings _ui;

    // List of available columns.
    QStringList _columns;

    // List of available devices.
    QStringList _devices;
};

#endif /* WIDGETSETTINGSUI_H_ */
