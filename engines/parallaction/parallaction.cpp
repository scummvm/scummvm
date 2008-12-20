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
#include "parallaction/saveload.h"
#include "parallaction/sound.h"



namespace Parallaction {

Parallaction *_vm = NULL;
// public stuff

char		_saveData1[30] = { '\0' };
uint32		_engineFlags = 0;

uint16		_score = 1;
char		_password[8];

uint32		_globalFlags = 0;

// private stuff


Parallaction::Parallaction(OSystem *syst, const PARALLACTIONGameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc), _char(this) {

	_vm = this;
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
	delete _globalFlagsNames;
	delete _callableNames;
	delete _cmdExec;
	delete _programExec;
	delete _saveLoad;

	_gfx->clearGfxObjects(kGfxObjCharacter | kGfxObjNormal);
	hideDialogueStuff();
	delete _balloonMan;

	freeCharacter();
	destroyInventory();

	cleanupGui();

	delete _localFlagNames;
	delete _gfx;
	delete _soundMan;
	delete _disk;
	delete _input;
}


Common::Error Parallaction::init() {

	_engineFlags = 0;
	_objectsNames = NULL;
	_globalFlagsNames = NULL;
	_location._hasSound = false;
	_numLocations = 0;
	_location._startPosition.x = -1000;
	_location._startPosition.y = -1000;
	_location._startFrame = 0;

	_screenSize = _screenWidth * _screenHeight;

	strcpy(_characterName1, "null");

	memset(_locationNames, 0, NUM_LOCATIONS * 32);

	initInventory();	// needs to be pushed into subclass

	// this needs _disk to be already setup
	_input = new Input(this);

	_gfx = new Gfx(this);

	_debugger = new Debugger(this);

	_menuHelper = 0;

	setupBalloonManager();

	return Common::kNoError;
}

bool canScroll() {
	return (_vm->_gfx->_backgroundInfo->width > _vm->_screenWidth);
}

void Parallaction::updateView() {

	if ((_engineFlags & kEnginePauseJobs) && (_input->_inputMode != Input::kInputModeInventory)) {
		return;
	}

	#define SCROLL_BAND_WIDTH		120

	if (canScroll()) {
		int scrollX = _gfx->getScrollPos();

		Common::Point foot;
		_char.getFoot(foot);

		foot.x -= scrollX;
		//foot.y -= ...

		int min = SCROLL_BAND_WIDTH;
		int max = _screenWidth - SCROLL_BAND_WIDTH;

		if (foot.x < min) {
			scrollX -= (min - foot.x);
		} else
		if (foot.x > max) {
			scrollX += (foot.x - max);
		}

		_gfx->setScrollPos(scrollX);
	}

	_gfx->animatePalette();
	_gfx->updateScreen();
	_system->delayMillis(30);
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

AnimationPtr Location::findAnimation(const char *name) {

	for (AnimationList::iterator it = _animations.begin(); it != _animations.end(); it++)
		if (!scumm_stricmp((*it)->_name, name)) return *it;

	return nullAnimationPtr;
}

void Location::freeAnimations() {
	for (AnimationList::iterator it = _animations.begin(); it != _animations.end(); it++) {
		(*it)->_commands.clear();	// See comment for freeZones(), about circular references.
	}
	_animations.clear();
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


Location::Location() {
	cleanup(true);
}

Location::~Location() {
	cleanup(true);
}

void Location::cleanup(bool removeAll) {
	_comment.clear();
	_endComment.clear();

	freeZones(removeAll);
	freeAnimations();

	_programs.clear();
	_commands.clear();
	_aCommands.clear();

	_hasSound = false;

	// NS specific
	_walkPoints.clear();

	// BRA specific
	_zeta0 = _zeta1 = _zeta2 = 0;
	_escapeCommands.clear();
}

int Location::getScale(int z) const {
	int scale = 100;
	if (z <= _zeta0) {
		scale = _zeta2;
		if (z >= _zeta1) {
			scale += ((z - _zeta1) * (100 - _zeta2)) / (_zeta0 - _zeta1);
		}
	}
	return scale;
}


void Parallaction::showSlide(const char *name, int x, int y) {
	BackgroundInfo *info = new BackgroundInfo;
	_disk->loadSlide(*info, name);

	info->x = (x == CENTER_LABEL_HORIZONTAL) ? ((_screenWidth - info->width) >> 1) : x;
	info->y = (y == CENTER_LABEL_VERTICAL) ? ((_screenHeight - info->height) >> 1) : y;

	_gfx->setBackground(kBackgroundSlide, info);
}


void Parallaction::setBackground(const char* name, const char* mask, const char* path) {
	BackgroundInfo *info = new BackgroundInfo;
	_disk->loadScenery(*info, name, mask, path);
	_gfx->setBackground(kBackgroundLocation, info);
}

void Parallaction::showLocationComment(const Common::String &text, bool end) {
	_balloonMan->setLocationBalloon(text.c_str(), end);
}


void Parallaction::runGameFrame(int event) {
	if (_input->_inputMode != Input::kInputModeGame) {
		return;
	}

	if (event != kEvNone) {
		_input->stopHovering();
		if (event == kEvSaveGame) {
			_saveLoad->saveGame();
		} else
		if (event == kEvLoadGame) {
			_saveLoad->loadGame();
		}
		_input->setArrowCursor();
	}

	runPendingZones();

	if (shouldQuit())
		return;

	if (_engineFlags & kEngineChangeLocation) {
		changeLocation(_location._name);
	}

	_programExec->runScripts(_location._programs.begin(), _location._programs.end());
	_char._ani->resetZ();
//	if (_char._ani->gfxobj) {
//		_char._ani->gfxobj->z = _char._ani->getZ();
//	}
	_char._walker->walk();
	drawAnimations();

}

void Parallaction::runGame() {

	int event = _input->updateInput();
	if (shouldQuit())
		return;

	runGuiFrame();
	runDialogueFrame();
	runCommentFrame();
	runGameFrame(event);

	if (shouldQuit())
		return;

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

	if (_location._comment.empty()) {
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
		_system->delayMillis(20);
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



void Parallaction::drawAnimations() {
	debugC(9, kDebugExec, "Parallaction_ns::drawAnimations()\n");

	uint16 layer = 0, scale = 100;

	for (AnimationList::iterator it = _location._animations.begin(); it != _location._animations.end(); it++) {

		AnimationPtr anim = *it;
		GfxObj *obj = anim->gfxobj;

		// Validation is performed here, so that every animation is affected, instead that only the ones
		// who *own* a script. In fact, some scripts can change values in other animations.
		// The right way to do this would be to enforce validation when any variable is modified from
		// a script.
		anim->validateScriptVars();

		if ((anim->_flags & kFlagsActive) && ((anim->_flags & kFlagsRemove) == 0))   {

			if (anim->_flags & kFlagsNoMasked) {
				layer = LAYER_FOREGROUND;
			} else {
				if (getGameType() == GType_Nippon) {
					// Layer in NS depends on where the animation is on the screen, for each animation.
					layer = _gfx->_backgroundInfo->getLayer(anim->getBottom());
				} else {
					// Layer in BRA is calculated from Z value. For characters it is the same as NS,
					// but other animations can have Z set from scripts independently from their
					// position on the screen.
					layer = _gfx->_backgroundInfo->getLayer(anim->getZ());
				}
			}

			scale = 100;
			if (getGameType() == GType_BRA) {
				if (anim->_flags & (kFlagsScaled | kFlagsCharacter)) {
					scale = _location.getScale(anim->getZ());
				}
			}

			if (obj) {
				_gfx->showGfxObj(obj, true);
				obj->frame = anim->getF();
				obj->x = anim->getX();
				obj->y = anim->getY();
				obj->z = anim->getZ();
				obj->layer = layer;
				obj->scale = scale;
			}
		}

		if (((anim->_flags & kFlagsActive) == 0) && (anim->_flags & kFlagsRemove))   {
			anim->_flags &= ~kFlagsRemove;
		}

		if ((anim->_flags & kFlagsActive) && (anim->_flags & kFlagsRemove))	{
			anim->_flags &= ~kFlagsActive;
			anim->_flags |= kFlagsRemove;
			if (obj) {
				_gfx->showGfxObj(obj, false);
			}
		}
	}

	debugC(9, kDebugExec, "Parallaction_ns::drawAnimations done()\n");

	return;
}


void Parallaction::showZone(ZonePtr z, bool visible) {
	if (!z) {
		return;
	}

	if (visible) {
		z->_flags &= ~kFlagsRemove;
		z->_flags |= kFlagsActive;
	} else {
		z->_flags |= kFlagsRemove;
	}

	if ((z->_type & 0xFFFF) == kZoneGet) {
		_gfx->showGfxObj(z->u.get->gfxobj, visible);
	}
}


//
//	ZONE TYPE: EXAMINE
//

void Parallaction::enterCommentMode(ZonePtr z) {
	if (!z) {
		return;
	}

	_commentZone = z;

	ExamineData *data = _commentZone->u.examine;

	if (data->_description.empty()) {
		return;
	}

	// TODO: move this balloons stuff into DialogueManager and BalloonManager
	if (getGameType() == GType_Nippon) {
		if (data->_filename) {
			if (data->_cnv == 0) {
				data->_cnv = _disk->loadStatic(data->_filename);
			}

			_gfx->setHalfbriteMode(true);
			_balloonMan->setSingleBalloon(data->_description.c_str(), 0, 90, 0, BalloonManager::kNormalColor);
			Common::Rect r;
			data->_cnv->getRect(0, r);
			_gfx->setItem(data->_cnv, 140, (_screenHeight - r.height())/2);
			_gfx->setItem(_char._head, 100, 152);
		} else {
			_balloonMan->setSingleBalloon(data->_description.c_str(), 140, 10, 0, BalloonManager::kNormalColor);
			_gfx->setItem(_char._talk, 190, 80);
		}
	} else
	if (getGameType() == GType_BRA) {
		_balloonMan->setSingleBalloon(data->_description.c_str(), 0, 0, 1, BalloonManager::kNormalColor);
		_gfx->setItem(_char._talk, 10, 80);
	}

	_input->_inputMode = Input::kInputModeComment;
}

void Parallaction::exitCommentMode() {
	_input->_inputMode = Input::kInputModeGame;

	hideDialogueStuff();
	_gfx->setHalfbriteMode(false);

	_cmdExec->run(_commentZone->_commands, _commentZone);
	_commentZone = nullZonePtr;
}

void Parallaction::runCommentFrame() {
	if (_input->_inputMode != Input::kInputModeComment) {
		return;
	}

	if (_input->getLastButtonEvent() == kMouseLeftUp) {
		exitCommentMode();
	}
}


void Parallaction::runZone(ZonePtr z) {
	debugC(3, kDebugExec, "runZone (%s)", z->_name);

	uint16 subtype = z->_type & 0xFFFF;

	debugC(3, kDebugExec, "type = %x, object = %x", subtype, (z->_type & 0xFFFF0000) >> 16);
	switch(subtype) {

	case kZoneExamine:
		enterCommentMode(z);
		return;

	case kZoneGet:
		pickupItem(z);
		break;

	case kZoneDoor:
		if (z->_flags & kFlagsLocked) break;
		updateDoor(z, !(z->_flags & kFlagsClosed));
		break;

	case kZoneHear:
		_soundMan->playSfx(z->u.hear->_name, z->u.hear->_channel, (z->_flags & kFlagsLooping) == kFlagsLooping, 60);
		break;

	case kZoneSpeak:
		enterDialogueMode(z);
		return;
	}

	debugC(3, kDebugExec, "runZone completed");

	_cmdExec->run(z->_commands, z);

	return;
}

//
//	ZONE TYPE: DOOR
//
void Parallaction::updateDoor(ZonePtr z, bool close) {
	z->_flags = close ? (z->_flags |= kFlagsClosed) : (z->_flags &= ~kFlagsClosed);

	if (z->u.door->gfxobj) {
		uint frame = (close ? 0 : 1);
//		z->u.door->gfxobj->setFrame(frame);
		z->u.door->gfxobj->frame = frame;
	}

	return;
}



//
//	ZONE TYPE: GET
//

bool Parallaction::pickupItem(ZonePtr z) {
	if (z->_flags & kFlagsFixed) {
		return false;
	}

	int slot = addInventoryItem(z->u.get->_icon);
	if (slot != -1) {
		showZone(z, false);
	}

	return (slot != -1);
}

// FIXME: input coordinates must be offseted to handle scrolling!
bool Parallaction::checkSpecialZoneBox(ZonePtr z, uint32 type, uint x, uint y) {
	// not a special zone
	if ((z->getX() != -2) && (z->getX() != -3)) {
		return false;
	}

	// WORKAROUND: this huge condition is needed because we made TypeData a collection of structs
	// instead of an union. So, merge->_obj1 and get->_icon were just aliases in the original engine,
	// but we need to check it separately here. The same workaround is applied in freeZones.
	if ((((z->_type & 0xFFFF) == kZoneMerge) && (((x == z->u.merge->_obj1) && (y == z->u.merge->_obj2)) || ((x == z->u.merge->_obj2) && (y == z->u.merge->_obj1)))) ||
		(((z->_type & 0xFFFF) == kZoneGet) && ((x == z->u.get->_icon) || (y == z->u.get->_icon)))) {

		// WORKAROUND for bug 2070751: special zones are only used in NS, to allow the
		// the EXAMINE/USE action to be applied on some particular item in the inventory.
		// The usage a verb requires at least an item match, so type can't be 0, as it
		// was in the original code. This bug has been here since the beginning, and was
		// hidden by label code, which filtered the bogus matches produced here.

		// look for action + item match
		if (z->_type == type)
			return true;
		// look for item match, but don't accept 0 types
		if (((z->_type & 0xFFFF0000) == type) && (type))
			return true;
	}

	return false;
}

// FIXME: input coordinates must be offseted to handle scrolling!
bool Parallaction::checkZoneBox(ZonePtr z, uint32 type, uint x, uint y) {
	if (z->_flags & kFlagsRemove)
		return false;

	debugC(5, kDebugExec, "checkZoneBox for %s (type = %x, x = %i, y = %i)", z->_name, type, x, y);

	if (!z->hitRect(x, y)) {

		// check for special zones (items defined in common.loc)
		if (checkSpecialZoneBox(z, type, x, y))
			return true;

		if (z->getX() != -1)
			return false;
		if (!_char._ani->hitFrameRect(x, y))
			return false;
	}

	// normal Zone
	if ((type == 0) && ((z->_type & 0xFFFF0000) == 0))
		return true;
	if (z->_type == type)
		return true;
	if ((z->_type & 0xFFFF0000) == type)
		return true;

	return false;
}

// FIXME: input coordinates must be offseted to handle scrolling!
bool Parallaction::checkLinkedAnimBox(ZonePtr z, uint32 type, uint x, uint y) {
	if (z->_flags & kFlagsRemove)
		return false;

	if ((z->_flags & kFlagsAnimLinked) == 0)
		return false;

	debugC(5, kDebugExec, "checkLinkedAnimBox for %s (type = %x, x = %i, y = %i)", z->_name, type, x, y);

	if (!z->_linkedAnim->hitFrameRect(x, y)) {
		return false;
	}

	// NOTE: the implementation of the following lines is a different in the
	// original... it is working so far, though
	if ((type == 0) && ((z->_type & 0xFFFF0000) == 0))
		return true;
	if (z->_type == type)
		return true;
	if ((z->_type & 0xFFFF0000) == type)
		return true;

	return false;
}

ZonePtr Parallaction::hitZone(uint32 type, uint16 x, uint16 y) {
	uint16 _di = y;
	uint16 _si = x;

	for (ZoneList::iterator it = _location._zones.begin(); it != _location._zones.end(); it++) {
		if (checkLinkedAnimBox(*it, type, x, y)) {
			return *it;
		}
		if (checkZoneBox(*it, type, x, y)) {
			return *it;
		}
	}


	int16 _a, _b, _c, _d;
	bool _ef;
	for (AnimationList::iterator ait = _location._animations.begin(); ait != _location._animations.end(); ait++) {

		AnimationPtr a = *ait;

		_a = (a->_flags & kFlagsActive) ? 1 : 0;															   // _a: active Animation
		_ef = a->hitFrameRect(_si, _di);

		_b = ((type != 0) || (a->_type == kZoneYou)) ? 0 : 1;										 // _b: (no type specified) AND (Animation is not the character)
		_c = (a->_type & 0xFFFF0000) ? 0 : 1;															// _c: Animation is not an object
		_d = ((a->_type & 0xFFFF0000) != type) ? 0 : 1;													// _d: Animation is an object of the same type

		if ((_a != 0 && _ef) && ((_b != 0 && _c != 0) || (a->_type == type) || (_d != 0))) {
			return a;
		}
	}

	return nullZonePtr;
}


ZonePtr Location::findZone(const char *name) {
	for (ZoneList::iterator it = _zones.begin(); it != _zones.end(); it++) {
		if (!scumm_stricmp((*it)->_name, name)) return *it;
	}
	return findAnimation(name);
}


void Location::freeZones(bool removeAll) {
	debugC(2, kDebugExec, "freeZones: removeAll = %i", removeAll);

	ZoneList::iterator it = _zones.begin();

	while ( it != _zones.end() ) {

		// NOTE : this condition has been relaxed compared to the original, to allow the engine
		// to retain special - needed - zones that were lost across location switches.
		ZonePtr z = *it;
		if (((z->getY() == -1) || (z->getX() == -2)) && (!removeAll)) {
			debugC(2, kDebugExec, "freeZones preserving zone '%s'", z->_name);
			it++;
		} else {
			(*it)->_commands.clear();	// Since commands may reference zones, and both commands and zones are kept stored into
										// SharedPtr's, we need to kill commands explicitly to destroy any potential circular
										// reference.
			it = _zones.erase(it);
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



Character::Character(Parallaction *vm) : _vm(vm), _ani(new Animation) {
	_talk = NULL;
	_head = NULL;
	_objs = NULL;

	_direction = WALK_DOWN;
	_step = 0;

	_ani->setX(150);
	_ani->setY(100);
	_ani->setZ(10);
	_ani->setF(0);
	_ani->_flags = kFlagsActive | kFlagsNoName | kFlagsCharacter;
	_ani->_type = kZoneYou;
	strncpy(_ani->_name, "yourself", ZONENAME_LENGTH);

	// TODO: move creation into Parallaction. Needs to make Character a pointer first.
	if (_vm->getGameType() == GType_Nippon) {
		_builder = new PathBuilder_NS(this);
		_walker = new PathWalker_NS(this);
	} else {
		_builder = new PathBuilder_BR(this);
		_walker = new PathWalker_BR(this);
	}
}

Character::~Character() {
	delete _builder;
	_builder = 0;

	delete _walker;
	_walker = 0;

	free();
}

void Character::getFoot(Common::Point &foot) {
	Common::Rect rect;
	_ani->gfxobj->getRect(_ani->getF(), rect);

	foot.x = _ani->getX() + (rect.left + rect.width() / 2);
	foot.y = _ani->getY() + (rect.top + rect.height());
}

void Character::setFoot(const Common::Point &foot) {
	Common::Rect rect;
	_ani->gfxobj->getRect(_ani->getF(), rect);

	_ani->setX(foot.x - (rect.left + rect.width() / 2));
	_ani->setY(foot.y - (rect.top + rect.height()));
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
	_engineFlags |= kEngineWalking;
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


void Character::setName(const char *name) {
	_name.bind(name);
}

const char *Character::getName() const {
	return _name.getName();
}

const char *Character::getBaseName() const {
	return _name.getBaseName();
}

const char *Character::getFullName() const {
	return _name.getFullName();
}

bool Character::dummy() const {
	return _name.dummy();
}

void Character::updateDirection(const Common::Point& pos, const Common::Point& to) {

	Common::Point dist(to.x - pos.x, to.y - pos.y);
	WalkFrames *frames = (_ani->getFrameNum() == 20) ? &_char20WalkFrames : &_char24WalkFrames;

	_step++;

	if (dist.x == 0 && dist.y == 0) {
		_ani->setF(frames->stillFrame[_direction]);
		return;
	}

	if (dist.x < 0)
		dist.x = -dist.x;
	if (dist.y < 0)
		dist.y = -dist.y;

	_direction = (dist.x > dist.y) ? ((to.x > pos.x) ? WALK_LEFT : WALK_RIGHT) : ((to.y > pos.y) ? WALK_DOWN : WALK_UP);
	_ani->setF(frames->firstWalkFrame[_direction] + (_step / frames->frameRepeat[_direction]) % frames->numWalkFrames[_direction]);
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

const char CharacterName::_prefixMini[] = "mini";
const char CharacterName::_suffixTras[] = "tras";
const char CharacterName::_empty[] = "\0";

void CharacterName::dummify() {
	_dummy = true;
	_baseName[0] = '\0';
	_name[0] = '\0';
	_fullName[0] = '\0';
}

CharacterName::CharacterName() {
	dummify();
}

CharacterName::CharacterName(const char *name) {
	bind(name);
}

void CharacterName::bind(const char *name) {
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

const char *CharacterName::getName() const {
	return _name;
}

const char *CharacterName::getBaseName() const {
	return _baseName;
}

const char *CharacterName::getFullName() const {
	return _fullName;
}

bool CharacterName::dummy() const {
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


} // namespace Parallaction
