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

#include "SocketSentryApplet.h"
#include "NetworkDeviceWidget.h"
#include "CommunicationFlow.h"
#include "LocalSettingsUi.h"
#include "GlobalSettingsUi.h"

#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QVariant>
#include <KDE/KConfigDialog>
#include <Plasma/DataEngine>

// Data engine properties.
const char* SocketSentryApplet::OS_PROCESS_SORT_ASCENDING_PROPERTY = "osProcessSortAscending";
const char* SocketSentryApplet::RESOLVE_NAMES_PROPERTY = "resolveNames";
const char* SocketSentryApplet::CUSTOM_FILTER_PROPERTY = "customFilter";


SocketSentryApplet::SocketSentryApplet(QObject* parent, const QVariantList& args) :
    Plasma::PopupApplet(parent, args), _mainWidget(NULL), _dataEngine(NULL), _localSettings(NULL), _globalSettings(NULL) {

}

SocketSentryApplet::~SocketSentryApplet() {
    if (!hasFailedToLaunch()) {
        emit configurationSaveRequested(_appletConfig);
        KConfigGroup localConfig = config();
        saveState(localConfig);
        if (!_connectedSourceName.isEmpty()) {
            _dataEngine->disconnectSource(_connectedSourceName, this);
        }
    }
}

bool SocketSentryApplet::isDisplayed() const {
    // As of KDE 4.4, Plasma doesn't have a convenient API for this, so we take a guess based on two assumptions:
    // 1) A form factor of horizontal or veritcal means we're contained in a panel.
    // 2) A panel is smaller than our minimum height or width, so we're iconified in it.
    // Discussed other approaches in the Plasma mailing list, but this'll have to suffice for now.
    if (formFactor() == Plasma::Horizontal || formFactor() == Plasma::Vertical) {
        // Applet is a popup icon on a panel. We're visible only if the popup is showing.
        return isPopupShowing();
    } else {
        // Applet is always open (e.g. on the desktop).
        return true;
    }
}

void SocketSentryApplet::init() {
    Q_ASSERT(!_mainWidget);
    Q_ASSERT(!_dataEngine);
    setHasConfigurationInterface(true);
    _dataEngine = dataEngine("socketsentry");
    if (!_dataEngine || !_dataEngine->isValid()) {
        // Data engine is missing or invalid.
        setFailedToLaunch(true, i18n("Sorry, the data engine couldn't be initialized. Try restarting KDE."));
    } else {
        // Try to get the engine's list of device sources.
        _appletConfig.importConfig(config(), globalConfig());
        QStringList devices = _dataEngine->sources();
        if (devices.isEmpty()) {
            // Can't continue. No devices found.
            QHash<QString, QVariant> engineStatus =  _dataEngine->query("status");
            if (engineStatus.contains("error")) {
                QString engineError = engineStatus["error"].value<QString>();
                QString msg = i18n("<p>Sorry, the data engine reported an error: \"%1\"</p>").arg(engineError);
                msg += i18n("<p>If you just installed this widget, please try logging out and logging back in.</p>");
                setFailedToLaunch(true, msg);
            } else {
                setFailedToLaunch(true, i18n("The data engine did not report an error, but no network devices are found."));
            }
        } else {
            // Set up the applet.
            setBackgroundHints(DefaultBackground);
            setAspectRatioMode(Plasma::IgnoreAspectRatio);
            setPopupIcon("socketsentry");

            // Initialize the main widget and send it the current configuration.
            _mainWidget = new NetworkDeviceWidget(this);
            _mainWidget->setPreferredSize(400, 500);
            setGraphicsWidget(_mainWidget);
            connect(this, SIGNAL(appletConfigurationChanged(const AppletConfiguration&)),
                    _mainWidget, SLOT(readConfiguration(const AppletConfiguration&)));
            connect(this, SIGNAL(appletConfigurationChanged(const AppletConfiguration&)),
                    this, SLOT(exportEngineConfiguration()));
            connect(this, SIGNAL(configurationSaveRequested(AppletConfiguration&)),
                    _mainWidget, SIGNAL(configurationSaveRequested(AppletConfiguration&)));
            connect(this, SIGNAL(configurationSaveRequested(AppletConfiguration&)),
                    this, SLOT(importEngineConfiguration()));
            emit appletConfigurationChanged(_appletConfig);
        }
    }
}

void SocketSentryApplet::popupEvent(bool popped) {
    Plasma::PopupApplet::popupEvent(popped);
    updateSourceConnection(popped);
}

void SocketSentryApplet::constraintsEvent(Plasma::Constraints constraints) {
   Plasma::PopupApplet::constraintsEvent(constraints);
   if (constraints & Plasma::FormFactorConstraint) {
        updateSourceConnection(isDisplayed());
    }
}

