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

#include "CommunicationFlowItem.h"
#include "FlowStatistics.h"

#include <QtCore/QString>
#include <KDE/KLocalizedString>
#include <KDE/KIconLoader>
#include <Plasma/Theme>

// Icons to represent the four possible network device states.
const KIcon CommunicationFlowItem::SendingIcon = KIcon("socketsentry_sending");
const KIcon CommunicationFlowItem::ReceivingIcon = KIcon("socketsentry_receiving");
const KIcon CommunicationFlowItem::SendingAndReceivingIcon = KIcon("socketsentry_sendingreceiving");
const KIcon CommunicationFlowItem::QuietIcon = KIcon("socketsentry_quiet");

CommunicationFlowItem::CommunicationFlowItem(const CommunicationFlow& actual) {
    _d = new CommunicationFlowItemData();
    update(actual);
}

CommunicationFlowItem::~CommunicationFlowItem() {
}

void CommunicationFlowItem::update(const CommunicationFlow& actual) {
    _d->actual = actual;
    _d->numConnections = 1;
    _d->zombie = false;
}

void CommunicationFlowItem::prepare() {
    initFont();
    initDeviceStateAndIcon();
    initProgramIcon();
}

void CommunicationFlowItem::initFont() {
    _d->font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    const FlowStatistics& stats = _d->actual.getFlowStatistics();
    qlonglong bps = stats.getRecentBytesPerSec();
    if (bps == 0) {
        _d->font.setWeight(QFont::Light);
    } else {
        qlonglong highAverage = stats.getPeakBytesPerSec() * 75 / 100;  // 75% of peak is a "high" average
        if (bps > highAverage) {
            _d->font.setWeight(QFont::Bold);
        } else {
            _d->font.setWeight(QFont::Normal);
        }
    }
}

void CommunicationFlowItem::zombify() {
    FlowStatistics stats = _d->actual.getFlowStatistics();
    stats.quietDevice();
    _d->actual.setFlowStatistics(stats);
    _d->zombie = true;
}

void CommunicationFlowItem::combineConnections(const CommunicationFlowItem& other) {
    // Merge values.
    _d->actual.combineConnections(other.actual());
    _d->numConnections += other.getNumConnections();
    if (other.getFirstSeenUtc() < _d->firstSeenUtc) {
        _d->firstSeenUtc = other.getFirstSeenUtc();
    }
    if (!other.isZombie()) {
        _d->zombie = false;
    }
}

void CommunicationFlowItem::initProgramIcon() {
#if QT_VERSION >= 0x040600
    _d->programIcon = KIcon(QIcon::fromTheme(_d->actual.getFirstOsProcess().getProgram(), QIcon()));
#else
    KIconLoader* iconLoader = KIconLoader::global();
    QString iconPath = iconLoader->iconPath(_d->actual.getFirstOsProcess().getProgram(), KIconLoader::Desktop, true);
    if (!iconPath.isNull()) {
        _d->programIcon = KIcon(_d->actual.getFirstOsProcess().getProgram());
    } else {
        _d->programIcon = KIcon();
    }
#endif
}

void CommunicationFlowItem::initDeviceStateAndIcon() {
    const FlowStatistics& stats = _d->actual.getFlowStatistics();
    if (stats.isSendingNow() && stats.isReceivingNow()) {
        _d->deviceState = (int)SendingAndReceiving;
        _d->deviceStateIcon = &SendingAndReceivingIcon;
    } else if (stats.isSendingNow()) {
        _d->deviceState = (int)Sending;
        _d->deviceStateIcon = &SendingIcon;
    } else if (stats.isReceivingNow()) {
        _d->deviceState = (int)Receiving;
        _d->deviceStateIcon = &ReceivingIcon;
    } else {
        _d->deviceState = (int)Quiet;
        _d->deviceStateIcon = &QuietIcon;
    }
}
