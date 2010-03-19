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

#ifndef COMMUNICATIONFLOWITEM_H_
#define COMMUNICATIONFLOWITEM_H_

#include "CommunicationFlowItemData.h"

#include <KDE/KIcon>
#include <QtCore/QSharedDataPointer>

/*
 * A container for a communication flow object that provides hints for displaying the flow in a view. It is
 * intended to be used in conjunction with a model such as a FlowGroupItemModel.
 *
 * Flow items may be combined with other flow items representing different connections thst share some common
 * attributes. When flow items are combined, the underlying communication flow objects that they wrap are also
 * combined. That is, their metrics and statistics are aggregated. To combine other flow items into this one,
 * call the "combineConnections" method. Additionally, flow items can be reset and reused to describe the same
 * (or a similar) group of connections over a new time period. To assign a new flow to this flow item, call its
 * "update" method.
 *
 * Once you have assigned a communication flow object to this flow item and aggregated as many additional items
 * into this one as needed, you must call "prepare" before making the item available to the view. This causes
 * the item to initiaze the program icon, device state icon, font, etc. based on the current state of its
 * communication flow. This should only be done after aggregation, since it is relatively expensive.
 */
class CommunicationFlowItem {
public:
    // Create a new instance that wraps the given communication flow and supplies the base font for the item.
    explicit CommunicationFlowItem(const CommunicationFlow& actual);
    virtual ~CommunicationFlowItem();

    // Possible network device states. These are ordered by "most interesting" to "least interesting", which is
    // relevant when sorting the model.
    enum DeviceState {
        SendingAndReceiving = 0,
        Receiving,
        Sending,
        Quiet
    };

    // Replace the wrapped communication flow with a new one. This method does not update the "first seen"
    // time stamp, implying that the new flow represents the same connection (or one from the same group
    // of connections) at a later point in time. You must call "prepare" after calling this method before
    // exposing the item to a view.
    void update(const CommunicationFlow& actual);

    // Turn this item into a zombie, which zeroes out all of its stats. A zombie is a communication flow
    // item in which the underlying connections have closed, but the item needs to persist in the view so
    // a human gets a chance to see it. Zombies can be returned to living state by calling "update" with
    // a new flow. You must call "prepare" after turning the item into a zombie to update dependent
    // properties.
    void zombify();

    // Initiate this item's derived properties such as program icon, device state and icon, and font
    // based on the current flow values. This method should be called before making the item available
    // to the view and after any changes.
    void prepare();

    // Combine one or more connections from another item into this item. The metrics and statistics of
    // the other item are merged into this item. After you have finished aggregating connections, you
    // must call "prepare" before exposing the item to a view.
    void combineConnections(const CommunicationFlowItem& other);

    // Return the actual flow wrapped by this item.
    CommunicationFlow actual() const { return _d->actual; }

    // Return the number of connections aggregated by this flow item.
    int getNumConnections() const { return _d->numConnections; }

    // The date and time when this flow was first seen by the model.
    QDateTime getFirstSeenUtc() const { return _d->firstSeenUtc; }
    void setFirstSeenUtc(const QDateTime& firstSeenUtc) { _d->firstSeenUtc = firstSeenUtc; }

    // Get the font that should be used to display this item. Font weight is proportional to the transfer
    // rate. If the rate is 0, the font is light. Else, if the rate is at least 75% of the peak rate, the
    // font is bold. Else, the font is normal. All other attributes of the font are identical to the base font
    // supplied to this object when it was instantiated.
    QFont getFont() const { return _d->font; }

    // Get an icon representing the current device state of the flow (seding, receiving, sending and receiving,
    // or quiet).
    KIcon getDeviceStateIcon() const { return *(_d->deviceStateIcon); }

    // Get the network device state.
    DeviceState getDeviceState() const { return (DeviceState)_d->deviceState; }

    // Get an icon representing the program if possible. If no suitable icon is found, a null icon is returned.
    KIcon getProgramIcon() const { return _d->programIcon; }

    // Returns true if this item is a zombie. See "zombify".
    bool isZombie() const { return _d->zombie; }

private:
    // Initialize the device state of the flow and find a matching icon.
    void initDeviceStateAndIcon();

    // Find the program icon if available.
    void initProgramIcon();

    // Set font to bold if transfer rate is >75% of peak. Set to light if it's 0. Set to normal otherwise. The base
    // font comes from the current theme's default font.
    void initFont();

    // Shared data.
    QSharedDataPointer<CommunicationFlowItemData> _d;

    // Icons to represent the four possible network device states.
    static const KIcon SendingIcon;
    static const KIcon ReceivingIcon;
    static const KIcon SendingAndReceivingIcon;
    static const KIcon QuietIcon;
};

#endif /* COMMUNICATIONFLOWITEM_H_ */
