/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
#include "parallaction/menu.h"
#include "parallaction/parser.h"
#include "parallaction/disk.h"
#include "parallaction/music.h"
#include "parallaction/inventory.h"
#include "parallaction/graphics.h"
#include "parallaction/walk.h"
#include "parallaction/zone.h"


namespace Parallaction {

// FIXME: remove this
Parallaction *_vm = NULL;

// public stuff
Point	_mousePos = { 0, 0 };
uint16	_mouseButtons = 0;


char		_saveData1[30] = { '\0' };
uint16		_language = 0;
char		_slideText[2][40];
uint32		_engineFlags = 0;
char	   *_objectsNames[100];
Zone	   *_activeZone = NULL;

uint16		_score = 1;

uint32		_localFlags[120] = { 0 };

static char tmp_visited_str[] = "visited";

char *_localFlagNames[32] = {
	tmp_visited_str, // "visited",
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

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
char		_soundFile[20];

char	   *_globalTable[32];

byte		_mouseHidden = 0;

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
	_disk = new Disk(this);

	_skipMenu = false;

	_transCurrentHoverItem = 0;
	_actionAfterWalk = false;  // actived when the character needs to move before taking an action
	_activeItem._index = 0;
	_activeItem._id = 0;
	_procCurrentHoverItem = -1;

	_locationScript = NULL;

	_musicData1 = 0;
	strcpy(_characterName1, "null");

	_midiPlayer = 0;

	_baseTime = 0;

	Common::File::addDefaultDirectory( _gameDataPath );

	Common::addSpecialDebugLevel(kDebugDialogue, "dialogue", "Dialogues debug level");
	Common::addSpecialDebugLevel(kDebugLocation, "location", "Location debug level");
	Common::addSpecialDebugLevel(kDebugDisk, "disk", "Disk debug level");
	Common::addSpecialDebugLevel(kDebugWalk, "walk", "Walk debug level");
	Common::addSpecialDebugLevel(kDebugGraphics, "gfx", "Gfx debug level");
	Common::addSpecialDebugLevel(kDebugJobs, "jobs", "Jobs debug level");
	Common::addSpecialDebugLevel(kDebugInput, "input", "Input debug level");

}


Parallaction::~Parallaction() {
	delete _midiPlayer;
	delete _disk;
}


int Parallaction::init() {

	// Detect game
	if (!detectGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}

	_engineFlags = 0;

	strcpy(_characterName, "dough");

	memset(_locationNames, 0, 120*32);
	_numLocations = 0;

	_location._startPosition._x = -1000;
	_location._startPosition._x = -1000;
	_location._startFrame = 0;
	_location._walkNodes._prev = NULL;
	_location._walkNodes._next = NULL;
	strcpy(_location._name, "fogne");
	_location._aCommands = NULL;
	_location._commands = NULL;
	_location._comment = NULL;
	_location._endComment = NULL;

	initWalk();

	_vm->_char._talk._width = 0;
	_vm->_char._talk._height = 0;
	_vm->_char._talk._count = 0;
	_vm->_char._talk._array = NULL;

	_vm->_char._head._width = 0;
	_vm->_char._head._height = 0;
	_vm->_char._head._data0 = NULL;
	_vm->_char._head._data1 = NULL;

	_vm->_char._ani._zone.pos._position._x = 150;
	_vm->_char._ani._zone.pos._position._y = 100;
	initInventory();
	_vm->_char._ani._z = 10;

	_vm->_char._ani._zone.pos._oldposition._x = -1000;
	_vm->_char._ani._zone.pos._oldposition._y = -1000;
	_vm->_char._ani._frame = 0;

	_vm->_char._ani._zone._flags = kFlagsActive | kFlagsNoName;
	_vm->_char._ani._zone._type = kZoneYou;

	_vm->_char._ani._zone._label._cnv._data0 = NULL;
	_vm->_char._ani._zone._label._text = strdup("yourself");

	addNode(&_animations, &_vm->_char._ani._zone);
	_gfx = new Gfx(this);

	int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	MidiDriver *driver = MidiDriver::createMidi(midiDriver);
	_midiPlayer = new MidiPlayer(driver);

	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

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

	if (_location._startPosition._x != -1000) {
		_vm->_char._ani._zone.pos._position._x = _location._startPosition._x;
		_vm->_char._ani._zone.pos._position._y = _location._startPosition._y;
		_vm->_char._ani._frame = _location._startFrame;
		_location._startPosition._y = -1000;
		_location._startPosition._x = -1000;
	}

	return;
}

void Parallaction::initGlobals() {

	initTable("global.tab", _globalTable);
}

// FIXME: the engine has 3 event loops. The following routine hosts the main one,
// and it's called from 8 different places in the code. There exist 2 more specialised
// loops which could possibly be merged into this one with some effort in changing
// caller code, i.e. adding condition checks.
//
uint16 Parallaction::updateInput() {

	OSystem::Event e;
	uint16 KeyDown = 0;

	_mouseButtons = kMouseNone;

	Common::EventManager *eventMan = _system->getEventManager();
	while (eventMan->pollEvent(e)) {

		switch (e.type) {
		case OSystem::EVENT_KEYDOWN:
			if (e.kbd.ascii == 'l') KeyDown = kEvLoadGame;
			if (e.kbd.ascii == 's') KeyDown = kEvSaveGame;
			break;

		case OSystem::EVENT_LBUTTONDOWN:
			_mouseButtons = kMouseLeftDown;
			break;

		case OSystem::EVENT_LBUTTONUP:
			_mouseButtons = kMouseLeftUp;
			break;

		case OSystem::EVENT_RBUTTONDOWN:
			_mouseButtons = kMouseRightDown;
			break;

		case OSystem::EVENT_RBUTTONUP:
			_mouseButtons = kMouseRightUp;
			break;

		case OSystem::EVENT_MOUSEMOVE:
			_mousePos._x = e.mouse.x;
			_mousePos._y = e.mouse.y;
			break;

		case OSystem::EVENT_QUIT:
			_system->quit();
			break;

		default:
			break;

		}

	}

	return KeyDown;

}

// FIXME: see comment for updateInput()
void waitUntilLeftClick() {

	OSystem::Event e;

	Common::EventManager *eventMan = g_system->getEventManager();
	for (;;) {
		eventMan->pollEvent(e);

		if (e.type == OSystem::EVENT_LBUTTONUP)
			break;

		if (e.type == OSystem::EVENT_QUIT) {
			g_system->quit();
			break;
		}

		g_system->delayMillis(10);
	}


	return;
}


void Parallaction::runGame() {

	addJob(jobEraseAnimations, (void*)1, kPriority20);
	_jRunScripts = addJob(jobRunScripts, 0, kPriority15);
	addJob(jobDisplayAnimations, 0, kPriority3);

	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBit2);

