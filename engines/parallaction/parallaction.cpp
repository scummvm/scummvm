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


#include "parallaction/input.h"
#include "parallaction/parallaction.h"
#include "parallaction/debug.h"
#include "parallaction/sound.h"



namespace Parallaction {

// FIXME: remove this
Parallaction *_vm = NULL;

// public stuff

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
	delete _globalTable;
	delete _callableNames;
	delete _cmdExec;
	delete _programExec;

	_gfx->clearGfxObjects(kGfxObjCharacter | kGfxObjNormal);
	hideDialogueStuff();
	delete _balloonMan;
	freeLocation();

	freeCharacter();
	destroyInventory();

	cleanupGui();

	delete _localFlagNames;
	delete _gfx;
	delete _soundMan;
	delete _disk;
	delete _input;
}


int Parallaction::init() {

	_engineFlags = 0;
	_objectsNames = NULL;
	_globalTable = NULL;
	_location._hasSound = false;
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

	_input = new Input(this);

	_gfx = new Gfx(this);

	_debugger = new Debugger(this);

	_menuHelper = 0;

	setupBalloonManager();

	return 0;
}


void Parallaction::clearSet(OpcodeSet &opcodes) {
	for (Common::Array<const Opcode*>::iterator i = opcodes.begin(); i != opcodes.end(); ++i)
		delete *i;
	opcodes.clear();
}


void Parallaction::updateView() {

	if ((_engineFlags & kEnginePauseJobs) && (_input->_inputMode != Input::kInputModeInventory)) {
		return;
	}

	_gfx->animatePalette();
	_gfx->updateScreen();
	g_system->delayMillis(30);
}


void Parallaction::hideDialogueStuff() {
	_gfx->freeItems();
	_balloonMan->freeBalloons();
}


void Parallaction::freeCharacter() {
	debugC(1, kDebugExec, "freeCharacter()");

	delete _objectsNames;
	_objectsNames = 0;

	_gfx->clearGfxObjects(kGfxObjCharacter);

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
	for (AnimationList::iterator it = _location._animations.begin(); it != _location._animations.end(); it++) {
		(*it)->_commands.clear();	// See comment for freeZones(), about circular references.
	}
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

	_location._walkPoints.clear();

	_gfx->clearGfxObjects(kGfxObjNormal);
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
	_balloonMan->setLocationBalloon(const_cast<char*>(text), end);
}


void Parallaction::processInput(InputData *data) {
	if (!data) {
		return;
	}

	switch (data->_event) {
	case kEvSaveGame:
		_input->stopHovering();
		saveGame();
		setArrowCursor();
		break;

	case kEvLoadGame:
		_input->stopHovering();
		loadGame();
		setArrowCursor();
		break;

	}

	return;
}

void Parallaction::runGame() {

	InputData *data = _input->updateInput();
	if (_engineFlags & kEngineQuit)
		return;

	runGuiFrame();
	runDialogueFrame();
	runCommentFrame();

	if (_input->_inputMode == Input::kInputModeGame) {
		processInput(data);
		runPendingZones();

		if (_engineFlags & kEngineQuit)
			return;

		if (_engineFlags & kEngineChangeLocation) {
			changeLocation(_location._name);
		}
	}

	_gfx->beginFrame();

	if (_input->_inputMode == Input::kInputModeGame) {
		_programExec->runScripts(_location._programs.begin(), _location._programs.end());
		_char._ani->_z = _char._ani->height() + _char._ani->_top;
		if (_char._ani->gfxobj) {
			_char._ani->gfxobj->z = _char._ani->_z;
		}
		walk(_char);
		drawAnimations();
	}

	// change this to endFrame?
	updateView();
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

	_programExec->runScripts(_location._programs.begin(), _location._programs.end());
	drawAnimations();
	showLocationComment(_location._comment, false);
	_gfx->updateScreen();

	_input->waitForButtonEvent(kMouseLeftUp);
	_balloonMan->freeBalloons();

	// fades maximum intensity palette towards approximation of main palette
	for (uint16 _si = 0; _si<6; _si++) {
		pal.fadeTo(_gfx->_palette, 4);
		_gfx->setPalette(pal);
		_gfx->updateScreen();
		g_system->delayMillis(20);
	}

	_gfx->setPalette(_gfx->_palette);

	debugC(2, kDebugExec, "doLocationEnterTransition completed");

	return;
}

void Parallaction::setLocationFlags(uint32 flags) {
	_localFlags[_currentLocationIndex] |= flags;
}

void Parallaction::clearLocationFlags(uint32 flags) {
	_localFlags[_currentLocationIndex] &= ~flags;
}

void Parallaction::toggleLocationFlags(uint32 flags) {
	_localFlags[_currentLocationIndex] ^= flags;
}

