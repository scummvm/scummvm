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

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/util.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "parallaction/parallaction.h"
#include "parallaction/debug.h"
#include "parallaction/menu.h"
#include "parallaction/sound.h"



namespace Parallaction {

// FIXME: remove this
Parallaction *_vm = NULL;

// public stuff

uint16	_mouseButtons = 0;


char		_saveData1[30] = { '\0' };
uint16		_language = 0;
char		_slideText[2][40];
uint32		_engineFlags = 0;
Zone	   *_activeZone = NULL;

uint16		_score = 1;

uint32		_localFlags[120] = { 0 };
Command *	_forwardedCommands[20] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

char		_forwardedAnimationNames[20][20];
uint16		_numForwards = 0;

uint32		_commandFlags = 0;
uint16		_introSarcData3 = 200;
uint16		_introSarcData2 = 1;

// private stuff

static Job	   *_jDrawInventory = NULL;
Job	   *_jDrawLabel = NULL;
Job	   *_jEraseLabel = NULL;
Zone    *_hoverZone = NULL;
static Job	   *_jRunScripts = NULL;


Parallaction::Parallaction(OSystem *syst) :
	Engine(syst) {

	// FIXME
	_vm = this;

	_mouseHidden = false;

	Common::File::addDefaultDirectory( _gameDataPath );

	Common::addSpecialDebugLevel(kDebugDialogue, "dialogue", "Dialogues debug level");
	Common::addSpecialDebugLevel(kDebugLocation, "location", "Location debug level");
	Common::addSpecialDebugLevel(kDebugDisk, "disk", "Disk debug level");
	Common::addSpecialDebugLevel(kDebugWalk, "walk", "Walk debug level");
	Common::addSpecialDebugLevel(kDebugGraphics, "gfx", "Gfx debug level");
	Common::addSpecialDebugLevel(kDebugJobs, "jobs", "Jobs debug level");
	Common::addSpecialDebugLevel(kDebugInput, "input", "Input debug level");
	Common::addSpecialDebugLevel(kDebugAudio, "audio", "Audio debug level");
	Common::addSpecialDebugLevel(kDebugMenu, "menu", "Menu debug level");
}


Parallaction::~Parallaction() {
	delete _debugger;

	delete _globalTable;

	delete _callableNames;
	delete _commandsNames;
	delete _instructionNames;
	delete _zoneTypeNames;
	delete _zoneFlagNames;

	_animations.remove(&_char._ani);

	freeLocation();

	freeCharacter();
	destroyInventory();

	delete _gfx;
	delete _soundMan;
	delete _disk;
}


int Parallaction::init() {

	// Detect game
	if (!detectGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}

	_objectsNames = NULL;
	_globalTable = NULL;
	_localFlagNames = NULL;
	initResources();

	_hasLocationSound = false;

	_skipMenu = false;

	_transCurrentHoverItem = 0;
	_actionAfterWalk = false;  // actived when the character needs to move before taking an action
	_activeItem._index = 0;
	_activeItem._id = 0;
	_procCurrentHoverItem = -1;

//	_musicData1 = 0;
	strcpy(_characterName1, "null");

	_soundMan = 0;

	_baseTime = 0;

	if (getPlatform() == Common::kPlatformPC) {
		_disk = new DosDisk(this);
	} else {
		if (getFeatures() & GF_DEMO) {
			strcpy(_location._name, "fognedemo");
		}
		_disk = new AmigaDisk(this);
		_disk->selectArchive((_vm->getFeatures() & GF_DEMO) ? "disk0" : "disk1");
	}

	_engineFlags = 0;

	strcpy(_characterName, "dough");

	memset(_locationNames, 0, 120*32);
	_numLocations = 0;

	_location._startPosition.x = -1000;
	_location._startPosition.y = -1000;
	_location._startFrame = 0;

	_location._comment = NULL;
	_location._endComment = NULL;

	initWalk();

	initInventory();

	_animations.push_front(&_vm->_char._ani);
	_gfx = new Gfx(this);

	if (getPlatform() == Common::kPlatformPC) {
		int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
		MidiDriver *driver = MidiDriver::createMidi(midiDriver);
		_soundMan = new DosSoundMan(this, driver);
		_soundMan->setMusicVolume(ConfMan.getInt("music_volume"));
	} else {
		_soundMan = new AmigaSoundMan(this);
	}

	_debugger = new Debugger(this);

	return 0;
}





int Parallaction::go() {

	initGame();
	runGame();

	return 0;
}

void Parallaction::initGame() {

	_menu = new Menu(this);

	initGlobals();
	if (_skipMenu == false) {
		_menu->start();
	}

	char *v4 = strchr(_location._name, '.');
	if (v4) {
		*v4 = '\0';
	}

	_engineFlags &= ~kEngineChangeLocation;
	changeCharacter(_characterName);

	strcpy(_saveData1, _location._name);
	parseLocation(_location._name);

	if (_location._startPosition.x != -1000) {
		_vm->_char._ani._left = _location._startPosition.x;
		_vm->_char._ani._top = _location._startPosition.y;
		_vm->_char._ani._frame = _location._startFrame;
		_location._startPosition.y = -1000;
		_location._startPosition.x = -1000;
	}

	return;
}

void Parallaction::initGlobals() {
	_globalTable = _disk->loadTable("global");
}

// FIXME: the engine has 3 event loops. The following routine hosts the main one,
// and it's called from 8 different places in the code. There exist 2 more specialised
// loops which could possibly be merged into this one with some effort in changing
// caller code, i.e. adding condition checks.
//
uint16 Parallaction::updateInput() {

	Common::Event e;
	uint16 KeyDown = 0;

	_mouseButtons = kMouseNone;

	Common::EventManager *eventMan = _system->getEventManager();
	while (eventMan->pollEvent(e)) {

		switch (e.type) {
		case Common::EVENT_KEYDOWN:
			if (e.kbd.flags == Common::KBD_CTRL && e.kbd.keycode == 'd')
				_debugger->attach();
			if (getFeatures() & GF_DEMO) break;
			if (e.kbd.keycode == Common::KEYCODE_l) KeyDown = kEvLoadGame;
			if (e.kbd.keycode == Common::KEYCODE_s) KeyDown = kEvSaveGame;
			break;

		case Common::EVENT_LBUTTONDOWN:
			_mouseButtons = kMouseLeftDown;
			break;

		case Common::EVENT_LBUTTONUP:
			_mouseButtons = kMouseLeftUp;
			break;

		case Common::EVENT_RBUTTONDOWN:
			_mouseButtons = kMouseRightDown;
			break;

		case Common::EVENT_RBUTTONUP:
			_mouseButtons = kMouseRightUp;
			break;

		case Common::EVENT_MOUSEMOVE:
			_mousePos = e.mouse;
			break;

		case Common::EVENT_QUIT:
			_engineFlags |= kEngineQuit;
			g_system->quit();
			break;

		default:
			break;

		}

	}

	if (_debugger->isAttached())
		_debugger->onFrame();

	return KeyDown;

}

// FIXME: see comment for updateInput()
void waitUntilLeftClick() {

	Common::Event e;

	Common::EventManager *eventMan = g_system->getEventManager();
	for (;;) {
		eventMan->pollEvent(e);

		if (e.type == Common::EVENT_LBUTTONUP)
			break;

		if (e.type == Common::EVENT_QUIT) {
			_engineFlags |= kEngineQuit;
			break;
		}

		g_system->delayMillis(10);
	}


	return;
}


void Parallaction::runGame() {

	addJob(&jobEraseAnimations, (void*)1, kPriority20);
	_jRunScripts = addJob(&jobRunScripts, 0, kPriority15);
	addJob(&jobDisplayAnimations, 0, kPriority3);

	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBit2);

