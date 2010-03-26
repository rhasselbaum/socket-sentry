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

# This script creates the pristine source tarball and spec, dsc, and other
# packaging files for upload to the openSUSE Build Service for building and
# package generation. Run it from the top-level project directory. Output
# is placed into "staging" subdirectory, overwriting what was there.

if [ ! -d staging  ]
then
	mkdir staging
fi

# Check exit code passed as a parameter and display a message if it's not 0 (success).
# $1: The exit code value.
# $2: Message to display if it's non-zero.
check_errs()
{
  if [ "${1}" -ne "0" ]; then
    echo "ERROR # ${1} : ${2}"
    exit ${1}
  fi
}


VERSION=`cat VERSION`
BASE_NAME=socketsentry-$VERSION
echo Creating $BASE_NAME.tar.gz.
tar cvfz staging/$BASE_NAME.tar.gz \
	--exclude './.*' --exclude 'staging/**' --exclude 'build/**' \
	--transform s\\^\\$BASE_NAME/\\g * >/dev/null
check_errs $? "Can't create archive $BASE_NAME"

echo "Creating RPM spec."
sed <obs/socketsentry_basic.spec s/@VERSION@/$VERSION/g >staging/$BASE_NAME.spec
check_errs $? "Can't generate spec file."
