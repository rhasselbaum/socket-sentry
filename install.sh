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

# This script builds and installs the software using CMake and GNU Make.
# Call it from within the top-level project directory.
#
# This script was adapted from the Fancy Tasks Plasmoid project by Michael D.
# http://www.kde-look.org/content/show.php?action=content&content=99737.

if [ ! -d build  ]
then
    mkdir build
fi

cd build
cmake ../ -DCMAKE_INSTALL_PREFIX=`kde4-config --prefix`
make

if [ $? != 0 ]
then
    echo
    echo "An error occured during compilation!"
    echo "Read the INSTALL file for help."
    exit
fi

if [ ! `whoami` = "root" ]
then
    echo
    echo "Installation requires root privileges - Ctrl+C to cancel."
    sudo make install

    if [ $? != 0 ]
    then
        exit
    fi
else
    make install
fi

if [ `whoami` = "root" ]
then
    exit
fi

kbuildsycoca4
echo
echo "You MUST restart Plasma before using the applet."
echo "Read the INSTALL file for details. Enjoy!"
