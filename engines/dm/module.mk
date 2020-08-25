# ScummVM - Graphic Adventure Engine
#
# ScummVM is the legal property of its developers, whose names
# are too numerous to list here. Please refer to the COPYRIGHT
# file distributed with this source distribution.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
#
#
#
# Based on the Reverse Engineering work of Christophe Fontanel,
# maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
#


MODULE := engines/dm

MODULE_OBJS := \
	champion.o \
	console.o \
	dialog.o \
	dm.o \
	dmglobals.o \
	dungeonman.o \
	eventman.o \
	gfx.o \
	group.o \
	inventory.o \
	loadsave.o \
	lzw.o \
	menus.o \
	metaengine.o \
	movesens.o \
	objectman.o \
	projexpl.o \
	sounds.o \
	text.o \
	timeline.o

MODULE_DIRS += \
	engines/dm

# This module can be built as a plugin
ifeq ($(ENABLE_DM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