	if (_location._commands)
		runCommands(_location._commands);

	runJobs();

	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

	if (_location._comment)
		doLocationEnterTransition();

	changeCursor(kCursorArrow);

	if (_location._aCommands)
		runCommands(_location._aCommands);

	while ((_engineFlags & kEngineQuit) == 0) {
		_keyDown = updateInput();

		debugC(3, kDebugInput, "runGame: input flags (%i, %i, %i, %i)",
			_mouseHidden == 0,
			(_engineFlags & kEngineMouse) == 0,
			(_engineFlags & kEngineWalking) == 0,
			(_engineFlags & kEngineChangeLocation) == 0
		);

		// WORKAROUND: the engine doesn't check for displayed labels before performing a location
		// switch, thus crashing whenever a jobDisplayLabel/jEraseLabel pair is left into the
		// queue after the character enters a door.
		// Skipping input processing when kEngineChangeLocation is set solves the issue. It's
		// noteworthy that the programmers added this very check in Big Red Adventure's engine,
		// so it should be ok here in Nippon Safes too.
		if ((_mouseHidden == 0) && ((_engineFlags & kEngineMouse) == 0) && ((_engineFlags & kEngineWalking) == 0) && ((_engineFlags & kEngineChangeLocation) == 0)) {
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
			byte palette[PALETTE_SIZE];
			memcpy(palette, _vm->_gfx->_palette, sizeof(palette));
			_gfx->animatePalette(palette);
			_gfx->setPalette(palette);
		}

	}

	delete _menu;

	return;
}


