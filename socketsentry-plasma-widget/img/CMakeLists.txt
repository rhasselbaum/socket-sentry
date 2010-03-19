project (SocketSentryPlasmaIcons)

find_package(KDE4 REQUIRED)
add_definitions (${KDE4_DEFINITIONS})

# Install icons
kde4_install_icons( ${ICON_INSTALL_DIR} )
