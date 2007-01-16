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

#include "parallaction/parallaction.h"
#include "parallaction/menu.h"
#include "parallaction/disk.h"
#include "parallaction/music.h"
#include "parallaction/inventory.h"
#include "parallaction/graphics.h"
#include "parallaction/zone.h"

#include "common/util.h"
#include "common/file.h"


namespace Parallaction {

// FIXME: remove this
Parallaction *_vm = NULL;

// public stuff
Point	_mousePos = { 0, 0 };
uint16	_mouseButtons = 0;


char		_saveData1[30] = { '\0' };
uint16		_language = 0;
char		_location[100] = "fogne";
uint32		_engineFlags = 0;
char	   *_objectsNames[100];
Zone	   *_activeZone = NULL;
Animation	_yourself;
uint16		_score = 1;
Command    *_locationACommands = NULL;
Command    *_locationCommands = NULL;
char	   *_locationComment = NULL;
char	   *_locationEndComment = NULL;
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
char		_slideText[2][40];
Point		_firstPosition = { -1000, -1000 };
char		_newLocation[100];
char	   *_globalTable[32];
uint16		_firstFrame = 0;
Cnv 		_characterFace;
byte		_mouseHidden = 0;
Node		_locationWalkNodes = { 0, 0 };
uint32		_commandFlags = 0;
Cnv 		_tempFrames;
uint16		_introSarcData3 = 200;
uint16		_introSarcData2 = 1;

// private stuff




static Job	   *_jDrawInventory = NULL;
static Job	   *_jDrawLabel = NULL;
static Job	   *_jEraseLabel = NULL;
static Zone    *_hoverZone = NULL;
static Job	   *_jRunScripts = NULL;

static Cnv		_miniCharacterFrames;

static Job		_jobs = { { NULL, NULL }, 0, 0, 0, NULL, 0 };




Parallaction::Parallaction(OSystem *syst) :
	Engine(syst) {

	// FIXME
	_vm = this;

	_skipMenu = false;

	_transCurrentHoverItem = 0;
	_actionAfterWalk = false;  // actived when the character needs to move before taking an action
	_activeItem._index = 0;
	_activeItem._id = 0;
	_procCurrentHoverItem = -1;

	_musicData1 = 0;
	strcpy(_characterName1, "null");


	_baseTime = 0;

	Common::File::addDefaultDirectory( _gameDataPath );

/*    Common::addSpecialDebugLevel(kDebugAnimation, "Animation", "Animations debug level");
    Common::addSpecialDebugLevel(kDebugZone, "Zone", "Zones debug level");
    Common::addSpecialDebugLevel(kDebugCommand, "Command", "Commands debug level");
    Common::addSpecialDebugLevel(kDebugDialogue, "Dialogue", "Dialogues debug level");
    Common::addSpecialDebugLevel(kDebugIntro, "Intro", "Intro debug level");
    Common::addSpecialDebugLevel(kDebugInventory, "Inventory", "Inventory debug level");
    Common::addSpecialDebugLevel(kDebugLocation, "Location", "Locations debug level");
*/
    Common::addSpecialDebugLevel(kDebugDisk, "Disk", "Disk debug level");
    Common::addSpecialDebugLevel(kDebugWalk, "Walk", "Walk debug level");
}


Parallaction::~Parallaction() {

}


int Parallaction::init() {

	// Detect game
	if (!detectGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}

//	strcpy(_location, "night");
//	strcpy(_characterName, "drki");
//	strcpy(_languageDir, "it/");
//	_skipMenu = true;

	_engineFlags = 0;

	strcpy(_characterName, "dough");
	memset(_locationNames, 0, 120*32);
	_numLocations = 0;




	_yourself._zone.pos._position._x = 150;
	_yourself._zone.pos._position._y = 100;
	initInventory();
	_yourself._z = 10;

	_yourself._zone.pos._oldposition._x = -1000;
	_yourself._zone.pos._oldposition._y = -1000;
	_yourself._frame = 0;

	_yourself._zone._flags = kFlagsActive | kFlagsNoName;
	_yourself._zone._type = kZoneYou;

	_yourself._zone._label._data0 = NULL;
	_yourself._zone._name = strdup("yourself");

	addNode(&_animations, &_yourself._zone._node);
	_graphics = new Graphics(this);


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

	char *v4 = strchr(_location, '.');
	if (v4) {
		*v4 = '\0';
	}

	_engineFlags &= ~kEngineChangeLocation;
	changeCharacter(_characterName);

	strcpy(_saveData1, _location);
	parseLocation(_location);

	if (_firstPosition._x != -1000) {
		_yourself._zone.pos._position._x = _firstPosition._x;
		_yourself._zone.pos._position._y = _firstPosition._y;
		_yourself._frame = _firstFrame;
		_firstPosition._y = -1000;
		_firstPosition._x = -1000;
	}

	return;
}

void Parallaction::initGlobals() {

	initTable("global.tab", _globalTable);
}

//
//	broken input management
//
uint16 Parallaction::updateInput() {

	OSystem::Event e;
	uint16 KeyDown = 0;

	_mouseButtons = kMouseNone;

	while (g_system->pollEvent(e)) {

		switch (e.type) {
		case OSystem::EVENT_KEYDOWN:
			if (e.kbd.ascii == ' ') KeyDown = kEvQuitGame;
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
			_engineFlags |= kEngineQuit;
			break;

		default:
			break;

		}

	}

	return KeyDown;

}


void waitUntilLeftClick() {

	OSystem::Event e;

	for (;;) {
		g_system->pollEvent(e);
		g_system->delayMillis(10);
		if (e.type == OSystem::EVENT_LBUTTONUP)
			break;

		if (e.type == OSystem::EVENT_QUIT) {
			_engineFlags |= kEngineQuit;
			break;
		}
	}


	return;
}


void Parallaction::runGame() {
//	printf("runGame()\n");

	addJob(jobEraseAnimations, (void*)1, JOBPRIORITY_RUNSTUFF);
	_jRunScripts = addJob(jobRunScripts, 0, JOBPRIORITY_RUNSTUFF);
	addJob(jobDisplayAnimations, 0, JOBPRIORITY_DRAWANIMATIONS);

	_graphics->copyScreen(Graphics::kBitBack, Graphics::kBit2);

	if (_locationCommands)
		runCommands(_locationCommands);

	runJobs();

	_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);

