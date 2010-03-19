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
 
install(FILES socksent-plasma-widget.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

# Install icons
add_subdirectory (img)