	if (_location._commands.size() > 0)
		runCommands(_location._commands);

	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

	if (_location._comment)
		doLocationEnterTransition();

	if (_hasLocationSound)
		_soundMan->playSfx(_locationSound, 0, true);

	changeCursor(kCursorArrow);

	if (_location._aCommands.size() > 0)
		runCommands(_location._aCommands);

	while ((_engineFlags & kEngineQuit) == 0) {
		_keyDown = updateInput();

		debugC(3, kDebugInput, "runGame: input flags (%i, %i, %i, %i)",
			!_mouseHidden,
			(_engineFlags & kEngineBlockInput) == 0,
			(_engineFlags & kEngineWalking) == 0,
			(_engineFlags & kEngineChangeLocation) == 0
		);

		// WORKAROUND: the engine doesn't check for displayed labels before performing a location
		// switch, thus crashing whenever a jobDisplayLabel/jEraseLabel pair is left into the
		// queue after the character enters a door.
		// Skipping input processing when kEngineChangeLocation is set solves the issue. It's
		// noteworthy that the programmers added this very check in Big Red Adventure's engine,
		// so it should be ok here in Nippon Safes too.
		if ((!_mouseHidden) && ((_engineFlags & kEngineBlockInput) == 0) && ((_engineFlags & kEngineWalking) == 0) && ((_engineFlags & kEngineChangeLocation) == 0)) {
			InputData *v8 = translateInput();
			if (v8) processInput(v8);
		}

		if (_activeZone) {
			Zone *z = _activeZone;	// speak Zone or sound
			_activeZone = NULL;
			if (runZone( z ) == 0)
				runCommands( z->_commands, z );
		}

		if (_engineFlags & kEngineChangeLocation) {
			_engineFlags &= ~kEngineChangeLocation;
			changeLocation(_location._name);
			continue;
		}

		g_system->delayMillis(30);

		runJobs();

		if ((_engineFlags & kEnginePauseJobs) == 0 || (_engineFlags & kEngineInventory)) {
			_gfx->swapBuffers();
			_gfx->animatePalette();
		}

	}

