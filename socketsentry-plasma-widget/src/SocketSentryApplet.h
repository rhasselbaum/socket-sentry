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

#ifndef SOCKETSENTRYAPPLET_H_
#define SOCKETSENTRYAPPLET_H_

#include "AppletConfiguration.h"
#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>

class NetworkDeviceWidget;
class QString;
class KConfigDialog;
class LocalSettingsUi;
class GlobalSettingsUi;

/*
 * The Socket Sentry applet.
 */
class SocketSentryApplet : public Plasma::PopupApplet {
    Q_OBJECT

public:
    SocketSentryApplet(QObject* parent, const QVariantList& args);
    virtual ~SocketSentryApplet();

    // Initialize the applet.
    virtual void init();

    // Create the configuration settings UI and add it to the parent dialog.
    virtual void createConfigurationInterface(KConfigDialog* parent);

signals:
    // Emitted whenever the applet configuration changes.
    void appletConfigurationChanged(const AppletConfiguration& newConfig);

    // Emitted whenever the applet needs its widgets to save their state.
    void configurationSaveRequested(AppletConfiguration& config);

protected:
    // Save the state of this applet. Emits "configurationSaveRequested" to give its children a chance to write their
    // configuration to the shared applet configuration object. Then changes are written to the local and global
    // config groups.
    virtual void saveState(KConfigGroup& localConfig) const;

    // Disconnect from source when hidden, reconnect when shown.
    virtual void popupEvent(bool popped);

    // If the form factor changes, disconnect or reconnect to the configured source depending on whether or not
    // the applet is iconified.
    virtual void constraintsEvent(Plasma::Constraints constraints);

private slots:
    // Receives updates from the engine.
    void dataUpdated(const QString& sourceName, const Plasma::DataEngine::Data& data);

    // Transfers current configuration from the engine to the applet configuration object.
    void importEngineConfiguration();

    // Transfers configuration from the applet configuration object to the engine and change the currently connected
    // source if the applet configuration specifies a new one.
    void exportEngineConfiguration();

    // Transfer new configuration settings from the configuration UI to the shared applet config object and emit
    // the appletConfigurationChanged signal to all listeners.
    void configurationAccepted();

    // Remove pointers to configuration interface objects. They are no longer valid.
    void removeConfigurationInterface();

private:
    // Returns true if the applet is either open on the desktop or popped up if it is contained on a panel.
    bool isDisplayed() const;

    // Disconnect from or connect to the configured data engine source depending on whether or not the applet is
    // currently visible (not iconified). Disconnecting from the data engine when the applet is not visible
    // reduces our resource overhead. If the connection state of the source is already correct for the current
    // visibility, no changes are made. The caller specifies the current visibility in the argument, which it
    // can obtain through a call to isDisplayed() if it doesn't know any better.
    void updateSourceConnection(bool visible);

    // The main widget.
    NetworkDeviceWidget* _mainWidget;

    // The data engine.
    Plasma::DataEngine* _dataEngine;

    // The applet configuration.
    AppletConfiguration _appletConfig;

    // The currently connected source (device) name or empty if no source is connected.
    QString _connectedSourceName;

    // Applet configuration pages. These objects are owned by the KConfigDialog supplied by Plasma, but we keep pointers
    // to them so we can extract new values after the user completes the configuration.
    LocalSettingsUi* _localSettings;
    GlobalSettingsUi* _globalSettings;

    // Data engine properties.
    static const char* OS_PROCESS_SORT_ASCENDING_PROPERTY;
    static const char* RESOLVE_NAMES_PROPERTY;
    static const char* CUSTOM_FILTER_PROPERTY;
};

#endif /* SOCKETSENTRYAPPLET_H_ */
