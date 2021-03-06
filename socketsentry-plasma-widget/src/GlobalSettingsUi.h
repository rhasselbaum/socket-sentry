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

#ifndef GLOBALSETTINGSUI_H_
#define GLOBALSETTINGSUI_H_

#include <QtGui/QWidget>

#include "ui_GlobalSettings.h"

class AppletConfiguration;

/*
 * Global settings for all widgets.
 */
class GlobalSettingsUi : public QWidget {
    Q_OBJECT

public:
    GlobalSettingsUi(QWidget* parent = 0);
    virtual ~GlobalSettingsUi();

    // Initialize controls from the given applet configuration.
    void readConfiguration(const AppletConfiguration& config);

    // Transfer control values to the applet configuration.
    void writeConfiguration(AppletConfiguration& config);

private:
    Ui::GlobalSettings _ui;
};

#endif /* GLOBALSETTINGSUI_H_ */
