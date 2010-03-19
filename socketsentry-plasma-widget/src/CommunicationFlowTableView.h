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

#ifndef COMMUNICATIONFLOWTABLEVIEW_H_
#define COMMUNICATIONFLOWTABLEVIEW_H_

#include <Plasma/TreeView>

class QgraphicsWidget;
class AppletConfiguration;
class QModelIndex;
class QAbstractItemModel;
class QMenu;
class QContextMenuEvent;
class QPoint;

/*
 * A table that displays current communication flows on a network device.
 */
class CommunicationFlowTableView : public Plasma::TreeView {
    Q_OBJECT

public:
    CommunicationFlowTableView(QGraphicsWidget* parent = 0);
    virtual ~CommunicationFlowTableView();

    // Set the model used by this view. Also, initializes the context menu based on headers.
    void setModel(QAbstractItemModel* model);

public slots:
    // Updates this widget to reflect configuration changes.
    void readConfiguration(const AppletConfiguration& newConfig);

    // When invoked, the widget will save its state including current sort order, column widths, etc.
    void writeConfiguration(AppletConfiguration& config);

    // Show the context menu allowing user to show/hide columns.
    void showContextMenu(const QPoint& pos);

private slots:
    // Initialize the tree view color palette from the plasma theme.
    void updateStyle();

    // Updates the view after a completed model update. This initializes column widths and hidden state to
    // defaults if they haven't already been set by configuration.
    void tick();

private:
    // The context menu.
    QMenu* _contextMenu;

    // True if columns have been sized as a result of explicit configuration or default sizes having been applied.
    bool _columnsSized;

    // True if column hiding has been applied as a result of explicit configuration or default policy.
    bool _appliedColumnHiding;

};

#endif /* COMMUNICATIONFLOWTABLEVIEW_H_ */
