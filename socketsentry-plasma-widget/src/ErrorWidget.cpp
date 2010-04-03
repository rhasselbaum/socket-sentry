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

#include "ErrorWidget.h"

#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsItem>
#include <QtGui/QLabel>

ErrorWidget::ErrorWidget(QGraphicsItem* parent) :
    QGraphicsWidget(parent), _messageLabel(NULL), _configureButton(NULL) {

    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Horizontal);
    layout->setSpacing(5);
    _messageLabel = new Plasma::Label();
    _messageLabel->setAlignment(Qt::AlignCenter);
    _messageLabel->nativeWidget()->setWordWrap(true);
    // Make label use all available space. Without this, it won't allocate enough space if the text size increases.
    // Size policy doesn't seem to help, nor does invalidating the layout. Bug?
    _messageLabel->setPreferredSize(65535, 65535);
    layout->addItem(_messageLabel);
    _configureButton = new Plasma::PushButton();
    _configureButton->setText(i18n("Configure..."));
    _configureButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addItem(_configureButton);
    layout->setAlignment(_configureButton, Qt::AlignCenter);
    layout->addStretch(1);

    setLayout(layout);
    connect(_configureButton, SIGNAL(clicked()), this, SIGNAL(configButtonClicked()));
}

ErrorWidget::~ErrorWidget() {
}