	if (_locationComment)
		doLocationEnterTransition();

	changeCursor(kCursorArrow);

	if (_locationACommands)
		runCommands(_locationACommands);

//	printf("entering game loop...\n");

	while ((_engineFlags & kEngineQuit) == 0) {
		_keyDown = updateInput();

		if ((_mouseHidden == 0) && ((_engineFlags & kEngineMouse) == 0) && ((_engineFlags & kEngineWalking) == 0)) {
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
			changeLocation(_location);
			continue;
		}

		g_system->delayMillis(30);

//		printflags();
		runJobs();

		if ((_engineFlags & kEnginePauseJobs) == 0 || (_engineFlags & kEngineInventory)) {
			_graphics->swapBuffers();
			byte palette[PALETTE_SIZE];
			memcpy(palette, _palette, sizeof(palette));
			_graphics->animatePalette(palette);
			_graphics->setPalette(palette);
		}

	}

//	printf("exiting game loop...\n");

	delete _menu;

	return;
}


void Parallaction::processInput(InputData *data) {
//	printf("processInput()\n");
	Zone *z;
	WalkNode *v4;

	switch (data->_event) {
	case kEvEnterZone:
		_graphics->_labelPosition[1]._x = -1000;
		_graphics->_labelPosition[1]._y = -1000;
		_graphics->_labelPosition[0]._x = -1000;
		_graphics->_labelPosition[0]._y = -1000;
		_jDrawLabel = addJob(&jobDisplayLabel, (void*)data->_cnv, JOBPRIORITY_DRAWLABEL);
		_jEraseLabel = addJob(&jobEraseLabel, (void*)data->_cnv, JOBPRIORITY_HIDEINVENTORY);
		break;

	case kEvExitZone:
		removeJob(_jDrawLabel);
		addJob(&jobWaitRemoveJob, _jEraseLabel, JOBPRIORITY_RUNSTUFF);
		_jDrawLabel = NULL;
		break;

	case kEvAction:
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
		_hoverZone = 0;
		if (_jDrawLabel != 0) {
			removeJob(_jDrawLabel);
			_jDrawLabel = NULL;
			addJob(&jobWaitRemoveJob, _jEraseLabel, JOBPRIORITY_SHOWINVENTORY);
		}
		if (hitZone(kZoneYou, _mousePos._x, _mousePos._y) == 0)
		changeCursor(kCursorArrow);
		removeJob(_jRunScripts);
		_jDrawInventory = addJob(&jobShowInventory, 0, JOBPRIORITY_SHOWINVENTORY);
		openInventory();
		break;

	case kEvCloseInventory: // closes inventory and possibly select item
		closeInventory();
		if ((data->_inventoryIndex != -1) && (_inventory[data->_inventoryIndex]._id != 0)) {
			// activates item
			changeCursor(data->_inventoryIndex);
		}
		_jRunScripts = addJob(&jobRunScripts, 0, JOBPRIORITY_RUNSTUFF);
		addJob(&jobHideInventory, 0, JOBPRIORITY_HIDEINVENTORY);
		removeJob(_jDrawInventory);
		break;

	case kEvHoverInventory:
		highlightInventoryItem(_procCurrentHoverItem, 12);	// disable
		highlightInventoryItem(data->_inventoryIndex, 19);						// enable
		_procCurrentHoverItem = data->_inventoryIndex;
		break;

	case kEvWalk:
		_hoverZone = 0;
		changeCursor(kCursorArrow);
		if (_yourself._zone._flags & kFlagsRemove) break;
		if ((_yourself._zone._flags & kFlagsActive) == 0) break;
		v4 = buildWalkPath(data->_mousePos._x, data->_mousePos._y);
		addJob(jobWalk, v4, JOBPRIORITY_WALK);
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
//	printf("translateInput(%c)\n", _keyDown);

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
//			printf("1\n");

			// if walking is over, then take programmed action

			_input._event = kEvAction;
			_actionAfterWalk = false;
			return &_input;
		}

		Zone *z = hitZone(_activeItem._id, _mousePos._x, _mousePos._y);

		if (_mouseButtons == kMouseRightDown) {
//			printf("2\n");

			// right button down shows inventory

			if (hitZone(kZoneYou, _mousePos._x, _mousePos._y) && (_activeItem._id != 0)) {
//				printf("2.1\n");

				_activeItem._index = (_activeItem._id >> 16) & 0xFFFF;
				_engineFlags |= kEngineDragging;
			}


			_input._event = kEvOpenInventory;
			_transCurrentHoverItem = -1;
			return &_input;
		}

		if (((_mouseButtons == kMouseLeftUp) && (_activeItem._id == 0) && ((_engineFlags & kEngineWalking) == 0)) && ((z == NULL) || ((z->_type & 0xFFFF) != kZoneCommand))) {
//			printf("3\n");

			_input._event = kEvWalk;
			return &_input;
		}

		if ((z != _hoverZone) && (_hoverZone != NULL)) {
//			printf("4\n");

			_hoverZone = NULL;
			_input._event = kEvExitZone;
//			_input._data= &z->_name;
			return &_input;
		}

		if (z == NULL) {
//			printf("5\n");

			return NULL;
		}

		if ((_hoverZone == NULL) && ((z->_flags & kFlagsNoName) == 0)) {
//			printf("6\n");

			_hoverZone = z;
			_input._event = kEvEnterZone;
			_input._cnv = &z->_label;
			return &_input;
		}

		if ((_mouseButtons == kMouseLeftUp) && ((_activeItem._id != 0) || ((z->_type & 0xFFFF) == kZoneCommand))) {

			_input._zone = z;
			if (z->_flags & kFlagsNoWalk) {
//				printf("7.1\n");

				// character doesn't need to walk to take specified action
				_input._event = kEvAction;

			} else {
//				  printf("7.2\n");

				// action delayed: if Zone defined a moveto position the character is programmed to move there,
				// else it will move to the mouse position

				_input._event = kEvWalk;
				_actionAfterWalk = true;
				if (z->_moveTo._y != 0) {
	//				printf("3.2.2\n");
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

//	printf("8\n");


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
//	printf("waitTime(%i)\n", t);

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
//	printf("changeCursor(%i)\n", index);

	if (index == kCursorArrow) {		// standard mouse pointer

		if (_jDrawLabel != NULL) {
			removeJob(_jDrawLabel);
			addJob(&jobWaitRemoveJob, _jEraseLabel, JOBPRIORITY_RUNSTUFF);
			_jDrawLabel = NULL;
		}

		_activeItem._id = 0;

	} else {
		_activeItem._id = _inventory[index]._id;
	}

	_graphics->setMousePointer(index);

	return;
}



void freeCharacterFrames() {

	_vm->_graphics->freeCnv(&_tempFrames);

	if (_vm->_characterName[0] != 'D') {
		_vm->_graphics->freeCnv(&_miniCharacterFrames);
		_vm->freeTable(_objectsNames);
	}

	return;
}



void Parallaction::changeCharacter(const char *name) {
//	printf("changeCharacter(%s)\n", name);

	uint16 _si = 0;

	if (!scumm_strnicmp(name, "mini", 4)) {
		name+=4;
		_si = 1;
	}

	char v32[20];
	strcpy(v32, name);



	if (_engineFlags & kEngineMiniDonna) {
		strcat(v32, "tras");
	}

	if (scumm_stricmp(v32, _characterName1)) {

		if (scumm_stricmp(_characterName1, "null")) {
			freeCharacterFrames();
		}

		closeArchive();

		strcpy(_disk, "disk1");
		openArchive("disk1");

		char path[PATH_LEN];
		strcpy(path, v32);
		_graphics->loadCnv(path, &_tempFrames);

		if (name[0] != 'D') {
			sprintf(path, "mini%s", v32);
			_graphics->loadCnv(path, &_miniCharacterFrames);

			sprintf(path, "%s.tab", name);
			initTable(path, _objectsNames);

			refreshInventory(name);

			stopMusic();

			if (scumm_stricmp(name, "night") && scumm_stricmp(name, "intsushi")) {
				if (!scumm_stricmp(name, "dino") || !scumm_stricmp(name, "minidino")) {
					loadMusic("dino");
				} else
				if (!scumm_stricmp(name, "donna") || !scumm_stricmp(name, "minidonna")) {
					loadMusic("donna");
				} else {
					loadMusic("nuts");
				}

				playMusic();
			}

		}

	}

	if (_si == 1) {
		memcpy(&_yourself._cnv, &_miniCharacterFrames, sizeof(Cnv));
	} else {
		memcpy(&_yourself._cnv, &_tempFrames, sizeof(Cnv));
	}

	strcpy(_characterName1, v32);

	return;
}

void freeNodeList(Node *list) {

	while (list) {
		Node *v4 = list->_next;
		memFree(list);
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


Job *addJob(JobFn fn, void *parm, uint16 tag) {
//	printf("addJob(%i)\n", tag);

	Job *v8 = (Job*)memAlloc(sizeof(Job));

	v8->_parm = parm;
	v8->_fn = fn;
	v8->_tag = tag;
	v8->_finished = 0;
	v8->_count = 0;

	Job *v4 = &_jobs;

	while (v4->_node._next && ((Job*)(v4->_node._next))->_tag > tag) {
		v4 = (Job*)v4->_node._next;
	}

	addNode(&v4->_node, &v8->_node);
	return v8;
}

void removeJob(Job *j) {
//	printf("removeJob(%x, %i)\n", j, j->_tag);

	removeNode(&j->_node);
	memFree(j);
	return;
}

void pauseJobs() {
	_engineFlags |= kEnginePauseJobs;
	return;
}

void resumeJobs() {
	_engineFlags &= ~kEnginePauseJobs;
	return;
}

void runJobs() {
//	printf("runJobs...START\n");

	if (_engineFlags & kEnginePauseJobs) return;

	Job *j = (Job*)_jobs._node._next;
	while (j) {
//		printf("Job run %s\n", _jobDescriptions[j->_tag]);

		(*j->_fn)(j->_parm, j);
		Job *v4 = (Job*)j->_node._next;

		if (j->_finished == 1) removeJob(j);

		j = v4;
	}

//	printf("runJobs...DONE\n");

	return;}

// this Job uses a static counter to delay removal
// and is in fact only used to remove jEraseLabel jobs
//
void jobWaitRemoveJob(void *parm, Job *j) {
//	printf("jobWaitRemoveJob(%x)\n", parm);

	Job *arg = (Job*)parm;

	static uint16 count = 0;

	_engineFlags |= kEngineMouse;

	count++;
	if (count == 2) {
		count = 0;
		removeJob(arg);
		_engineFlags &= ~kEngineMouse;
		j->_finished = 1;
	}

	return;
}


} // namespace Parallaction
