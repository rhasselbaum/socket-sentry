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

#include "GlobalSettingsUi.h"
#include "AppletConfiguration.h"

GlobalSettingsUi::GlobalSettingsUi(QWidget* parent) :
    QWidget(parent) {
    _ui.setupUi(this);
}

GlobalSettingsUi::~GlobalSettingsUi() {
}

void GlobalSettingsUi::readConfiguration(const AppletConfiguration& config) {
    _ui.hostNameLookupCheckBox->setChecked(config.getResolveNames());
    if (config.getOsProcessSortAscending()) {
        _ui.oldestProcessRadioButton->setChecked(true);
    } else {
        _ui.newestProcessRadioButton->setChecked(true);
    }
    _ui.filterTextEdit->setPlainText(config.getCustomFilter());
}

void GlobalSettingsUi::writeConfiguration(AppletConfiguration& config) {
    config.setResolveNames(_ui.hostNameLookupCheckBox->isChecked());
    config.setOsProcessSortAscending(_ui.oldestProcessRadioButton->isChecked());
    config.setCustomFilter(_ui.filterTextEdit->toPlainText());
}