void SocketSentryApplet::updateSourceConnection(bool visible) {
    if (!visible && !_connectedSourceName.isEmpty()) {
        // Transitioning from displayed to hidden. Disconnect source.
        _dataEngine->disconnectSource(_connectedSourceName, this);
        _connectedSourceName = "";
    } else if (visible && _connectedSourceName.isEmpty()) {
        // Transitioning from hidden to displayed. Connect source.
        _dataEngine->connectSource(_appletConfig.getSelectedDevice(), this, 0, Plasma::NoAlignment);
        _connectedSourceName = _appletConfig.getSelectedDevice();
    }
}

void SocketSentryApplet::exportEngineConfiguration() {
    // If device name is changing from a non-empty one, disconnect from the old source.
    if (!_connectedSourceName.isEmpty() && _connectedSourceName != _appletConfig.getSelectedDevice()) {
        _dataEngine->disconnectSource(_connectedSourceName, this);
        _connectedSourceName = "";
    }
    // Set properties.
    _dataEngine->setProperty(OS_PROCESS_SORT_ASCENDING_PROPERTY, _appletConfig.getOsProcessSortAscending());
    _dataEngine->setProperty(RESOLVE_NAMES_PROPERTY, _appletConfig.getResolveNames());
    _dataEngine->setProperty(CUSTOM_FILTER_PROPERTY, _appletConfig.getCustomFilter());
    // If device name is changing, connect to new source.
    if (isDisplayed() && _connectedSourceName != _appletConfig.getSelectedDevice()) {
        _dataEngine->connectSource(_appletConfig.getSelectedDevice(), this, 0, Plasma::NoAlignment);
        _connectedSourceName = _appletConfig.getSelectedDevice();
    }
}

void SocketSentryApplet::importEngineConfiguration() {
    Q_ASSERT(_dataEngine);
    QVariant osProcessSortAscending = _dataEngine->property(OS_PROCESS_SORT_ASCENDING_PROPERTY);
    Q_ASSERT(osProcessSortAscending.isValid());
    _appletConfig.setOsProcessSortAscending(osProcessSortAscending.value<bool>());

    QVariant resolveNames = _dataEngine->property(RESOLVE_NAMES_PROPERTY);
    Q_ASSERT(resolveNames.isValid());
    _appletConfig.setResolveNames(resolveNames.value<bool>());

    QVariant customFilter = _dataEngine->property(CUSTOM_FILTER_PROPERTY);
    Q_ASSERT(customFilter.isValid());
    _appletConfig.setCustomFilter(customFilter.toString());
}

void SocketSentryApplet::createConfigurationInterface(KConfigDialog* parent) {
    // First, synchronize any configuration changes made by other parts of the UI with the applet configuration.
    emit configurationSaveRequested(_appletConfig);
    KConfigGroup localConfig = config();
    saveState(localConfig);

    // Now generate the config UI.
    _localSettings = new LocalSettingsUi(_dataEngine->sources(), _mainWidget->getFullColumnNames(), parent);
    _localSettings->readConfiguration(_appletConfig);
    parent->addPage(_localSettings, i18n("Local Settings"), "socketsentry");

    _globalSettings = new GlobalSettingsUi(parent);
    _globalSettings->readConfiguration(_appletConfig);
    parent->addPage(_globalSettings, i18n("Global Settings"), "preferences-system-network");

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configurationAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configurationAccepted()));
    connect(parent, SIGNAL(destroyed()), this, SLOT(removeConfigurationInterface()));
}

void SocketSentryApplet::configurationAccepted() {
    Q_ASSERT(_localSettings);
    _localSettings->writeConfiguration(_appletConfig);
    Q_ASSERT(_globalSettings);
    _globalSettings->writeConfiguration(_appletConfig);
    emit appletConfigurationChanged(_appletConfig);
    KConfigGroup localConfig = config();
    saveState(localConfig);
}

void SocketSentryApplet::removeConfigurationInterface() {
    // Don't delete because we don't own them.
    _localSettings = NULL;
    _globalSettings = NULL;
}

void SocketSentryApplet::saveState(KConfigGroup& localConfig) const {
    KConfigGroup global = globalConfig();
    _appletConfig.exportConfig(localConfig, global);
}

void SocketSentryApplet::dataUpdated(const QString& sourceName, const Plasma::DataEngine::Data& data) {
    if (sourceName == _appletConfig.getSelectedDevice()) {
        // Let the main widget know about the device update or failure.
        if (data.contains("data")) {
            QList<CommunicationFlow> flows = data["data"].value<QList<CommunicationFlow> >();
            _mainWidget->deviceUpdated(sourceName, flows);
        } else if (data.contains("error")) {
            _mainWidget->deviceFailed(sourceName, data["error"].value<QString>());
        }
    }
}

K_EXPORT_PLASMA_APPLET(socketsentry, SocketSentryApplet)
