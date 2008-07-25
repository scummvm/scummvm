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
#include "common/util.h"

#include "parallaction/parallaction.h"
#include "parallaction/input.h"
#include "parallaction/sound.h"

namespace Parallaction {


const char *Parallaction_br::_partNames[] = {
	"PART0",
	"PART1",
	"PART2",
	"PART3",
	"PART4"
};

const char *partFirstLocation[] = {
	"intro",
	"museo",
	"start",
	"bolscoi",
	"treno"
};

int Parallaction_br::init() {

	_screenWidth = 640;
	_screenHeight = 400;

	if (getGameType() == GType_BRA) {
		if (getPlatform() == Common::kPlatformPC) {
			_disk = new DosDisk_br(this);
			_disk->setLanguage(2);					// NOTE: language is now hardcoded to English. Original used command-line parameters.
			_soundMan = new DummySoundMan(this);
		} else {
			_disk = new AmigaDisk_br(this);
			_disk->setLanguage(2);					// NOTE: language is now hardcoded to English. Original used command-line parameters.
			_soundMan = new AmigaSoundMan(this);
		}
	} else {
		error("unknown game type");
	}


	initResources();
	initFonts();
	initCursors();
	_locationParser = new LocationParser_br(this);
	_locationParser->init();
	_programParser = new ProgramParser_br(this);
	_programParser->init();

	_cmdExec = new CommandExec_br(this);
	_cmdExec->init();
	_programExec = new ProgramExec_br(this);
	_programExec->init();

	_part = -1;

	_subtitle[0] = -1;
	_subtitle[1] = -1;

	Parallaction::init();

	return 0;
}

Parallaction_br::~Parallaction_br() {
	freeFonts();

	delete _dinoCursor;
	delete _dougCursor;
	delete _donnaCursor;

}

void Parallaction_br::callFunction(uint index, void* parm) {
	assert(index < 6);	// magic value 6 is maximum # of callables for Big Red Adventure

	(this->*_callables[index])(parm);
}

int Parallaction_br::go() {

	startGui();

	while ((_engineFlags & kEngineQuit) == 0) {

//		initCharacter();

		_input->_inputMode = Input::kInputModeGame;
		while ((_engineFlags & (kEngineReturn | kEngineQuit)) == 0) {
			runGame();
		}
		_engineFlags &= ~kEngineReturn;

		freePart();
//		freeCharacter();

	}

	return 0;
}



void Parallaction_br::freeFonts() {

	delete _menuFont;
	delete _dialogueFont;

	return;
}

void Parallaction_br::initCursors() {

	if (getPlatform() == Common::kPlatformPC) {
		_dinoCursor = _disk->loadPointer("pointer1");
		_dougCursor = _disk->loadPointer("pointer2");
		_donnaCursor = _disk->loadPointer("pointer3");

		_mouseArrow = _donnaCursor;
	} else {
		// TODO: Where are the Amiga cursors?
	}

}

void Parallaction_br::setMousePointer(int16 index) {
	// FIXME: Where are the Amiga cursors?
	if (getPlatform() == Common::kPlatformAmiga)
		return;

	Common::Rect r;
	_mouseArrow->getRect(0, r);

	_system->setMouseCursor(_mouseArrow->getData(0), r.width(), r.height(), 0, 0, 0);
	_system->showMouse(true);

}

void Parallaction_br::initPart() {

	memset(_counters, 0, ARRAYSIZE(_counters));

	_globalTable = _disk->loadTable("global");
	_objectsNames = _disk->loadTable("objects");
	_countersNames = _disk->loadTable("counters");

//	_disk->loadObjects("icone.ico");

}

void Parallaction_br::freePart() {

	delete _globalTable;
	delete _objectsNames;
	delete _countersNames;

	_globalTable = 0;
	_objectsNames = 0;
	_countersNames = 0;
}

void Parallaction_br::startPart(uint part) {
	_part = part;
	_disk->selectArchive(_partNames[_part]);

	initPart();

	strcpy(_location._name, partFirstLocation[_part]);

	parseLocation("common");
	changeLocation(_location._name);

}

void Parallaction_br::runPendingZones() {
	ZonePtr z;

	if (_activeZone) {
		z = _activeZone;	// speak Zone or sound
		_activeZone = nullZonePtr;
		if ((z->_type & 0xFFFF) == kZoneSpeak) {
			enterDialogueMode(z);
		} else {
			runZone(z);			// FIXME: BRA doesn't handle sound yet
		}
	}

	if (_activeZone2) {
		z = _activeZone2;	// speak Zone or sound
		_activeZone2 = nullZonePtr;
		if ((z->_type & 0xFFFF) == kZoneSpeak) {
			enterDialogueMode(z);
		} else {
			runZone(z);			// FIXME: BRA doesn't handle sound yet
		}
	}
}


void Parallaction_br::changeLocation(char *location) {

	// free open location stuff
	clearSubtitles();
	freeBackground();
	_gfx->clearGfxObjects(kGfxObjNormal);
	_location._programs.clear();

	_location._animations.remove(_char._ani);

	freeZones();
	freeAnimations();

	_location._animations.push_front(_char._ani);

//	free(_location._comment);
//	_location._comment = 0;
	_location._commands.clear();
	_location._aCommands.clear();

	// load new location
	parseLocation(location);

	// kFlagsRemove is cleared because the character defaults to visible on new locations
	// script command can hide the character, anyway, so that's why the flag is cleared
	// before _location._commands are executed
	_char._ani->_flags &= ~kFlagsRemove;

	_cmdExec->run(_location._commands);
//	doLocationEnterTransition();
	_cmdExec->run(_location._aCommands);

	_engineFlags &= ~kEngineChangeLocation;
}


// FIXME: Parallaction_br::parseLocation() is now a verbatim copy of the same routine from Parallaction_ns.
void Parallaction_br::parseLocation(const char *filename) {
	debugC(1, kDebugParser, "parseLocation('%s')", filename);

	allocateLocationSlot(filename);
	Script *script = _disk->loadLocation(filename);

	_locationParser->parse(script);
	delete script;

	// this loads animation scripts
	AnimationList::iterator it = _vm->_location._animations.begin();
	for ( ; it != _vm->_location._animations.end(); it++) {
		if ((*it)->_scriptName) {
			loadProgram(*it, (*it)->_scriptName);
		}
	}

	debugC(1, kDebugParser, "parseLocation('%s') done", filename);
	return;
}

void Parallaction_br::loadProgram(AnimationPtr a, const char *filename) {
	debugC(1, kDebugParser, "loadProgram(Animation: %s, script: %s)", a->_name, filename);

	Script *script = _disk->loadScript(filename);
	ProgramPtr program(new Program);
	program->_anim = a;

	_programParser->parse(script, program);

	delete script;

	_vm->_location._programs.push_back(program);

	debugC(1, kDebugParser, "loadProgram() done");

	return;
}



void Parallaction_br::changeCharacter(const char *name) {
	const char *charName = _char.getName();

	if (scumm_stricmp(charName, name)) {
		debugC(1, kDebugExec, "changeCharacter(%s)", name);

		_char.setName(name);
		_char._ani->gfxobj = _gfx->loadAnim(name);
		_char._ani->gfxobj->setFlags(kGfxObjCharacter);
		_char._ani->gfxobj->clearFlags(kGfxObjNormal);
		_char._talk = _disk->loadTalk(name);
	}

	_char._ani->_flags |= kFlagsActive;
}


void Parallaction_br::setArrowCursor() {
	// TODO: choose the pointer depending on the active character
	// For now, defaults to 0, that corresponds to the default in the original
	setMousePointer(0);
}

void Parallaction_br::setInventoryCursor(int pos) {



}

} // namespace Parallaction
