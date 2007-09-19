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


const char *_jobDescriptions[] = {
	"draw label",
	"draw mouse",
	"delayed label deletion || show inventory",
	"draw animations",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"delayed label deletion || run scripts || erase animations",
	"NONE",
	"put item || pickup item",
	"toggle door",
	"walk",
	"erase label || hide inventory",
	"erase mouse"
};

Debugger::Debugger(Parallaction *vm)
	: GUI::Debugger() {
	_vm = vm;

	DCmd_Register("continue", WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("location", WRAP_METHOD(Debugger, Cmd_Location));
	DCmd_Register("give",     WRAP_METHOD(Debugger, Cmd_Give));
	DCmd_Register("jobs",     WRAP_METHOD(Debugger, Cmd_Jobs));
	DCmd_Register("zones",     WRAP_METHOD(Debugger, Cmd_Zones));
	DCmd_Register("animations",     WRAP_METHOD(Debugger, Cmd_Animations));


}


void Debugger::preEnter() {
}


void Debugger::postEnter() {
}

bool Debugger::Cmd_Location(int argc, const char **argv) {

	char *character = _vm->_characterName;
	char *location = _vm->_location._name;

	switch (argc) {
	case 3:
		character = const_cast<char*>(argv[2]);
		location = const_cast<char*>(argv[1]);
		sprintf(_vm->_location._name, "%s.%s", location, character);
		// TODO: check if location exists
		_engineFlags |= kEngineChangeLocation;
		break;

	case 2:
		location = const_cast<char*>(argv[1]);
		sprintf(_vm->_location._name, "%s", location);
		// TODO: check if location exists
		_engineFlags |= kEngineChangeLocation;
		break;

	case 1:
		DebugPrintf("location <location name> [character name]\n");

	}

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


bool Debugger::Cmd_Jobs(int argc, const char **argv) {

	JobList::iterator b = _vm->_jobs.begin();
	JobList::iterator e = _vm->_jobs.end();

	DebugPrintf("+---+-------------------------------------------------------------+\n"
				"|tag| description                                                 |\n"
				"+---+-------------------------------------------------------------+\n");
	for ( ; b != e; b++) {
		DebugPrintf("|%3i| %-60s|\n", (*b)->_job->_tag, _jobDescriptions[(*b)->_job->_tag] );
	}
	DebugPrintf("+---+-------------------------------------------------------------+\n");


	return true;
}

bool Debugger::Cmd_Zones(int argc, const char **argv) {

	ZoneList::iterator b = _vm->_zones.begin();
	ZoneList::iterator e = _vm->_zones.end();

	DebugPrintf("+--------------------+---+---+---+---+--------+--------+\n"
				"| name               | l | t | r | b |  type  |  flag  |\n"
				"+--------------------+---+---+---+---+--------+--------+\n");
	for ( ; b != e; b++) {
		Zone *z = *b;
		DebugPrintf("|%-20s|%3i|%3i|%3i|%3i|%8x|%8x|\n", z->_label._text, z->_left, z->_top, z->_right, z->_bottom, z->_type, z->_flags );
	}
	DebugPrintf("+--------------------+---+---+---+---+--------+--------+\n");


	return true;
}

bool Debugger::Cmd_Animations(int argc, const char **argv) {

	AnimationList::iterator b = _vm->_animations.begin();
	AnimationList::iterator e = _vm->_animations.end();

	DebugPrintf("+--------------------+---+---+---+---+--------+--------+\n"
				"| name               | x | y | z | f |  type  |  flag  | \n"
				"+--------------------+---+---+---+---+--------+--------+\n");
	for ( ; b != e; b++) {
		Animation *a = *b;
		DebugPrintf("|%-20s|%3i|%3i|%3i|%3i|%8x|%8x|\n", a->_label._text, a->_left, a->_top, a->_z, a->_frame, a->_type, a->_flags );
	}
	DebugPrintf("+--------------------+---+---+---+---+--------+--------+\n");


	return true;
}

} // namespace Parallaction