	delete _menu;

	return;
}


void Parallaction::processInput(InputData *data) {
	Zone *z;

	switch (data->_event) {
	case kEvEnterZone:
		debugC(2, kDebugInput, "processInput: kEvEnterZone");
		_gfx->_labelPosition[1].x = -1000;
		_gfx->_labelPosition[1].y = -1000;
		_gfx->_labelPosition[0].x = -1000;
		_gfx->_labelPosition[0].y = -1000;
		_jDrawLabel = addJob(&jobDisplayLabel, (void*)data->_label, kPriority0);
		_jEraseLabel = addJob(&jobEraseLabel, (void*)data->_label, kPriority20);
		break;

	case kEvExitZone:
		debugC(2, kDebugInput, "processInput: kEvExitZone");
		removeJob(_jDrawLabel);
		addJob(&jobWaitRemoveJob, _jEraseLabel, kPriority15);
		_jDrawLabel = NULL;
		break;

	case kEvAction:
		debugC(2, kDebugInput, "processInput: kEvAction");
		_procCurrentHoverItem = -1;
		_hoverZone = NULL;
		pauseJobs();
		z = data->_zone;
		if (runZone(z) == 0) {
			runCommands( z->_commands, z );
		}
		resumeJobs();
		break;

	case kEvOpenInventory:
		_procCurrentHoverItem = -1;
		_hoverZone = NULL;
		if (_jDrawLabel != 0) {
			removeJob(_jDrawLabel);
			_jDrawLabel = NULL;
			addJob(&jobWaitRemoveJob, _jEraseLabel, kPriority2);
		}
		if (hitZone(kZoneYou, _mousePos.x, _mousePos.y) == 0)
		changeCursor(kCursorArrow);
		removeJob(_jRunScripts);
		_jDrawInventory = addJob(&jobShowInventory, 0, kPriority2);
		openInventory();
		break;

	case kEvCloseInventory: // closes inventory and possibly select item
		closeInventory();
		if ((data->_inventoryIndex != -1) && (_inventory[data->_inventoryIndex]._id != 0)) {
			// activates item
			changeCursor(data->_inventoryIndex);
		}
		_jRunScripts = addJob(&jobRunScripts, 0, kPriority15);
		addJob(&jobHideInventory, 0, kPriority20);
		removeJob(_jDrawInventory);
		break;

	case kEvHoverInventory:
		highlightInventoryItem(_procCurrentHoverItem, 12);	// disable
		highlightInventoryItem(data->_inventoryIndex, 19);						// enable
		_procCurrentHoverItem = data->_inventoryIndex;
		break;

	case kEvWalk: {
		debugC(2, kDebugInput, "processInput: kEvWalk");
		_hoverZone = NULL;
		changeCursor(kCursorArrow);
		if (_vm->_char._ani._flags & kFlagsRemove) break;
		if ((_vm->_char._ani._flags & kFlagsActive) == 0) break;
		WalkNodeList *v4 = _vm->_char._builder.buildPath(data->_mousePos.x, data->_mousePos.y);
		addJob(&jobWalk, v4, kPriority19);
		_engineFlags |= kEngineWalking; 								   // inhibits processing of input until walking is over
		}
		break;

	case kEvQuitGame:
		_engineFlags |= kEngineQuit;
		break;

	case kEvSaveGame:
		_hoverZone = NULL;
		saveGame();
		changeCursor(kCursorArrow);
		break;

	case kEvLoadGame:
		_hoverZone = NULL;
		loadGame();
		changeCursor(kCursorArrow);
		break;

	}

	return;
}



