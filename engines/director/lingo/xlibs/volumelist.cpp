/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*************************************
 *
 * USED IN:
 * Yearn2Learn: The Flintstones Coloring Book
 *
 *************************************/

/*
 * v1.0, ©1989, 1990 Eric Carlson, Apple Computer, Inc.
 * VolumeList(«“nodialog:ErrMsg”»)
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/volumelist.h"


namespace Director {

const char *VolumeList::xlibName = "VolumeList";
const char *VolumeList::fileNames[] = {
	"VolumeList",
	0
};

static BuiltinProto builtins[] = {
	{ "VolumeList", VolumeList::m_volumelist, 0, 0, 300, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void VolumeList::open(int type) {
	g_lingo->initBuiltIns(builtins);
}

void VolumeList::close(int type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void VolumeList::m_volumelist(int nargs) {
	// Would presumably give a list of volumes attached,
	// with the first being the boot disk
	g_lingo->push(Datum(""));
}

} // End of namespace Director