void Parallaction::processInput(InputData *data) {
	Zone *z;
	WalkNode *v4;

	switch (data->_event) {
	case kEvEnterZone:
		debugC(2, kDebugInput, "processInput: kEvEnterZone");
		_gfx->_labelPosition[1]._x = -1000;
		_gfx->_labelPosition[1]._y = -1000;
		_gfx->_labelPosition[0]._x = -1000;
		_gfx->_labelPosition[0]._y = -1000;
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
		if (hitZone(kZoneYou, _mousePos._x, _mousePos._y) == 0)
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

	case kEvWalk:
		debugC(2, kDebugInput, "processInput: kEvWalk");
		_hoverZone = NULL;
		changeCursor(kCursorArrow);
		if (_vm->_char._ani._zone._flags & kFlagsRemove) break;
		if ((_vm->_char._ani._zone._flags & kFlagsActive) == 0) break;
		v4 = buildWalkPath(data->_mousePos._x, data->_mousePos._y);
		addJob(jobWalk, v4, kPriority19);
		_engineFlags |= kEngineWalking; 								   // inhibits processing of input until walking is over
		break;

	case kEvQuitGame:
		_engineFlags |= kEngineQuit;
		break;

	case kEvSaveGame:
		_hoverZone = NULL;
		changeCursor(kCursorArrow);
		saveGame();
		break;

	case kEvLoadGame:
		_hoverZone = NULL;
		changeCursor(kCursorArrow);
		loadGame();
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

	_input._mousePos._x = _mousePos._x;
	_input._mousePos._y = _mousePos._y;

	if (((_engineFlags & kEnginePauseJobs) == 0) && ((_engineFlags & kEngineInventory) == 0)) {

		if (_actionAfterWalk == true) {
			// if walking is over, then take programmed action
			_input._event = kEvAction;
			_actionAfterWalk = false;
			return &_input;
		}

		Zone *z = hitZone(_activeItem._id, _mousePos._x, _mousePos._y);

		if (_mouseButtons == kMouseRightDown) {
			// right button down shows inventory

			if (hitZone(kZoneYou, _mousePos._x, _mousePos._y) && (_activeItem._id != 0)) {
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
				if (z->_moveTo._y != 0) {
					_input._mousePos._x = z->_moveTo._x;
					_input._mousePos._y = z->_moveTo._y;
				}
			}

			beep();
			changeCursor(kCursorArrow);
			return &_input;
		}

	}

	if ((_engineFlags & kEngineInventory) == 0) return NULL;

	// in inventory
	int16 _si = getHoverInventoryItem(_mousePos._x, _mousePos._y);

	if (_mouseButtons == kMouseRightUp) {
		// right up hides inventory

		_input._event = kEvCloseInventory;
		_input._inventoryIndex = getHoverInventoryItem(_mousePos._x, _mousePos._y);
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



void freeCharacter() {

	_vm->_gfx->freeCnv(&_vm->_char._normalFrames);

	if (!IS_DUMMY_CHARACTER(_vm->_characterName)) {
		_vm->_gfx->freeCnv(&_vm->_char._miniFrames);
		_vm->freeTable(_objectsNames);
		_vm->_gfx->freeCnv(&_vm->_char._talk);
		_vm->_gfx->freeStaticCnv(&_vm->_char._head);
		_vm->_gfx->freeCnv(&_vm->_char._objs);
	}

	return;
}

void Parallaction::selectCharacterMusic(const char *name) {
	if (IS_MINI_CHARACTER(name))
		name+=4;

	if (!scumm_stricmp(name, _dinoName)) {
		_midiPlayer->play("dino");
	} else if (!scumm_stricmp(name, _donnaName)) {
		_midiPlayer->play("donna");
	} else {
		_midiPlayer->play("nuts");
	}

	return;
}

void Parallaction::pickMusic(const char *location) {
	if (_musicData1 != 0) {
		selectCharacterMusic(_vm->_characterName);
		_musicData1 = 0;
		debugC(2, kDebugLocation, "changeLocation: started character specific music");
	}

	if (!scumm_stricmp(location, "night") || !scumm_stricmp(location, "intsushi")) {
		_vm->_midiPlayer->play("soft");

		debugC(2, kDebugLocation, "changeLocation: started music 'soft'");
	}

	if (!scumm_stricmp(location, "museo") ||
		!scumm_stricmp(location, "caveau") ||
		!scumm_strnicmp(location, "plaza1", 6) ||
		!scumm_stricmp(location, "estgrotta") ||
		!scumm_stricmp(location, "intgrottadopo") ||
		!scumm_stricmp(location, "endtgz") ||
		!scumm_stricmp(location, "common")) {

		_vm->_midiPlayer->stop();
		_musicData1 = 1;

		debugC(2, kDebugLocation, "changeLocation: music stopped");
	}
}

void Parallaction::changeCharacter(const char *name) {

	bool miniCharacter = false;

	if (IS_MINI_CHARACTER(name)) {
		name+=4;
		miniCharacter = true;
	}

	char v32[20];
	strcpy(v32, name);

	if (_engineFlags & kEngineTransformedDonna)
		strcat(v32, "tras");

	if (scumm_stricmp(v32, _characterName1)) {

		if (scumm_stricmp(_characterName1, "null"))
			freeCharacter();

		_disk->selectArchive("disk1");

		char path[PATH_LEN];
		strcpy(path, v32);
		_disk->loadFrames(path, &_vm->_char._normalFrames);

		if (!IS_DUMMY_CHARACTER(name)) {
			_disk->loadHead(path, &_vm->_char._head);
			_disk->loadTalk(path, &_vm->_char._talk);
			_disk->loadObjects(name, &_vm->_char._objs);

			sprintf(path, "mini%s", v32);
			_disk->loadFrames(path, &_vm->_char._miniFrames);

			sprintf(path, "%s.tab", name);
			initTable(path, _objectsNames);

			refreshInventory(name);

			if (scumm_stricmp(name, "night") && scumm_stricmp(name, "intsushi"))
				selectCharacterMusic(name);
		}
	}

	if (miniCharacter)
		memcpy(&_vm->_char._ani._cnv, &_vm->_char._miniFrames, sizeof(Cnv));
	else
		memcpy(&_vm->_char._ani._cnv, &_vm->_char._normalFrames, sizeof(Cnv));

	strcpy(_characterName1, v32);

	return;
}

void freeNodeList(Node *list) {

	while (list) {
		Node *v4 = list->_next;
		free(list);
		list = v4;
	}

	return;
}


void addNode(Node *list, Node *n) {

	Node *v4 = list->_next;

	if (v4 != NULL) {
		v4->_prev = n;
	}

	n->_next = v4;
	list->_next = n;
	n->_prev = list;

	return;
}

void removeNode(Node *n) {

	Node *v4 = n->_next;
	if (v4 != NULL) {
		v4->_prev = n->_prev;
	}

	n->_prev->_next = n->_next;

	return;
}


Job *Parallaction::addJob(JobFn fn, void *parm, uint16 tag) {

	Job *v8 = (Job*)malloc(sizeof(Job));

	v8->_parm = parm;
	v8->_fn = fn;
	v8->_tag = tag;
	v8->_finished = 0;
	v8->_count = 0;

	Job *v4 = &_jobs;

	while (v4->_next && ((Job*)(v4->_next))->_tag > tag) {
		v4 = (Job*)v4->_next;
	}

	addNode(v4, v8);
	return v8;
}

void Parallaction::removeJob(Job *j) {

	removeNode(j);
	free(j);
	return;
}

void Parallaction::pauseJobs() {
	_engineFlags |= kEnginePauseJobs;
	return;
}

void Parallaction::resumeJobs() {
	_engineFlags &= ~kEnginePauseJobs;
	return;
}

void Parallaction::runJobs() {

	if (_engineFlags & kEnginePauseJobs) return;

	Job *j = (Job*)_jobs._next;
	while (j) {
		debugC(3, kDebugJobs, "runJobs: %i", j->_tag);

		(*j->_fn)(j->_parm, j);
		Job *v4 = (Job*)j->_next;

		if (j->_finished == 1)
			removeJob(j);

		j = v4;
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

	_engineFlags |= kEngineMouse;

	count++;
	if (count == 2) {
		count = 0;
		_vm->removeJob(arg);
		_engineFlags &= ~kEngineMouse;
		j->_finished = 1;
	}

	return;
}


void Parallaction::initTable(const char *path, char** table) {
//	printf("initTable(%s)\n", path);

	Common::File	stream;

	if (!stream.open(path))
		errorFileNotFound(path);

	uint16 count = 0;

	fillBuffers(stream);

	while (scumm_stricmp(_tokens[0], "ENDTABLE")) {

		table[count] = (char*)malloc(strlen(_tokens[0])+1);
		strcpy(table[count], _tokens[0]);

		count++;
		fillBuffers(stream);
	}

	table[count] = NULL;

	stream.close();

	return;

}

void Parallaction::freeTable(char** table) {

	uint16 count = 0;

	while (table[count]) {

		free(table[count]);
		table[count] = NULL;

		count++;
	}

	return;

}

int16 Parallaction::searchTable(const char *s, const char **table) {

	int16 count = 0;

	if (!s) return 0;

	while (table[count]) {
		if (!scumm_stricmp(table[count], s)) return count + 1;
		count++;
	}

	return -1;
}


} // namespace Parallaction
