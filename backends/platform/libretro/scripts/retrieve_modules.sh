#!/bin/bash

 # Copyright (C) 2022 Giovanni Cascione <ing.cascione@gmail.com>
 #
 # This program is free software: you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation, either version 3 of the License, or
 # (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program.  If not, see <http://www.gnu.org/licenses/>.

# This script is to extract shared modules definitions from ScummVM
# Makefile.common, in order to be in sync with modifications upstream

# Externally passed variables shall be:
# $1 [REQ] ScummVM Makefile.common full path


if [ -f "$1" ] ; then
	sed -e ':a' -e 'N' -e '$!ba' -e 's/\n/§/g' -e 's|.*-include engines/engines.mk||' -e 's/###.*//' -e 's/§/\n/g' -e 's|MODULES|SHARED_MODULES|g' "$1"
else
	printf "\$(error Error retrieving shared modules definitions from main Makefile.common)"
fi
