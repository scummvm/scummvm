/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "common/system.h"

#include "parallaction/parallaction.h"
#include "parallaction/debug.h"

namespace Parallaction {


Debugger::Debugger(Parallaction *vm)
	: GUI::Debugger() {
	_vm = vm;

	DCmd_Register("continue",	WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("location",	WRAP_METHOD(Debugger, Cmd_Location));
	DCmd_Register("give",		WRAP_METHOD(Debugger, Cmd_Give));
	DCmd_Register("zones",		WRAP_METHOD(Debugger, Cmd_Zones));
	DCmd_Register("animations",	WRAP_METHOD(Debugger, Cmd_Animations));
	DCmd_Register("globalflags",WRAP_METHOD(Debugger, Cmd_GlobalFlags));
	DCmd_Register("localflags",	WRAP_METHOD(Debugger, Cmd_LocalFlags));
	DCmd_Register("locations",	WRAP_METHOD(Debugger, Cmd_Locations));
	DCmd_Register("gfxobjects",	WRAP_METHOD(Debugger, Cmd_GfxObjects));
	DCmd_Register("set", 		WRAP_METHOD(Debugger, Cmd_Set));
	DCmd_Register("programs", 	WRAP_METHOD(Debugger, Cmd_Programs));

}


void Debugger::preEnter() {
}


void Debugger::postEnter() {
}

bool Debugger::Cmd_Location(int argc, const char **argv) {

	const char *character = _vm->_char.getName();
	const char *location = _vm->_location._name;

	char tmp[PATH_LEN];

	switch (argc) {
	case 3:
		character = const_cast<char*>(argv[2]);
		location = const_cast<char*>(argv[1]);
		sprintf(tmp, "%s.%s", location, character);
		_vm->scheduleLocationSwitch(tmp);
		break;

	case 2:
		location = const_cast<char*>(argv[1]);
		_vm->scheduleLocationSwitch(location);
		break;

	case 1:
		DebugPrintf("location <location name> [character name]\n");

	}

	return true;
}

bool Debugger::Cmd_Locations(int argc, const char **argv) {

	DebugPrintf("+------------------------------+---------+\n"
				"| location name                |  flags  |\n"
				"+------------------------------+---------+\n");
	for (uint i = 0; i < _vm->_numLocations; i++) {
		DebugPrintf("|%-30s| %08x|\n", _vm->_locationNames[i], _vm->_localFlags[i]);
	}
	DebugPrintf("+------------------------------+---------+\n");

	return true;
}

bool Debugger::Cmd_GlobalFlags(int argc, const char **argv) {

	uint32 flags = _globalFlags;

	DebugPrintf("+------------------------------+---------+\n"
				"| flag name                    |  value  |\n"
				"+------------------------------+---------+\n");
	for (uint i = 0; i < _vm->_globalFlagsNames->count(); i++) {
		const char *value = ((flags & (1 << i)) == 0) ? "OFF" : "ON";
		DebugPrintf("|%-30s|   %-6s|\n", _vm->_globalFlagsNames->item(i),  value);
	}
	DebugPrintf("+------------------------------+---------+\n");

	return true;
}

bool Debugger::Cmd_LocalFlags(int argc, const char **argv) {

	uint32 flags = _vm->getLocationFlags();

	DebugPrintf("+------------------------------+---------+\n"
				"| flag name                    |  value  |\n"
				"+------------------------------+---------+\n");
	for (uint i = 0; i < _vm->_localFlagNames->count(); i++) {
		const char *value = ((flags & (1 << i)) == 0) ? "OFF" : "ON";
		DebugPrintf("|%-30s|   %-6s|\n", _vm->_localFlagNames->item(i),  value);
	}
	DebugPrintf("+------------------------------+---------+\n");

	return true;
}

bool Debugger::Cmd_Give(int argc, const char **argv) {

	if (argc == 1) {
		DebugPrintf("give <item name>\n");
	} else {
		int index = _vm->_objectsNames->lookup(argv[1]);
		if (index != Table::notFound)
			_vm->addInventoryItem(index + 4);
		else
			DebugPrintf("invalid item name '%s'\n", argv[1]);
	}

	return true;
}


bool Debugger::Cmd_Zones(int argc, const char **argv) {

	ZoneList::iterator b = _vm->_location._zones.begin();
	ZoneList::iterator e = _vm->_location._zones.end();

	DebugPrintf("+--------------------+---+---+---+---+--------+--------+\n"
				"| name               | l | t | r | b |  type  |  flag  |\n"
				"+--------------------+---+---+---+---+--------+--------+\n");
	for ( ; b != e; b++) {
		ZonePtr z = *b;
		DebugPrintf("|%-20s|%3i|%3i|%3i|%3i|%8x|%8x|\n", z->_name, z->getX(), z->getY(), z->getX() + z->width(), z->getY() + z->height(), z->_type, z->_flags );
	}
	DebugPrintf("+--------------------+---+---+---+---+--------+--------+\n");


	return true;
}

bool Debugger::Cmd_Animations(int argc, const char **argv) {

	AnimationList::iterator b = _vm->_location._animations.begin();
	AnimationList::iterator e = _vm->_location._animations.end();

	DebugPrintf("+--------------------+---+---+---+---+--------+--------+\n"
				"| name               | x | y | z | f |  type  |  flag  | \n"
				"+--------------------+---+---+---+---+--------+--------+\n");
	for ( ; b != e; b++) {
		AnimationPtr a = *b;
		DebugPrintf("|%-20s|%3i|%3i|%3i|%3i|%8x|%8x|\n", a->_name, a->getX(), a->getY(), a->getZ(), a->getF(), a->_type, a->_flags );
	}
	DebugPrintf("+--------------------+---+---+---+---+--------+--------+\n");


	return true;
}

bool Debugger::Cmd_GfxObjects(int argc, const char **argv) {

	const char *objType[] = { "DOOR", "GET", "ANIM" };

	DebugPrintf("+--------------------+-----+-----+-----+-------+-----+--------+--------+\n"
				"| name               |  x  |  y  |  z  | layer |  f  |  type  |  visi  |\n"
				"+--------------------+-----+-----+-----+-------+-----+--------+--------+\n");

	GfxObjList::iterator b = _vm->_gfx->_gfxobjList.begin();
	GfxObjList::iterator e = _vm->_gfx->_gfxobjList.end();

	for ( ; b != e; b++) {
		GfxObj *obj = *b;
		DebugPrintf("|%-20s|%5i|%5i|%5i|%7i|%5i|%8s|%8x|\n", obj->getName(), obj->x, obj->y, obj->z, obj->layer, obj->frame, objType[obj->type], obj->isVisible() );
	}

	DebugPrintf("+--------------------+-----+-----+-----+-------+-----+--------+--------+\n");

	return true;
}

bool Debugger::Cmd_Set(int argc, const char** argv) {

	if (argc < 3) {
		DebugPrintf("set <var name> <value>\n");
	} else {
		_vm->_gfx->setVar(Common::String(argv[1]), atoi(argv[2]));
	}

	return true;
}

bool Debugger::Cmd_Programs(int argc, const char** argv) {

	ProgramList::iterator b = _vm->_location._programs.begin();
	ProgramList::iterator e = _vm->_location._programs.end();

	const char *status[] = { "idle", "running", "completed" };

	int i = 1;

	DebugPrintf("+---+--------------------+--------+----------+\n"
				"| # | bound animation    |  size  |  status  |\n"
				"+---+--------------------+--------+----------+\n");
	for ( ; b != e; b++, i++) {
		ProgramPtr p = *b;
		DebugPrintf("|%3i|%-20s|%8i|%-10s|\n", i, p->_anim->_name, p->_instructions.size(), status[p->_status] );
	}
	DebugPrintf("+---+--------------------+--------+----------+\n");

	return true;
}

} // namespace Parallaction