uint32 Parallaction::getLocationFlags() {
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
			(*it)->_commands.clear();	// Since commands may reference zones, and both commands and zones are kept stored into
										// SharedPtr's, we need to kill commands explicitly to destroy any potential circular
										// reference.
			it = _location._zones.erase(it);
		}
	}

	return;
}


enum {
	WALK_LEFT = 0,
	WALK_RIGHT = 1,
	WALK_DOWN = 2,
	WALK_UP = 3
};

struct WalkFrames {
	int16 stillFrame[4];
	int16 firstWalkFrame[4];
	int16 numWalkFrames[4];
	int16 frameRepeat[4];
};

WalkFrames _char20WalkFrames = {
	{  0,  7, 14, 17 },
	{  1,  8, 15, 18 },
	{  6,  6,  2,  2 },
	{  2,  2,  4,  4 }
};

WalkFrames _char24WalkFrames = {
	{  0,  9, 18, 21 },
	{  1, 10, 19, 22 },
	{  8,  8,  2,  2 },
	{  2,  2,  4,  4 }
};

const char Character::_prefixMini[] = "mini";
const char Character::_suffixTras[] = "tras";
const char Character::_empty[] = "\0";


Character::Character(Parallaction *vm) : _vm(vm), _ani(new Animation) {
	_talk = NULL;
	_head = NULL;
	_objs = NULL;

	_direction = WALK_DOWN;
	_step = 0;

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

	// TODO: move creation into Parallaction. Needs to make Character a pointer first.
	if (_vm->getGameType() == GType_Nippon)
		_builder = new PathBuilder_NS(this);
	else
		_builder = new PathBuilder_BR(this);
}

Character::~Character() {
	delete _builder;
	_builder = 0;

	free();
}

void Character::getFoot(Common::Point &foot) {
	Common::Rect rect;
	_ani->gfxobj->getRect(_ani->_frame, rect);

	foot.x = _ani->_left + (rect.left + rect.width() / 2);
	foot.y = _ani->_top + (rect.top + rect.height());
}

void Character::setFoot(const Common::Point &foot) {
	Common::Rect rect;
	_ani->gfxobj->getRect(_ani->_frame, rect);

	_ani->_left = foot.x - (rect.left + rect.width() / 2);
	_ani->_top = foot.y - (rect.top + rect.height());
}

#if 0
void dumpPath(const PointList &list, const char* text) {
	for (PointList::iterator it = list.begin(); it != list.end(); it++)
		printf("node (%i, %i)\n", it->x, it->y);

	return;
}
#endif

void Character::scheduleWalk(int16 x, int16 y) {
	if ((_ani->_flags & kFlagsRemove) || (_ani->_flags & kFlagsActive) == 0) {
		return;
	}

	_builder->buildPath(x, y);
#if 0
	dumpPath(_walkPath, _name);
#endif

	if (_vm->getGameType() == GType_Nippon) {
		_engineFlags |= kEngineWalking;
	} else {
		// BRA can't walk yet!
		_walkPath.clear();
	}
}

void Character::free() {

	delete _talk;
	delete _head;
	delete _objs;
	delete _ani->gfxobj;

	_talk = NULL;
	_head = NULL;
	_objs = NULL;
	_ani->gfxobj = NULL;

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
	_suffix = _empty;

	_dummy = IS_DUMMY_CHARACTER(name);

	if (!_dummy) {
		if (!strstr(name, "donna")) {
			_engineFlags &= ~kEngineTransformedDonna;
		} else
		if (_engineFlags & kEngineTransformedDonna) {
			_suffix = _suffixTras;
		} else {
			const char *s = strstr(name, "tras");
			if (s) {
				_engineFlags |= kEngineTransformedDonna;
				_suffix = _suffixTras;
				end = s;
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
	debugC(9, kDebugExec, "scheduleLocationSwitch(%s)\n", location);
	strcpy(_location._name, location);
	_engineFlags |= kEngineChangeLocation;
}





void Character::updateDirection(const Common::Point& pos, const Common::Point& to) {

	Common::Point dist(to.x - pos.x, to.y - pos.y);
	WalkFrames *frames = (_ani->getFrameNum() == 20) ? &_char20WalkFrames : &_char24WalkFrames;

	_step++;

	if (dist.x == 0 && dist.y == 0) {
		_ani->_frame = frames->stillFrame[_direction];
		return;
	}

	if (dist.x < 0)
		dist.x = -dist.x;
	if (dist.y < 0)
		dist.y = -dist.y;

	_direction = (dist.x > dist.y) ? ((to.x > pos.x) ? WALK_LEFT : WALK_RIGHT) : ((to.y > pos.y) ? WALK_DOWN : WALK_UP);
	_ani->_frame = frames->firstWalkFrame[_direction] + (_step / frames->frameRepeat[_direction]) % frames->numWalkFrames[_direction];
}


} // namespace Parallaction