Parallaction::InputData *Parallaction::translateInput() {

	if (_keyDown == kEvQuitGame) {
		_input._event = kEvQuitGame;
		return &_input;
	}

	if (_keyDown == kEvSaveGame) {
		_input._event = kEvSaveGame;
		return &_input;
	}

	if (_keyDown == kEvLoadGame) {
		_input._event = kEvLoadGame;
		return &_input;
	}

	_input._mousePos = _mousePos;

	if (((_engineFlags & kEnginePauseJobs) == 0) && ((_engineFlags & kEngineInventory) == 0)) {

		if (_actionAfterWalk == true) {
			// if walking is over, then take programmed action
			_input._event = kEvAction;
			_actionAfterWalk = false;
			return &_input;
		}

		Zone *z = hitZone(_activeItem._id, _mousePos.x, _mousePos.y);

		if (_mouseButtons == kMouseRightDown) {
			// right button down shows inventory

			if (hitZone(kZoneYou, _mousePos.x, _mousePos.y) && (_activeItem._id != 0)) {
				_activeItem._index = (_activeItem._id >> 16) & 0xFFFF;
				_engineFlags |= kEngineDragging;
			}

			_input._event = kEvOpenInventory;
			_transCurrentHoverItem = -1;
			return &_input;
		}

		if (((_mouseButtons == kMouseLeftUp) && (_activeItem._id == 0) && ((_engineFlags & kEngineWalking) == 0)) && ((z == NULL) || ((z->_type & 0xFFFF) != kZoneCommand))) {
			_input._event = kEvWalk;
			return &_input;
		}

		if ((z != _hoverZone) && (_hoverZone != NULL)) {
			_hoverZone = NULL;
			_input._event = kEvExitZone;
//			_input._data= &z->_name;
			return &_input;
		}

		if (z == NULL) {
			return NULL;
		}

		if ((_hoverZone == NULL) && ((z->_flags & kFlagsNoName) == 0)) {
			_hoverZone = z;
			_input._event = kEvEnterZone;
			_input._label = &z->_label;
			return &_input;
		}

		if ((_mouseButtons == kMouseLeftUp) && ((_activeItem._id != 0) || ((z->_type & 0xFFFF) == kZoneCommand))) {

			_input._zone = z;
			if (z->_flags & kFlagsNoWalk) {
				// character doesn't need to walk to take specified action
				_input._event = kEvAction;

			} else {
				// action delayed: if Zone defined a moveto position the character is programmed to move there,
				// else it will move to the mouse position
				_input._event = kEvWalk;
				_actionAfterWalk = true;
				if (z->_moveTo.y != 0) {
					_input._mousePos = z->_moveTo;
				}
			}

//			beep();
			changeCursor(kCursorArrow);
			return &_input;
		}

	}

	if ((_engineFlags & kEngineInventory) == 0) return NULL;

	// in inventory
	int16 _si = getHoverInventoryItem(_mousePos.x, _mousePos.y);

	if (_mouseButtons == kMouseRightUp) {
		// right up hides inventory

		_input._event = kEvCloseInventory;
		_input._inventoryIndex = getHoverInventoryItem(_mousePos.x, _mousePos.y);
		highlightInventoryItem(_transCurrentHoverItem, 12); 		// disable

		if ((_engineFlags & kEngineDragging) == 0) return &_input;

		_engineFlags &= ~kEngineDragging;
		Zone *z = hitZone(kZoneMerge, _activeItem._index, _inventory[_input._inventoryIndex]._index);

		if (z != NULL) {
			dropItem(z->u.merge->_obj1 - 4);
			dropItem(z->u.merge->_obj2 - 4);
			addInventoryItem(z->u.merge->_obj3);
			runCommands(z->_commands);
		}

		return &_input;
	}

	if (_si == _transCurrentHoverItem) return NULL;

	_transCurrentHoverItem = _si;
	_input._event = kEvHoverInventory;
	_input._inventoryIndex = _si;
	return &_input;

}

