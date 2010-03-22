#   Copyright (C) 2010 by Rob Hasselbaum <rob@hasselbaum.net>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, see <http://www.gnu.org/licenses/>

project (SocketSentryPlasmaWidget)

find_package (KDE4 4.3 REQUIRED)
include (KDE4Defaults)

find_package (Qt4 4.5 REQUIRED)
set (QT_USE_QTNETWORK 1)
set (QT_USE_QTDBUS 1)
include (${QT_USE_FILE})

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${SocketSentryService_SOURCE_DIR}/src ${KDE4_INCLUDES})
 
set (SsPlasmaWidget_SRCS 
	src/SocketSentryApplet.cpp
	src/NetworkDeviceWidget.cpp
	src/CommunicationFlowTableView.cpp
	src/CommunicationFlowTableModel.cpp
	src/CommunicationFlowSortFilterProxyModel.cpp
	src/CommunicationFlowItem.cpp
	src/CommunicationFlowItemData.cpp
	src/CommunicationFlowItemKey.cpp
	src/CommunicationFlowItemKeyData.cpp
	src/AppletConfiguration.cpp
	src/AppletConfigurationData.cpp
	src/LocalSettingsUi.cpp
	src/GlobalSettingsUi.cpp
	src/ColumnListWidgetItem.cpp
)

kde4_add_ui_files (SsPlasmaWidget_SRCS
	src/GlobalSettings.ui
	src/LocalSettings.ui
)

kde4_add_plugin(socksent-plasma-widget ${SsPlasmaWidget_SRCS})
target_link_libraries(socksent-plasma-widget socketsent-client-common ${QT_LIBRARIES} ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
 
install(TARGETS socksent-plasma-widget
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
set (SS_WIDGET_DESKTOP_FILE socksent-plasma-widget.desktop)
configure_file (${SS_WIDGET_DESKTOP_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${SS_WIDGET_DESKTOP_FILE})
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${SS_WIDGET_DESKTOP_FILE} DESTINATION ${SERVICES_INSTALL_DIR})

# Install icons
add_subdirectory (img)
