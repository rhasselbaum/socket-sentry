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

#ifndef NETWORKDEVICEWIDGET_H_
#define NETWORKDEVICEWIDGET_H_

#include <QtGui/QGraphicsWidget>
#include <QtCore/QString>

class ErrorWidget;
class QGraphicsItem;
class AppletConfiguration;
class QGraphicsLinearLayout;
class CommunicationFlow;
class CommunicationFlowTableView;
class OsProcess;
template <class K, class V> class QHash;
template <class T> class QList;
template <class T> class QSet;

namespace Plasma {
    class Frame;
}

/*
 * The top-level widget that displays traffic on a device. It manages a table of communication flows showing
 * current network activity. In the event of an error, the table is replaced by a label that displays an
 * appropriate error message, which persists until a successful traffic update is received. This widget
 * receives traffic (or error) updates via its public slots, which changes the display accordingly.
 */
class NetworkDeviceWidget : public QGraphicsWidget {
    Q_OBJECT

public:
    NetworkDeviceWidget(QGraphicsItem* parent = 0);
    virtual ~NetworkDeviceWidget();

    // Get the canonical list of full column names available in the communication flow model.
    QStringList getFullColumnNames() const;

signals:
    // Emitted when a new set of flows are available.
    void processFlowUpdate(const QList<CommunicationFlow>& flows);

    // Emitted when the applet configuration changes after the change has been processed by this widget.
    void configurationChanged(const AppletConfiguration& newConfig);

    // Emitted whenever the model and view must save their state.
    void configurationSaveRequested(AppletConfiguration& config);

    // Emitted when the user has requested the configuration interface.
    void configurationInterfaceRequested();

public slots:
    // Updates this widget and it's children to reflect configuration changes.
    void readConfiguration(const AppletConfiguration& newConfig);

    // Update the display of flow groups to reflect a new list of flows if the device name is equal to this
    // widget's device.
    void deviceUpdated(const QString& deviceName, const QList<CommunicationFlow>& allFlows);

    // Replace the current display of flow groups (if any) with an error label if the device name is equal to
    // this widget's device.
    void deviceFailed(const QString& deviceName, const QString& error);

private:
    // Set the title text. If the device is the default pseudo-device that captures traffic on all interfaces
    // as indicated by the argument, then the device name is excluded from the title. Else, the title mentions
    // the current device by name.
    void updateTitle(bool defaultDeviceSelected);

    // Switch between displaying the flow view and the error label.
    void swapMainWidgets();

    // The title frame.
    Plasma::Frame* _titleFrame;

    // Index of the main widget (either the flow view or the error label) in the layout.
    int _mainWidgetIndex;

    // The flow view table.
    CommunicationFlowTableView* _flowView;

    // Device name for display or empty if unspecified.
    QString _deviceName;

    // Error widget shown in case of a failure.
    ErrorWidget* _errorWidget;

    // Layout of this widget.
    QGraphicsLinearLayout* _topmostLayout;
};

#endif /* NETWORKDEVICEWIDGET_H_ */