uint32 Parallaction::getElapsedTime() {
	return g_system->getMillis() - _baseTime;
}

void Parallaction::resetTimer() {
	_baseTime = g_system->getMillis();
	return;
}


void Parallaction::waitTime(uint32 t) {

	uint32 v4 = 0;

	while (v4 < t * (1000 / 18.2)) {
		v4 = getElapsedTime();
	}

	resetTimer();

	return;
}


void Parallaction::showCursor(bool visible) {
	_mouseHidden = !visible;
	g_system->showMouse(visible);
}

//	changes the mouse pointer
//	index 0 means standard pointer (from pointer.cnv)
//	index > 0 means inventory item
//
void Parallaction::changeCursor(int32 index) {

	if (index == kCursorArrow) {		// standard mouse pointer

		debugC(1, kDebugInput, "changeCursor(%i), label: %p", index, (const void*)_jDrawLabel);

		if (_jDrawLabel != NULL) {
			removeJob(_jDrawLabel);
			addJob(&jobWaitRemoveJob, _jEraseLabel, kPriority15 );
			_jDrawLabel = NULL;
		}

		_activeItem._id = 0;

	} else {
		_activeItem._id = _inventory[index]._id;
	}

	_gfx->setMousePointer(index);

	return;
}



void Parallaction::freeCharacter() {
	debugC(3, kDebugLocation, "freeCharacter()");

	if (!IS_DUMMY_CHARACTER(_vm->_characterName)) {
		if (_objectsNames)
			delete _objectsNames;
		_objectsNames = NULL;

		if (_vm->_char._ani._cnv)
			delete _vm->_char._ani._cnv;
		_vm->_char._ani._cnv = NULL;

		if (_vm->_char._talk)
			delete _vm->_char._talk;
		_vm->_char._talk = NULL;

		_vm->_gfx->freeStaticCnv(_vm->_char._head);
		if (_vm->_char._head)
			delete _vm->_char._head;
		_vm->_char._head = NULL;

		if (_vm->_char._objs)
			delete _vm->_char._objs;
		_vm->_char._objs = NULL;
	}

	return;
}

