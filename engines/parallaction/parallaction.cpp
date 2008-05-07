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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/util.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "parallaction/parallaction.h"
#include "parallaction/debug.h"
#include "parallaction/sound.h"



namespace Parallaction {

// FIXME: remove this
Parallaction *_vm = NULL;

// public stuff

uint16	_mouseButtons = 0;


char		_saveData1[30] = { '\0' };
uint16		_language = 0;
uint32		_engineFlags = 0;

uint16		_score = 1;
char		_password[8];

uint32		_commandFlags = 0;

// private stuff


Parallaction::Parallaction(OSystem *syst, const PARALLACTIONGameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc), _char(this) {

	// FIXME: Fingolfin asks: why is there a FIXME here? Please either clarify what
	// needs fixing, or remove it!
	// FIXME
	_vm = this;

	_mouseHidden = false;

	Common::File::addDefaultDirectory( _gameDataPath );

	Common::addSpecialDebugLevel(kDebugDialogue, "dialogue", "Dialogues debug level");
	Common::addSpecialDebugLevel(kDebugParser, "parser", "Parser debug level");
	Common::addSpecialDebugLevel(kDebugDisk, "disk", "Disk debug level");
	Common::addSpecialDebugLevel(kDebugWalk, "walk", "Walk debug level");
	Common::addSpecialDebugLevel(kDebugGraphics, "gfx", "Gfx debug level");
	Common::addSpecialDebugLevel(kDebugExec, "exec", "Execution debug level");
	Common::addSpecialDebugLevel(kDebugInput, "input", "Input debug level");
	Common::addSpecialDebugLevel(kDebugAudio, "audio", "Audio debug level");
	Common::addSpecialDebugLevel(kDebugMenu, "menu", "Menu debug level");
	Common::addSpecialDebugLevel(kDebugInventory, "inventory", "Inventory debug level");

	syst->getEventManager()->registerRandomSource(_rnd, "parallaction");
}


Parallaction::~Parallaction() {
	delete _debugger;

	delete _locationParser;
	delete _globalTable;

	delete _callableNames;
	delete _localFlagNames;
	delete _zoneTypeNames;
	delete _zoneFlagNames;

	freeLocation();

	freeCharacter();
	destroyInventory();

	delete _gfx;
	delete _soundMan;
	delete _disk;
}


int Parallaction::init() {

	_engineFlags = 0;
	_objectsNames = NULL;
	_globalTable = NULL;
	_location._hasSound = false;
	_transCurrentHoverItem = 0;
	_actionAfterWalk = false;  // actived when the character needs to move before taking an action
	_activeItem._index = 0;
	_activeItem._id = 0;
	_procCurrentHoverItem = -1;
	_baseTime = 0;
	_numLocations = 0;
	_location._startPosition.x = -1000;
	_location._startPosition.y = -1000;
	_location._startFrame = 0;
	_location._comment = NULL;
	_location._endComment = NULL;

	_pathBuffer = 0;

	_screenSize = _screenWidth * _screenHeight;

	strcpy(_characterName1, "null");

	memset(_locationNames, 0, NUM_LOCATIONS * 32);

	initInventory();	// needs to be pushed into subclass

	_gfx = new Gfx(this);

	_debugger = new Debugger(this);

	return 0;
}







// FIXME: the engine has 3 event loops. The following routine hosts the main one,
// and it's called from 8 different places in the code. There exist 2 more specialised
// loops which could possibly be merged into this one with some effort in changing
// caller code, i.e. adding condition checks.
//
uint16 Parallaction::readInput() {

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
			_mousePos = e.mouse;
			break;

		case Common::EVENT_LBUTTONUP:
			_mouseButtons = kMouseLeftUp;
			_mousePos = e.mouse;
			break;

		case Common::EVENT_RBUTTONDOWN:
			_mouseButtons = kMouseRightDown;
			_mousePos = e.mouse;
			break;

		case Common::EVENT_RBUTTONUP:
			_mouseButtons = kMouseRightUp;
			_mousePos = e.mouse;
			break;

		case Common::EVENT_MOUSEMOVE:
			_mousePos = e.mouse;
			break;

		case Common::EVENT_QUIT:
			// TODO: don't quit() here, just have caller routines to check
			// on kEngineQuit and exit gracefully to allow the engine to shut down
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

// FIXME: see comment for readInput()
void waitUntilLeftClick() {

	do {
		_vm->readInput();
		_vm->_gfx->updateScreen();
		g_system->delayMillis(30);
	} while (_mouseButtons != kMouseLeftUp);

	return;
}

void Parallaction::runGame() {

	updateInput();

	runPendingZones();

	if (_engineFlags & kEngineChangeLocation) {
		changeLocation(_location._name);
	}


	_gfx->beginFrame();

	if (_inputMode == kInputModeGame) {
		runScripts();
		walk();
		drawAnimations();
	}

	// change this to endFrame?
	updateView();

}

void Parallaction::updateView() {

	if ((_engineFlags & kEnginePauseJobs) && (_engineFlags & kEngineInventory) == 0) {
		return;
	}

	_gfx->animatePalette();
	_gfx->updateScreen();
	g_system->delayMillis(30);
}


void Parallaction::processInput(InputData *data) {

	switch (data->_event) {
	case kEvEnterZone:
		debugC(2, kDebugInput, "processInput: kEvEnterZone");
		_gfx->setFloatingLabel(data->_label);
		break;

	case kEvExitZone:
		debugC(2, kDebugInput, "processInput: kEvExitZone");
		_gfx->setFloatingLabel(0);
		break;

	case kEvAction:
		debugC(2, kDebugInput, "processInput: kEvAction");
		_procCurrentHoverItem = -1;
		_hoverZone = nullZonePtr;
		pauseJobs();
		runZone(data->_zone);
		resumeJobs();
		break;

	case kEvOpenInventory:
		_procCurrentHoverItem = -1;
		_hoverZone = nullZonePtr;
		_gfx->setFloatingLabel(0);
		if (hitZone(kZoneYou, _mousePos.x, _mousePos.y) == 0) {
			setArrowCursor();
		}
		pauseJobs();
		openInventory();
		break;

	case kEvCloseInventory: // closes inventory and possibly select item
		closeInventory();
		setInventoryCursor(data->_inventoryIndex);
		resumeJobs();
		break;

	case kEvHoverInventory:
		highlightInventoryItem(_procCurrentHoverItem, 12);	// disable
		highlightInventoryItem(data->_inventoryIndex, 19);						// enable
		_procCurrentHoverItem = data->_inventoryIndex;
		break;

	case kEvWalk:
		debugC(2, kDebugInput, "processInput: kEvWalk");
		_hoverZone = nullZonePtr;
		setArrowCursor();
		_char.scheduleWalk(data->_mousePos.x, data->_mousePos.y);
		break;

	case kEvQuitGame:
		_engineFlags |= kEngineQuit;
		break;

	case kEvSaveGame:
		_hoverZone = nullZonePtr;
		saveGame();
		setArrowCursor();
		break;

	case kEvLoadGame:
		_hoverZone = nullZonePtr;
		loadGame();
		setArrowCursor();
		break;

	}

	return;
}





void Parallaction::updateGameInput() {

	int16 keyDown = readInput();

	debugC(3, kDebugInput, "translateInput: input flags (%i, %i, %i, %i)",
		!_mouseHidden,
		(_engineFlags & kEngineBlockInput) == 0,
		(_engineFlags & kEngineWalking) == 0,
		(_engineFlags & kEngineChangeLocation) == 0
	);

	if ((_mouseHidden) ||
		(_engineFlags & kEngineBlockInput) ||
		(_engineFlags & kEngineWalking) ||
		(_engineFlags & kEngineChangeLocation)) {

		return;
	}

	if (keyDown == kEvQuitGame) {
		_input._event = kEvQuitGame;
	} else
	if (keyDown == kEvSaveGame) {
		_input._event = kEvSaveGame;
	} else
	if (keyDown == kEvLoadGame) {
		_input._event = kEvLoadGame;
	} else {
		_input._mousePos = _mousePos;
		_input._event = kEvNone;
		if (!translateGameInput()) {
			translateInventoryInput();
		}
	}

	if (_input._event != kEvNone)
		processInput(&_input);

}

void Parallaction::updateCommentInput() {
	waitUntilLeftClick();

	_gfx->hideDialogueStuff();
	_gfx->setHalfbriteMode(false);

	_inputMode = kInputModeGame;
}

void Parallaction::updateInput() {

	switch (_inputMode) {
	case kInputModeComment:
		updateCommentInput();
		break;

	case kInputModeGame:
		updateGameInput();
		break;
	}

	return;
}

bool Parallaction::translateGameInput() {

	if ((_engineFlags & kEnginePauseJobs) || (_engineFlags & kEngineInventory)) {
		return false;
	}

	if (_actionAfterWalk) {
		// if walking is over, then take programmed action
		_input._event = kEvAction;
		_actionAfterWalk = false;
		return true;
	}

	if (_mouseButtons == kMouseRightDown) {
		// right button down shows inventory

		if (hitZone(kZoneYou, _mousePos.x, _mousePos.y) && (_activeItem._id != 0)) {
			_activeItem._index = (_activeItem._id >> 16) & 0xFFFF;
			_engineFlags |= kEngineDragging;
		}

		_input._event = kEvOpenInventory;
		_transCurrentHoverItem = -1;
		return true;
	}

	// test if mouse is hovering on an interactive zone for the currently selected inventory item
	ZonePtr z = hitZone(_activeItem._id, _mousePos.x, _mousePos.y);

	if (((_mouseButtons == kMouseLeftUp) && (_activeItem._id == 0) && ((_engineFlags & kEngineWalking) == 0)) && ((!z) || ((z->_type & 0xFFFF) != kZoneCommand))) {
		_input._event = kEvWalk;
		return true;
	}

	if ((z != _hoverZone) && (_hoverZone)) {
		_hoverZone = nullZonePtr;
		_input._event = kEvExitZone;
		return true;
	}

	if (!z) {
		_input._event = kEvNone;
		return true;
	}

	if ((!_hoverZone) && ((z->_flags & kFlagsNoName) == 0)) {
		_hoverZone = z;
		_input._event = kEvEnterZone;
		_input._label = z->_label;
		return true;
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

		beep();
		setArrowCursor();
		return true;
	}

	return true;

}

bool Parallaction::translateInventoryInput() {

	if ((_engineFlags & kEngineInventory) == 0) {
		return false;
	}

	// in inventory
	int16 _si = getHoverInventoryItem(_mousePos.x, _mousePos.y);

	if (_mouseButtons == kMouseRightUp) {
		// right up hides inventory

		_input._event = kEvCloseInventory;
		_input._inventoryIndex = getHoverInventoryItem(_mousePos.x, _mousePos.y);
		highlightInventoryItem(_transCurrentHoverItem, 12);			// disable

		if ((_engineFlags & kEngineDragging) == 0) {
			return true;
		}

		_engineFlags &= ~kEngineDragging;
		ZonePtr z = hitZone(kZoneMerge, _activeItem._index, getInventoryItemIndex(_input._inventoryIndex));

		if (z) {
			dropItem(z->u.merge->_obj1);
			dropItem(z->u.merge->_obj2);
			addInventoryItem(z->u.merge->_obj3);
			runCommands(z->_commands);
		}

		return true;
	}

	if (_si == _transCurrentHoverItem) {
		_input._event = kEvNone;
		return true;
	}

	_transCurrentHoverItem = _si;
	_input._event = kEvHoverInventory;
	_input._inventoryIndex = _si;
	return true;

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



void Parallaction::freeCharacter() {
	debugC(1, kDebugExec, "freeCharacter()");

	delete _objectsNames;
	_objectsNames = 0;

	_char.free();

	return;
}


void Parallaction::pauseJobs() {
	debugC(9, kDebugExec, "pausing jobs execution");

	_engineFlags |= kEnginePauseJobs;
	return;
}

void Parallaction::resumeJobs() {
	debugC(9, kDebugExec, "resuming jobs execution");

	_engineFlags &= ~kEnginePauseJobs;
	return;
}

AnimationPtr Parallaction::findAnimation(const char *name) {

	for (AnimationList::iterator it = _location._animations.begin(); it != _location._animations.end(); it++)
		if (!scumm_stricmp((*it)->_name, name)) return *it;

	return nullAnimationPtr;
}

void Parallaction::freeAnimations() {
	_location._animations.clear();
	return;
}



void Parallaction::allocateLocationSlot(const char *name) {
	// WORKAROUND: the original code erroneously incremented
	// _currentLocationIndex, thus producing inconsistent
	// savegames. This workaround modified the following loop
	// and if-statement, so the code exactly matches the one
	// in Big Red Adventure.
	_currentLocationIndex = -1;
	uint16 _di = 0;
	while (_locationNames[_di][0] != '\0') {
		if (!scumm_stricmp(_locationNames[_di], name)) {
			_currentLocationIndex = _di;
		}
		_di++;
	}

	if (_di == 120)
		error("No more location slots available. Please report this immediately to ScummVM team.");

	if (_currentLocationIndex  == -1) {
		strcpy(_locationNames[_numLocations], name);
		_currentLocationIndex = _numLocations;

		_numLocations++;
		_locationNames[_numLocations][0] = '\0';
		_localFlags[_numLocations] = 0;
	} else {
		setLocationFlags(kFlagsVisited);	// 'visited'
	}
}



void Parallaction::freeLocation() {
	debugC(2, kDebugExec, "freeLocation");

	_soundMan->stopSfx(0);
	_soundMan->stopSfx(1);
	_soundMan->stopSfx(2);
	_soundMan->stopSfx(3);

	_localFlagNames->clear();

	_location._walkNodes.clear();

	_gfx->clearGfxObjects();
	freeBackground();

	_location._programs.clear();
	freeZones();
	freeAnimations();

	free(_location._comment);
	_location._comment = 0;

	_location._commands.clear();
	_location._aCommands.clear();

	return;
}




void Parallaction::freeBackground() {

	_gfx->freeBackground();
	_pathBuffer = 0;

}

void Parallaction::setBackground(const char* name, const char* mask, const char* path) {

	_gfx->setBackground(kBackgroundLocation, name, mask, path);
	_pathBuffer = &_gfx->_backgroundInfo.path;

	return;
}

void Parallaction::showLocationComment(const char *text, bool end) {
	_gfx->setLocationBalloon(const_cast<char*>(text), end);
}





//	displays transition before a new location
//
//	clears screen (in white??)
//	shows location comment (if any)
//	waits for mouse click
//	fades towards game palette
//
void Parallaction::doLocationEnterTransition() {
	debugC(2, kDebugExec, "doLocationEnterTransition");

	if (!_location._comment) {
		return;
	}

	if (getLocationFlags() & kFlagsVisited) {
		debugC(2, kDebugExec, "skipping location transition");
		return; // visited
	}

	Palette pal(_gfx->_palette);
	pal.makeGrayscale();
	_gfx->setPalette(pal);

	runScripts();
	drawAnimations();

	_gfx->updateScreen();

	showLocationComment(_location._comment, false);
	waitUntilLeftClick();
	_gfx->freeBalloons();

	// fades maximum intensity palette towards approximation of main palette
	for (uint16 _si = 0; _si<6; _si++) {
		pal.fadeTo(_gfx->_palette, 4);
		_gfx->setPalette(pal);
		waitTime( 1 );
		_gfx->updateScreen();
	}

	_gfx->setPalette(_gfx->_palette);

	debugC(2, kDebugExec, "doLocationEnterTransition completed");

	return;
}

void Parallaction::setLocationFlags(int32 flags) {
	_localFlags[_currentLocationIndex] |= flags;
}

void Parallaction::clearLocationFlags(uint32 flags) {
	_localFlags[_currentLocationIndex] &= ~flags;
}

void Parallaction::toggleLocationFlags(int32 flags) {
	_localFlags[_currentLocationIndex] ^= flags;
}

int32 Parallaction::getLocationFlags() {
	return _localFlags[_currentLocationIndex];
}




ZonePtr Parallaction::findZone(const char *name) {

	for (ZoneList::iterator it = _location._zones.begin(); it != _location._zones.end(); it++) {
		if (!scumm_stricmp((*it)->_name, name)) return *it;
	}

	return findAnimation(name);
}


void Parallaction::freeZones() {
	debugC(2, kDebugExec, "freeZones: kEngineQuit = %i", _engineFlags & kEngineQuit);

	ZoneList::iterator it = _location._zones.begin();

	while ( it != _location._zones.end() ) {

		// NOTE : this condition has been relaxed compared to the original, to allow the engine
		// to retain special - needed - zones that were lost across location switches.
		ZonePtr z = *it;
		if (((z->_top == -1) || (z->_left == -2)) && ((_engineFlags & kEngineQuit) == 0)) {
			debugC(2, kDebugExec, "freeZones preserving zone '%s'", z->_name);
			it++;
		} else {
			it = _location._zones.erase(it);
		}
	}

	return;
}


const char Character::_prefixMini[] = "mini";
const char Character::_suffixTras[] = "tras";
const char Character::_empty[] = "\0";


Character::Character(Parallaction *vm) : _vm(vm), _ani(new Animation), _builder(_ani) {
	_talk = NULL;
	_head = NULL;
	_objs = NULL;

	_dummy = false;

	_ani->_left = 150;
	_ani->_top = 100;
	_ani->_z = 10;
	_ani->_oldPos.x = -1000;
	_ani->_oldPos.y = -1000;
	_ani->_frame = 0;
	_ani->_flags = kFlagsActive | kFlagsNoName;
	_ani->_type = kZoneYou;
	strncpy(_ani->_name, "yourself", ZONENAME_LENGTH);
}

void Character::getFoot(Common::Point &foot) {
	foot.x = _ani->_left + _ani->width() / 2;
	foot.y = _ani->_top + _ani->height();
}

void Character::setFoot(const Common::Point &foot) {
	_ani->_left = foot.x - _ani->width() / 2;
	_ani->_top = foot.y - _ani->height();
}

void Character::scheduleWalk(int16 x, int16 y) {
	if ((_ani->_flags & kFlagsRemove) || (_ani->_flags & kFlagsActive) == 0) {
		return;
	}

	_walkPath = _builder.buildPath(x, y);
	_engineFlags |= kEngineWalking;
}

void Character::free() {

	delete _talk;
	delete _head;
	delete _objs;

	_ani->gfxobj = NULL;
	_talk = NULL;
	_head = NULL;
	_objs = NULL;

	return;
}


// Various ways of detecting character modes used to exist
// inside the engine, so they have been unified in the two
// following macros.
// Mini characters are those used in far away shots, like
// the highway scenery, while Dummy characters are a mere
// workaround to keep the engine happy when showing slides.
// As a sidenote, standard sized characters' names start
// with a lowercase 'd'.
#define IS_MINI_CHARACTER(s) (((s)[0] == 'm'))
#define IS_DUMMY_CHARACTER(s) (((s)[0] == 'D'))

void Character::setName(const char *name) {
	const char *begin = name;
	const char *end = begin + strlen(name);

	_prefix = _empty;

	_dummy = IS_DUMMY_CHARACTER(name);

	if (!_dummy) {
		if (_engineFlags & kEngineTransformedDonna) {
			_suffix = _suffixTras;
		} else {
			const char *s = strstr(name, "tras");
			if (s) {
				_engineFlags |= kEngineTransformedDonna;
				_suffix = _suffixTras;
				end = s;
			} else {
				_suffix = _empty;
			}
		}
		if (IS_MINI_CHARACTER(name)) {
			_prefix = _prefixMini;
			begin = name+4;
		}
	}

	memset(_baseName, 0, 30);
	strncpy(_baseName, begin, end - begin);
	sprintf(_name, "%s%s", _prefix, _baseName);
	sprintf(_fullName, "%s%s%s", _prefix, _baseName, _suffix);
}

const char *Character::getName() const {
	return _name;
}

const char *Character::getBaseName() const {
	return _baseName;
}

const char *Character::getFullName() const {
	return _fullName;
}

bool Character::dummy() const {
	return _dummy;
}

void Parallaction::beep() {
	_soundMan->playSfx("beep", 3, false);
}

void Parallaction::scheduleLocationSwitch(const char *location) {
	strcpy(_location._name, location);
	_engineFlags |= kEngineChangeLocation;
}


} // namespace Parallaction
