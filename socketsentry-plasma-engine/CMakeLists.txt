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

project (SocketSentryPlasmaEngine)

find_package (KDE4 4.3 REQUIRED)
include (KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${SocketSentryService_SOURCE_DIR}/src ${KDE4_INCLUDES})
 
set (SsPlasmaEngine_SRCS 
	src/SocketSentryDataEngine.cpp
)

# Now make sure all files get to the right place
kde4_add_plugin(socksent-plasma-engine ${SsPlasmaEngine_SRCS})
target_link_libraries(socksent-plasma-engine socketsent-client-common ${KDE4_KDECORE_LIBS} ${KDE4_PLASMA_LIBS})
 
install(TARGETS socksent-plasma-engine
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
set (SS_ENGINE_DESKTOP_FILE socksent-plasma-engine.desktop)
configure_file (${SS_ENGINE_DESKTOP_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${SS_ENGINE_DESKTOP_FILE})
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${SS_ENGINE_DESKTOP_FILE} DESTINATION ${SERVICES_INSTALL_DIR})
 
