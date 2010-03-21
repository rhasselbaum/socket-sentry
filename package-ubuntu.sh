#!/bin/bash

# Generate (K)ubuntu DEB packages for the current architecture.
# This probably won't work unless you're running Ubuntu.
#
# If you have trouble running this script after a normal build,
# try clearing out the build directory first. 

# Create an Ubuntu package.
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

if [ ! -d build  ]
then
    mkdir build
fi

cd build

create_package 'karmic1'
create_package 'lucid1'
