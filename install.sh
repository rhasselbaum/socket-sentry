#!/bin/bash

# Builds and installs the software using CMake and GNU Make. Call it from
# within the top-level project directory.
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

../postinst.sh
