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

#ifndef ERRORWIDGET_H_
#define ERRORWIDGET_H_

#include <QtGui/QGraphicsWidget>
#include <QtGui/QGraphicsWidget>
#include <Plasma/Label>
#include <Plasma/PushButton>
#include <QtGui/QGraphicsLayout>

class QGraphicsItem;
class QEvent;
class QGraphicsLinearLayout;

/*
 * Dispalys a failure and optionally displays a configure button to configure the applet.
 */
class ErrorWidget : public QGraphicsWidget {
    Q_OBJECT

public:
    ErrorWidget(QGraphicsItem* parent = 0);
    virtual ~ErrorWidget();

    // The text of the error.
    void setText(const QString& message) { _messageLabel->setText(message); }
    QString getText() const { return _messageLabel->text(); }

signals:
    // Emitted when the configuration button is clicked.
    void configButtonClicked();

private:
    // Displays the error message.
    Plasma::Label* _messageLabel;

    // Configure button, which is shown by default.
    Plasma::PushButton* _configureButton;

};

#endif /* ERRORWIDGET_H_ */