void Parallaction::changeCharacter(const char *name) {
	debugC(1, kDebugLocation, "changeCharacter(%s)", name);

	char baseName[20];
	if (IS_MINI_CHARACTER(name)) {
		strcpy(baseName, name+4);
	} else {
		strcpy(baseName, name);
	}

	char fullName[20];
	strcpy(fullName, name);
	if (_engineFlags & kEngineTransformedDonna)
		strcat(fullName, "tras");

	if (scumm_stricmp(fullName, _characterName1)) {

		// freeCharacter takes responsibility for checking
		// character for sanity before memory is freed
		freeCharacter();

		Common::String oldArchive = _disk->selectArchive((_vm->getFeatures() & GF_LANG_MULT) ? "disk1" : "disk0");
		_vm->_char._ani._cnv = _disk->loadFrames(fullName);

		if (!IS_DUMMY_CHARACTER(name)) {
			if (_vm->getPlatform() == Common::kPlatformAmiga && (_vm->getFeatures() & GF_LANG_MULT))
				_disk->selectArchive("disk0");

			_vm->_char._head = _disk->loadHead(baseName);
			_vm->_char._talk = _disk->loadTalk(baseName);
			_vm->_char._objs = _disk->loadObjects(baseName);
			_objectsNames = _disk->loadTable(baseName);
			refreshInventory(baseName);

			_soundMan->playCharacterMusic(name);

			if (!(getFeatures() & GF_DEMO))
				parseLocation("common");
		}

		if (!oldArchive.empty())
			_disk->selectArchive(oldArchive);
	}

	strcpy(_characterName1, fullName);

	debugC(1, kDebugLocation, "changeCharacter() done");

	return;
}

/*
	helper function to provide *descending* ordering of the job list
	(higher priorities values comes first in the list)
*/
int compareJobPriority(const JobPointer &j1, const JobPointer &j2) {
	return (j1->_tag >= j2->_tag ? -1 : 1);
}

Job *Parallaction::addJob(JobFn fn, void *parm, uint16 tag) {
	debugC(3, kDebugJobs, "addJob(%i)", tag);

	Job *v8 = new Job;

	v8->_parm = parm;
	v8->_fn = fn;
	v8->_tag = tag;
	v8->_finished = 0;
	v8->_count = 0;

	_jobs.insertSorted(v8, compareJobPriority);

	return v8;
}

void Parallaction::removeJob(Job *j) {
	debugC(3, kDebugJobs, "addJob(%i)", j->_tag);

	j->_finished = 1;
	return;
}

void Parallaction::pauseJobs() {
	debugC(3, kDebugJobs, "pausing jobs execution");

	_engineFlags |= kEnginePauseJobs;
	return;
}

void Parallaction::resumeJobs() {
	debugC(3, kDebugJobs, "resuming jobs execution");

	_engineFlags &= ~kEnginePauseJobs;
	return;
}

void Parallaction::runJobs() {

	if (_engineFlags & kEnginePauseJobs) return;

	JobList::iterator it = _jobs.begin();
	while (it != _jobs.end()) {
		if ((*it)->_finished == 1)
			it = _jobs.erase(it);
		else
			it++;
	}

	it = _jobs.begin();
	while (it != _jobs.end()) {
		debugC(9, kDebugJobs, "runJobs: %i", (*it)->_tag);
		(*(*it)->_fn)((*it)->_parm, (*it));
		it++;
	}


	return;
}

// this Job uses a static counter to delay removal
// and is in fact only used to remove jEraseLabel jobs
//
void jobWaitRemoveJob(void *parm, Job *j) {
	Job *arg = (Job*)parm;

	static uint16 count = 0;

	debugC(3, kDebugJobs, "jobWaitRemoveJob: count = %i", count);

	_engineFlags |= kEngineBlockInput;

	count++;
	if (count == 2) {
		count = 0;
		_vm->removeJob(arg);
		_engineFlags &= ~kEngineBlockInput;
		j->_finished = 1;
	}

	return;
}



Table::Table(uint32 size) : _size(size), _used(0), _disposeMemory(true) {
	_data = (char**)malloc(sizeof(char*)*size);
}

Table::Table(uint32 size, const char **data) : _size(size), _used(size), _disposeMemory(false) {
	_data = const_cast<char**>(data);
}

Table::~Table() {

	if (!_disposeMemory) return;

	for (uint32 i = 0; i < _used; i++)
		free(_data[i]);

	free(_data);

}

void Table::addData(const char* s) {

	if (!(_used < _size))
		error("Table overflow");

	_data[_used++] = strdup(s);

}

int16 Table::lookup(const char* s) {

	for (uint16 i = 0; i < _used; i++) {
		if (!scumm_stricmp(_data[i], s)) return i + 1;
	}

	return -1;
}

} // namespace Parallaction
