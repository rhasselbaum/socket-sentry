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

include (InstallRequiredSystemLibraries)

# Common
set (CPACK_SET_DESTDIR "ON")
set (CPACK_PACKAGE_NAME "socketsentry")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Network traffic monitor for KDE Plasma")
set (CPACK_PACKAGE_VENDOR "Socket Sentry Developers")
set (CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "0")
set (CPACK_PACKAGE_VERSION_MINOR "9")
set (CPACK_PACKAGE_VERSION_PATCH "0")
set (CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

# Ubuntu
set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Rob Hasselbaum <rob@hasselbaum.net>")
set (CPACK_DEBIAN_PACKAGE_SECTION "KDE")
set (CPACK_DEBIAN_PACKAGE_VERSION  "${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_VERSION_SUFFIX}")
set (CPACK_DEBIAN_PACKAGE_DEPENDS "kdebase-runtime (>= 4:4.3.2), kdebase-workspace-libs4+5 (>= 4:4.3.2), kdelibs5 (>= 4:4.3.2), libqt4-dbus (>= 4.5.1), libqt4-network (>= 4.5.1), libqt4-xml (>= 4.5.1), libstdc++6 (>= 4.1.1), libqtcore4 (>= 4.5.1), libqtgui4 (>= 4.5.1), kdebase-workspace-bin, libpcap0.8")
set (CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/debian/prerm")

# Conditionals
if ((DEFINED CPACK_GENERATOR) AND (CPACK_GENERATOR STREQUAL "DEB"))
	set (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_DEBIAN_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
else ((DEFINED CPACK_GENERATOR) AND (CPACK_GENERATOR STREQUAL "DEB"))
	set (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
endif ((DEFINED CPACK_GENERATOR) AND (CPACK_GENERATOR STREQUAL "DEB"))
 
include (CPack)
 