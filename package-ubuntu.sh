#!/bin/bash
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

# This script generates (K)ubuntu DEB packages for the current architecture.
# Call it from the top level project directory. This script probably won't
# work unless you're running Kubuntu.
#
# If you have trouble running this script after a normal build,
# try clearing out the build directory first. 

# This function creates a default Ubuntu package for some version of Ubuntu.
# ARGS:
# $1: The Debian version (e.g. "karmic1", "lucid1", etc.)
create_package()
{
	DEBIAN_VERSION=$1

	cmake ../ -DCMAKE_INSTALL_PREFIX=`kde4-config --prefix` \
		-DCPACK_DEBIAN_PACKAGE_ARCHITECTURE=`dpkg --print-architecture` \
		-DCPACK_DEBIAN_PACKAGE_VERSION_SUFFIX=$DEBIAN_VERSION \
		-DCPACK_GENERATOR=DEB 
	make
	
	if [ $? != 0 ]
	then
	    echo
	    echo "An error occured during compilation!"
	    echo "Read the INSTALL file for help."
	    exit
	fi
	
	cpack
}

# Main script starts here. Make the build dir.
if [ ! -d build  ]
then
    mkdir build
fi

cd build

# Create Ubuntu packages.
create_package 'karmic1'
create_package 'lucid1'
