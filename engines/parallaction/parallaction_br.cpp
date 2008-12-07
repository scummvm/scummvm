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
#include "parallaction/saveload.h"
#include "parallaction/sound.h"

namespace Parallaction {


const char *Parallaction_br::_partNames[] = {
	"PART0",
	"PART1",
	"PART2",
	"PART3",
	"PART4"
};

Parallaction_br::Parallaction_br(OSystem* syst, const PARALLACTIONGameDescription *gameDesc) : Parallaction_ns(syst, gameDesc), 
	_locationParser(0), _programParser(0) { 
}

Common::Error Parallaction_br::init() {

	_screenWidth = 640;
	_screenHeight = 400;

	if (getGameType() == GType_BRA) {
		if (getPlatform() == Common::kPlatformPC) {
			if (getFeatures() & GF_DEMO) {
				_disk = new DosDemoDisk_br(this);
			} else {
				_disk = new DosDisk_br(this);
			}
			_disk->setLanguage(2);					// NOTE: language is now hardcoded to English. Original used command-line parameters.
			_soundMan = new DummySoundMan(this);
		} else {
			_disk = new AmigaDisk_br(this);
			_disk->setLanguage(2);					// NOTE: language is now hardcoded to English. Original used command-line parameters.
			_soundMan = new AmigaSoundMan(this);
		}

		_disk->init();
	} else {
		error("unknown game type");
	}


	initResources();
	initFonts();
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

	_countersNames = 0;

	_saveLoad = new SaveLoad_br(this, _saveFileMan);

	Parallaction::init();

	return Common::kNoError;
}

Parallaction_br::~Parallaction_br() {
	freeFonts();
	
	delete _locationParser;
	delete _programParser;
}

void Parallaction_br::callFunction(uint index, void* parm) {
	assert(index < 6);	// magic value 6 is maximum # of callables for Big Red Adventure

	(this->*_callables[index])(parm);
}

Common::Error Parallaction_br::go() {

	bool splash = true;

	while (!shouldQuit()) {

		if (getFeatures() & GF_DEMO) {
			scheduleLocationSwitch("camalb.1");
			_input->_inputMode = Input::kInputModeGame;
		} else {
			startGui(splash);
			 // don't show splash after first time
			splash = false;
		}

//		initCharacter();

		while (((_engineFlags & kEngineReturn) == 0) && (!shouldQuit())) {
			runGame();
		}
		_engineFlags &= ~kEngineReturn;

		cleanupGame();
	}

	return Common::kNoError;
}



void Parallaction_br::freeFonts() {
	delete _menuFont;
	delete _dialogueFont;

	_menuFont  = 0;
	_dialogueFont = 0;
	_labelFont = 0;
	_introFont = 0;
}


void Parallaction_br::runPendingZones() {
	ZonePtr z;

	_cmdExec->runSuspended();

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

void Parallaction_br::freeLocation(bool removeAll) {

	// free open location stuff
	clearSubtitles();
	freeBackground();
	_gfx->clearGfxObjects(kGfxObjNormal);
	_gfx->freeLabels();
	_subtitle[0] = _subtitle[1] = -1;

	_location._animations.remove(_char._ani);
	_location.cleanup(removeAll);
	_location._animations.push_front(_char._ani);

}

void Parallaction_br::cleanupGame() {
	freeLocation(true);

//		freeCharacter();

	delete _globalFlagsNames;
	delete _objectsNames;
	delete _countersNames;

	_globalFlagsNames = 0;
	_objectsNames = 0;
	_countersNames = 0;
}


void Parallaction_br::changeLocation(char *location) {
	char *partStr = strrchr(location, '.');
	if (partStr) {
		cleanupGame();

		int n = partStr - location;
		strncpy(_location._name, location, n);
		_location._name[n] = '\0';

		_part = atoi(++partStr);
		if (getFeatures() & GF_DEMO) {
			assert(_part == 1);
		} else {
			assert(_part >= 0 && _part <= 4);
		}

		_disk->selectArchive(_partNames[_part]);

		memset(_counters, 0, ARRAYSIZE(_counters));

		_globalFlagsNames = _disk->loadTable("global");
		_objectsNames = _disk->loadTable("objects");
		_countersNames = _disk->loadTable("counters");

		// TODO: maybe handle this into Disk
		if (getPlatform() == Common::kPlatformPC) {
			_char._objs = _disk->loadObjects("icone.ico");
		} else {
			_char._objs = _disk->loadObjects("icons.ico");
		}

		parseLocation("common.slf");
	}

	freeLocation(false);
	// load new location
	parseLocation(location);
	// kFlagsRemove is cleared because the character is visible by default.
	// Commands can hide the character, anyway.
	_char._ani->_flags &= ~kFlagsRemove;
	_cmdExec->run(_location._commands);

	doLocationEnterTransition();

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



} // namespace Parallaction
