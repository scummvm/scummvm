/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm.h"
#include "sound/mididrv.h"
#include "sound/imuse.h"
#include "actor.h"
#include "debug.h"
#include "gameDetector.h"
#include "gui.h"
#include "newgui.h"
#include "object.h"
#include "resource.h"
#include "string.h"

#ifdef _WIN32_WCE
extern void GraphicsOff(void);
#endif

int autosave(int interval)	/* Not in class to prevent being bound */
{
	g_scumm->_doAutosave = true;
	return interval;
}

void Scumm::initRandSeeds()
{
	_randSeed1 = 0xA943DE33;
	_randSeed2 = 0x37A9ED29;
}

uint Scumm::getRandomNumber(uint max)
{
	/* TODO: my own random number generator */
	_randSeed1 = 0xDEADBF03 * (_randSeed1 + 1);
	_randSeed1 = (_randSeed1 >> 13) | (_randSeed1 << 19);
	return _randSeed1 % (max + 1);
}

uint Scumm::getRandomNumberRng(uint min, uint max)
{
	return getRandomNumber(max - min) + min;
}


void Scumm::scummInit()
{
	int i;
	Actor *a;

	tempMusic=0;
	debug(9, "scummInit");

	if (_features & GF_SMALL_HEADER)
		_resourceHeaderSize = 6;
	else
		_resourceHeaderSize = 8;

	if (!(_features & GF_SMALL_NAMES))
		loadCharset(1);

	initScreens(0, 16, 320, 144);

	setShake(0);
	setupCursor();

	/* Allocate and initilise actors */
	_actors = new Actor[MAX_ACTORS];
	for (i = 1, a = getFirstActor(); ++a, i < NUM_ACTORS; i++) {
		a->number = i;
		a->initActorClass(this);
		a->initActor(1);
	}
	
	_vars[VAR_CHARINC] = 4;

	_numNestedScripts = 0;
	vm.cutSceneStackPointer = 0;

	memset(vm.cutScenePtr, 0, sizeof(vm.cutScenePtr));
	memset(vm.cutSceneData, 0, sizeof(vm.cutSceneData));

	for (i = 0; i < _maxVerbs; i++) {
		_verbs[i].verbid = 0;
		_verbs[i].right = 319;
		_verbs[i].oldleft = -1;
		_verbs[i].type = 0;
		_verbs[i].color = 2;
		_verbs[i].hicolor = 0;
		_verbs[i].charset_nr = 1;
		_verbs[i].curmode = 0;
		_verbs[i].saveid = 0;
		_verbs[i].center = 0;
		_verbs[i].key = 0;
	}

	if (!(_features & GF_AFTER_V7)) {
		camera._leftTrigger = 10;
		camera._rightTrigger = 30;
		camera._mode = 0;
	}
	camera._follows = 0;

	virtscr[0].xstart = 0;

	if (!(_features & GF_AFTER_V7)) {
		_vars[VAR_V5_DRAWFLAGS] = 11;
		_vars[VAR_59] = 3;

		_vars[VAR_CURRENT_LIGHTS] = LIGHTMODE_actor_base | LIGHTMODE_actor_color | LIGHTMODE_screen;
	}

	mouse.x = 104;
	mouse.y = 56;

	_ENCD_offs = 0;
	_EXCD_offs = 0;

	_currentScript = 0xFF;
	_sentenceNum = 0;

	_currentRoom = 0;
	_numObjectsInRoom = 0;
	_actorToPrintStrFor = 0;

	charset._bufPos = 0;
	_haveMsg = 0;

	_varwatch = -1;
	_screenStartStrip = 0;

	_vars[VAR_TALK_ACTOR] = 0;

	_talkDelay = 0;
	_keepText = false;

	_cursorState = 0;
	_userPut = 0;

	_newEffect = 129;
	_fullRedraw = true;

	clearDrawObjectQueue();

	for (i = 0; i < 6; i++) {
		if (_features & GF_OLD256) {
			string[i].t_xpos = 0;
			string[i].t_ypos = 0;
		} else {
			string[i].t_xpos = 2;
			string[i].t_ypos = 5;
		}
		string[i].t_right = 319;
		string[i].t_color = 0xF;
		string[i].t_center = 0;
		string[i].t_charset = 0;
	}

	_numInMsgStack = 0;

	createResource(rtTemp, 6, 500);

	initScummVars();

	if (!(_features & GF_AFTER_V6))
		_vars[VAR_V5_TALK_STRING_Y] = -0x50;

	getGraphicsPerformance();

#ifdef COMPRESSED_SOUND_FILE
	_current_cache = 0;
#endif
	
	_system->set_timer(5 * 60 * 1000, &autosave);
}


void Scumm::initScummVars()
{
	if (!(_features & GF_AFTER_V7)) {
		_vars[VAR_CURRENTDRIVE] = _currentDrive;
		_vars[VAR_FIXEDDISK] = checkFixedDisk();
		_vars[VAR_SOUNDCARD] = _soundCardType;
		_vars[VAR_VIDEOMODE] = 0x13;
		_vars[VAR_HEAPSPACE] = 630;
		_vars[VAR_MOUSEPRESENT] = _mousePresent;
		_vars[VAR_SOUNDPARAM] = _soundParam;
		_vars[VAR_SOUNDPARAM2] = _soundParam2;
		_vars[VAR_SOUNDPARAM3] = _soundParam3;
		if (_features & GF_AFTER_V6)
			_vars[VAR_V6_EMSSPACE] = 10000;
	}
}

void Scumm::checkRange(int max, int min, int no, const char *str)
{
	if (no < min || no > max) {
		error("Value %d is out of bounds (%d,%d) int script(%d) msg %s", no, min,
					max, vm.slot[_curExecScript].number, str);
	}
}

int Scumm::scummLoop(int delta)
{
	static int counter = 0;

#ifndef _WIN32_WCE
	if (_debugger)
		_debugger->on_frame();
#endif

	// Randomize the PRNG by calling it at regular intervals. This ensures
	// that it will be in a different state each time you run the program.
	getRandomNumber(2);

	_vars[VAR_TMR_1] += delta;
	_vars[VAR_TMR_2] += delta;
	_vars[VAR_TMR_3] += delta;
	_vars[VAR_TMR_4] += delta;

	if (delta > 15)
		delta = 15;

	decreaseScriptDelay(delta);

	_talkDelay -= delta;
	if (_talkDelay < 0)
		_talkDelay = 0;

	processKbd();

	if (_features & GF_AFTER_V7) {
		_vars[VAR_CAMERA_POS_X] = camera._cur.x;
		_vars[VAR_CAMERA_POS_Y] = camera._cur.y;
	} else {
		_vars[VAR_CAMERA_POS_X] = camera._cur.x;
	}
	_vars[VAR_HAVE_MSG] = (_haveMsg == 0xFE) ? 0xFF : _haveMsg;
	_vars[VAR_VIRT_MOUSE_X] = _virtual_mouse_x;
	_vars[VAR_VIRT_MOUSE_Y] = _virtual_mouse_y;
	_vars[VAR_MOUSE_X] = mouse.x;
	_vars[VAR_MOUSE_Y] = mouse.y;
	_vars[VAR_DEBUGMODE] = _debugMode;

	if (_features & GF_AUDIOTRACKS) {		
		if (delta) {
			if (++counter != 2)
				_vars[VAR_MI1_TIMER] += 5;
			else {
				counter = 0;
				_vars[VAR_MI1_TIMER] += 6;
			}				
		}
	} else if (_features & GF_OLD256) {

		if(tempMusic == 3) {
			tempMusic = 0;
			_vars[VAR_MUSIC_FLAG]++;
		} else {
			tempMusic ++;
		}
	}


	if (_saveLoadFlag) {
		if (_saveLoadFlag == 1) {
			saveState(_saveLoadSlot, _saveLoadCompatible);
			// Ender: Disabled for small_header games, as
			// can overwrite game variables (eg, Zak256 cashcards)
			if (_saveLoadCompatible && !(_features & GF_SMALL_HEADER))
 				_vars[VAR_GAME_LOADED] = 201;
		} else {
			loadState(_saveLoadSlot, _saveLoadCompatible);
			// Ender: Disabled for small_header games, as
			// can overwrite game variables (eg, Zak256 cashcards)
 			if (_saveLoadCompatible && !(_features & GF_SMALL_HEADER))
				_vars[VAR_GAME_LOADED] = 203;
		}
		_saveLoadFlag = 0;
	}

	if (_doAutosave) {
		_saveLoadSlot = 0;
		sprintf(_saveLoadName, "Autosave %d", _saveLoadSlot);
		_saveLoadFlag = 1;
		_saveLoadCompatible = false;
		_doAutosave = false;
	}

	if (_completeScreenRedraw) {
		_completeScreenRedraw = false;
		gdi.clearUpperMask();
		charset._hasMask = false;
		redrawVerbs();
		_fullRedraw = true;
	}

	runAllScripts();
	checkExecVerbs();
	checkAndRunVar33();

	if (_currentRoom == 0) {
		gdi._cursorActive = 0;
		CHARSET_1();
		drawDirtyScreenParts();
		processSoundQues();
		camera._last = camera._cur;
	} else {
		walkActors();
		moveCamera();
		fixObjectFlags();
		CHARSET_1();
		if (!(_features & GF_AFTER_V7)) {
			if (camera._cur.x != camera._last.x || _BgNeedsRedraw || _fullRedraw) {
				redrawBGAreas();
			}
		} else {
			if (camera._cur.x != camera._last.x || camera._cur.y != camera._last.y
					|| _BgNeedsRedraw || _fullRedraw) {
				redrawBGAreas();
			}
		}
		processDrawQue();
		setActorRedrawFlags();
		resetActorBgs();

		if (!(_vars[VAR_CURRENT_LIGHTS] & LIGHTMODE_screen) &&
		      _vars[VAR_CURRENT_LIGHTS] & LIGHTMODE_flashlight) {
			warning("Flashlight not implemented in this version");
		}

		processActors();
		clear_fullRedraw();
		cyclePalette();
		palManipulate();

		if (_doEffect) {
			_doEffect = false;
			fadeIn(_newEffect);
			clearClickedStatus();
		}

		if (_cursorState > 0) {
			verbMouseOver(checkMouseOver(mouse.x, mouse.y));
		}

		drawBlastObjects();
		drawDirtyScreenParts();
		removeBlastObjects();

		if (!(_features & GF_AFTER_V6))
			playActorSounds();

		processSoundQues();
		camera._last = camera._cur;
	}

	if (!(++_expire_counter)) {
		increaseResourceCounter();
	}

	animateCursor();
	
	/* show or hide mouse */
	_system->show_mouse(_cursorState > 0);

	_vars[VAR_TIMER] = 0;
	return _vars[VAR_TIMER_NEXT];

}

void Scumm::startScene(int room, Actor * a, int objectNr)
{
	int i, where;
	Actor *at;

	CHECK_HEAP debug(1, "Loading room %d", room);

	clearMsgQueue();

	fadeOut(_switchRoomEffect2);
	_newEffect = _switchRoomEffect;

	if (_currentScript != 0xFF) {
		if (vm.slot[_currentScript].where == WIO_ROOM ||
				vm.slot[_currentScript].where == WIO_FLOBJECT) {
			if (vm.slot[_currentScript].cutsceneOverride != 0)
				error("Object %d stopped with active cutscene/override in exit",
							vm.slot[_currentScript].number);
			_currentScript = 0xFF;
		} else if (vm.slot[_currentScript].where == WIO_LOCAL) {
			if (vm.slot[_currentScript].cutsceneOverride != 0)
				error("Script %d stopped with active cutscene/override in exit",
							vm.slot[_currentScript].number);
			_currentScript = 0xFF;
		}
	}

	if (!(_features & GF_SMALL_HEADER))  // Disable for SH games. Overwrites
		_vars[VAR_NEW_ROOM] = room; // gamevars, eg Zak cashcards

	runExitScript();
	killScriptsAndResources();
	clearEnqueue();
	stopCycle(0);

	for (i = 1, at = getFirstActor(); ++at, i < NUM_ACTORS; i++) {
		at->hideActor();
	}

	if (!(_features & GF_AFTER_V7)) {
		for (i = 0; i < 0x100; i++)
			_shadowPalette[i] = i;
	}

	clearDrawObjectQueue();

	_vars[VAR_ROOM] = room;
	_fullRedraw = true;

	increaseResourceCounter();

	_currentRoom = room;
	_vars[VAR_ROOM] = room;

	if (room >= 0x80)
		_roomResource = _resourceMapper[room & 0x7F];
	else
		_roomResource = room;

	_vars[VAR_ROOM_RESOURCE] = _roomResource;

	if (room != 0)
		ensureResourceLoaded(1, room);

	if (_currentRoom == 0) {
		_ENCD_offs = _EXCD_offs = 0;
		_numObjectsInRoom = 0;
		return;
	}

	initRoomSubBlocks();
	if (_features & GF_SMALL_HEADER)
		loadRoomObjectsSmall();
	else
		loadRoomObjects();

	if (!(_features & GF_AFTER_V7)) {
		camera._mode = CM_NORMAL;
		camera._cur.x = camera._dest.x = 160;
		camera._cur.y = camera._dest.y = 100;
	}

	if (_features & GF_AFTER_V6) {
		_vars[VAR_V6_SCREEN_WIDTH] = _scrWidth;
		_vars[VAR_V6_SCREEN_HEIGHT] = _scrHeight;
	}

	if (_features & GF_AFTER_V7) {
		_vars[VAR_CAMERA_MIN_X] = 160;
		_vars[VAR_CAMERA_MAX_X] = _scrWidth - 160;
		_vars[VAR_CAMERA_MIN_Y] = 100;
		_vars[VAR_CAMERA_MAX_Y] = _scrHeight - 100;
		setCameraAt(160, 100);
	} else {
		_vars[VAR_CAMERA_MAX_X] = _scrWidth - 160;
		_vars[VAR_CAMERA_MIN_X] = 160;
	}

	if (_roomResource == 0)
		return;


	memset(gfxUsageBits, 0, sizeof(gfxUsageBits));

	if (a) {
		where = whereIsObject(objectNr);
		if (where != WIO_ROOM && where != WIO_FLOBJECT)
			error("startScene: Object %d is not in room %d", objectNr,
						_currentRoom);
		int x, y, dir;
		getObjectXYPos(objectNr, x, y, dir);
		a->putActor(x, y, _currentRoom);
		a->setDirection(dir + 180);
		a->moving = 0;
	}

	showActors();

	_egoPositioned = false;
	runEntryScript();

	if (!(_features & GF_AFTER_V7)) {
		if (a && !_egoPositioned) {
			int x, y;
			getObjectXYPos(objectNr, x, y);
			a->putActor(x, y, _currentRoom);
			a->moving = 0;
		}
	} else {
		if (camera._follows) {
			Actor *a = derefActorSafe(camera._follows, "startScene: follows");
			setCameraAt(a->x, a->y);
		}
	}

	_doEffect = true;

	CHECK_HEAP;
}

void Scumm::initRoomSubBlocks()
{
	int i, offs;
	byte *ptr;
	byte *roomptr, *searchptr;
	RoomHeader *rmhd;

	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_CLUT_offs = 0;
	_PALS_offs = 0;

	nukeResource(rtMatrix, 1);
	nukeResource(rtMatrix, 2);

	for (i = 1; i < _maxScaleTable; i++)
		nukeResource(rtScaleTable, i);

	roomptr = getResourceAddress(rtRoom, _roomResource);

	rmhd = (RoomHeader *)findResourceData(MKID('RMHD'), roomptr);

	if (_features & GF_AFTER_V7) {
		_scrWidth = READ_LE_UINT16(&(rmhd->v7.width));
		_scrHeight = READ_LE_UINT16(&(rmhd->v7.height));
	} else {
		_scrWidth = READ_LE_UINT16(&(rmhd->old.width));
		_scrHeight = READ_LE_UINT16(&(rmhd->old.height));
	}


	if (_features & GF_SMALL_HEADER)
		_IM00_offs = findResourceData(MKID('IM00'), roomptr) - roomptr;
	else
		_IM00_offs =
			findResource(MKID('IM00'),
									 findResource(MKID('RMIM'), roomptr)) - roomptr;

	ptr = findResourceData(MKID('EXCD'), roomptr);
	if (ptr) {
		_EXCD_offs = ptr - roomptr;
#ifdef DUMP_SCRIPTS
		dumpResource("exit-", _roomResource, ptr - 8);
#endif
	}

	ptr = findResourceData(MKID('ENCD'), roomptr);
	if (ptr) {
		_ENCD_offs = ptr - roomptr;
#ifdef DUMP_SCRIPTS
		dumpResource("entry-", _roomResource, ptr - 8);
#endif
	}

	if (_features & GF_SMALL_HEADER) {
		ptr = findResourceData(MKID('BOXD'), roomptr);
		if (ptr) {
			byte numOfBoxes = *(ptr);
			int size;
			if (_features & GF_OLD256)
				size = numOfBoxes * (SIZEOF_BOX - 2) + 1;
			else
				size = numOfBoxes * SIZEOF_BOX + 1;


			createResource(rtMatrix, 2, size);
			memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
			ptr += size;
			size = getResourceDataSize(ptr - size - 6) - size;

			if (size >= 0) {					// do this :)
				createResource(rtMatrix, 1, size);
				memcpy(getResourceAddress(rtMatrix, 1), ptr, size);
			}

		}
	} else {
		ptr = findResourceData(MKID('BOXD'), roomptr);
		if (ptr) {
			int size = getResourceDataSize(ptr);
			createResource(rtMatrix, 2, size);
			roomptr = getResourceAddress(rtRoom, _roomResource);
			ptr = findResourceData(MKID('BOXD'), roomptr);
			memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
		}

		ptr = findResourceData(MKID('BOXM'), roomptr);
		if (ptr) {
			int size = getResourceDataSize(ptr);
			createResource(rtMatrix, 1, size);
			roomptr = getResourceAddress(rtRoom, _roomResource);
			ptr = findResourceData(MKID('BOXM'), roomptr);
			memcpy(getResourceAddress(rtMatrix, 1), ptr, size);
		}
	}

	ptr = findResourceData(MKID('SCAL'), roomptr);
	if (ptr) {
		offs = ptr - roomptr;
		for (i = 1; i < _maxScaleTable; i++, offs += 8) {
			int a = READ_LE_UINT16(roomptr + offs);
			int b = READ_LE_UINT16(roomptr + offs + 2);
			int c = READ_LE_UINT16(roomptr + offs + 4);
			int d = READ_LE_UINT16(roomptr + offs + 6);
			if (a || b || c || d) {
				setScaleItem(i, b, a, d, c);
				roomptr = getResourceAddress(rtRoom, _roomResource);
			}
		}
	}
	memset(_localScriptList, 0, sizeof(_localScriptList));

	searchptr = roomptr = getResourceAddress(rtRoom, _roomResource);
	if (_features & GF_SMALL_HEADER) {
		while ((ptr = findResourceSmall(MKID('LSCR'), searchptr)) != NULL) {
			int id = 0;
			ptr += _resourceHeaderSize;	/* skip tag & size */
			id = ptr[0];
#ifdef DUMP_SCRIPTS
			do {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - 6);
			} while (0);
#endif
			_localScriptList[id - _numGlobalScripts] = ptr + 1 - roomptr;
			searchptr = NULL;
		}
	} else {
		while ((ptr = findResource(MKID('LSCR'), searchptr)) != NULL) {
			int id = 0;

			ptr += _resourceHeaderSize;	/* skip tag & size */

			if (_features & GF_AFTER_V7) {
				id = READ_LE_UINT16(ptr);
				checkRange(2050, 2000, id, "Invalid local script %d");
				_localScriptList[id - _numGlobalScripts] = ptr + 2 - roomptr;
			} else {
				id = ptr[0];
				_localScriptList[id - _numGlobalScripts] = ptr + 1 - roomptr;
			}
#ifdef DUMP_SCRIPTS
			do {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - 8);
			} while (0);
#endif
			searchptr = NULL;
		}
	}

	if (_features & GF_SMALL_HEADER)
		ptr = findResourceSmall(MKID('EPAL'), roomptr);
	else
		ptr = findResource(MKID('EPAL'), roomptr);

	if (ptr)
		_EPAL_offs = ptr - roomptr;

	if (_features & GF_SMALL_HEADER)
		ptr = findResourceSmall(MKID('CLUT'), roomptr);
	else
		ptr = findResourceData(MKID('CLUT'), roomptr);

	if (ptr) {
		_CLUT_offs = ptr - roomptr;
		setPaletteFromRes();
	}

	if (_features & GF_AFTER_V6) {
		ptr = findResource(MKID('PALS'), roomptr);
		if (ptr) {
			_PALS_offs = ptr - roomptr;
			setPalette(0);
		}
	}

	if (_features & GF_SMALL_HEADER)
		ptr = findResourceData(MKID('CYCL'), roomptr);
	else
		ptr = findResourceData(MKID('CYCL'), roomptr);

	if (ptr)
		initCycl(findResourceData(MKID('CYCL'), roomptr));

	ptr = findResourceData(MKID('TRNS'), roomptr);
	if (ptr)
		gdi._transparency = ptr[0];
	else
		gdi._transparency = 255;

	initBGBuffers(_scrHeight);

	memset(_extraBoxFlags, 0, sizeof(_extraBoxFlags));
}

void Scumm::setScaleItem(int slot, int a, int b, int c, int d)
{
	byte *ptr;
	int cur, amounttoadd, i, tmp;

	ptr = createResource(rtScaleTable, slot, 200);

	if (a == c)
		return;

	cur = (b - d) * a;
	amounttoadd = d - b;

	for (i = 200; i > 0; i--) {
		tmp = cur / (c - a) + b;
		if (tmp < 1)
			tmp = 1;
		if (tmp > 255)
			tmp = 255;
		*ptr++ = tmp;
		cur += amounttoadd;
	}
}

void Scumm::dumpResource(char *tag, int idx, byte *ptr)
{
	char buf[256];
	FILE *out;

	uint32 size;
	if (_features & GF_SMALL_HEADER)
		size = READ_LE_UINT32(ptr);
	else
		size = READ_BE_UINT32_UNALIGNED(ptr + 4);

#if defined(MACOS_CARBON)
	sprintf(buf, ":dumps:%s%d.dmp", tag, idx);
#else
	sprintf(buf, "dumps/%s%d.dmp", tag, idx);
#endif

	out = fopen(buf, "rb");
	if (!out) {
		out = fopen(buf, "wb");
		if (!out)
			return;
		fwrite(ptr, size, 1, out);
	}
	fclose(out);
}


void Scumm::clear_fullRedraw()
{
	_fullRedraw = 0;
}

void Scumm::clearClickedStatus()
{
	checkKeyHit();
	_mouseButStat = 0;
	_leftBtnPressed = 0;
	_rightBtnPressed = 0;
}

int Scumm::checkKeyHit()
{
	int a = _keyPressed;
	_keyPressed = 0;
	return a;
}

void Scumm::unkRoomFunc3(int a, int b, int c, int d, int e)
{
	warning("stub unkRoomFunc3(%d,%d,%d,%d,%d)", a, b, c, d, e);
}


void Scumm::palManipulate(int palettes, int brightness, int color, int time, int e)
{
	byte *cptr;

	/* TODO: implement this */
	warning("palManipulate(%d, %d, %d, %d): not implemented", palettes, brightness, color, time);
	
	printf("_curPalIndex=%d\n", _curPalIndex);

	cptr = _currentPalette + color * 3;
	printf("color %d = (%d,%d,%d)\n", color, (int)*cptr++, (int)*cptr++, (int)*cptr++);

//	darkenPalette(0, 255, 0xFF+0x10, brightness, brightness);
	{
		int startColor = 0;
		int endColor = 255;
		int redScale = 0xFF;
		int greenScale = brightness;
		int blueScale = brightness;
		byte *cur;
		int num;
		int color;
	
		cptr = _currentPalette + startColor * 3;
		cur = _currentPalette + startColor * 3;
		num = endColor - startColor + 1;

		do {
			color = *cptr++;
			if (redScale != 0xFF)
				color = color * redScale / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;

			color = *cptr++;
			if (greenScale != 0xFF)
				color = color * greenScale / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;

			color = *cptr++;
			if (blueScale != 0xFF)
				color = color * blueScale / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;
		} while (--num);
		setDirtyColors(startColor, endColor);
	}

	cptr = _currentPalette + color * 3;
	printf("color %d = (%d,%d,%d)\n", color, (int)*cptr++, (int)*cptr++, (int)*cptr++);

//	setPalette(palettes);
}

void Scumm::pauseGame(bool user)
{
	//_gui->pause();
	_newgui->pauseDialog();
}

void Scumm::setOptions()
{
	_gui->options();
	//_newgui->optionsDialog();
}

void Scumm::shutDown(int i)
{
	/* TODO: implement this */
	warning("shutDown: not implemented");
}

void Scumm::processKbd()
{
	int saveloadkey;
	getKeyInput(0);

	if (_features & GF_OLD256) /* FIXME: Support ingame screen */
		saveloadkey = 319;
	else
		saveloadkey = _vars[VAR_SAVELOADDIALOG_KEY];

	_virtual_mouse_x = mouse.x + virtscr[0].xstart;



	if(_features & GF_AFTER_V7)
		_virtual_mouse_y = mouse.y + camera._cur.y-100;
	else
		_virtual_mouse_y = mouse.y;

	if (!(_features & GF_OLD256))
		_virtual_mouse_y += virtscr[0].topline;
	else
		_virtual_mouse_y -= 16;

	if (_virtual_mouse_y < 0)
		_virtual_mouse_y = -1;

	if (_features & GF_OLD256) {
		if (_virtual_mouse_y >= virtscr[0].height + virtscr[0].topline)
			_virtual_mouse_y = -1;
	} else {
		if (_virtual_mouse_y >= virtscr[0].height)
			_virtual_mouse_y = -1;
	}

	if (!_lastKeyHit)
		return;

	if (_lastKeyHit == KEY_SET_OPTIONS) {
		setOptions();
		return;
	}

	if (_lastKeyHit == _vars[VAR_RESTART_KEY]) {
		warning("Restart not implemented");
//    pauseGame(true);
		return;
	}

	if (_lastKeyHit == _vars[VAR_PAUSE_KEY]) {
		pauseGame(true);
		/* pause */
		return;
	}

	if (_lastKeyHit == _vars[VAR_CUTSCENEEXIT_KEY]) {
		if (_insaneState) {
			videoFinished = 1;
		} else
			exitCutscene();
	} else if (_lastKeyHit == saveloadkey
						 && _currentRoom != 0) {
		if (_features & GF_AFTER_V7)
			runScript(_vars[VAR_UNK_SCRIPT], 0, 0, 0);
		_gui->saveLoadDialog();
		if (_features & GF_AFTER_V7)
			runScript(_vars[VAR_UNK_SCRIPT_2], 0, 0, 0);
	} else if (_lastKeyHit == _vars[VAR_TALKSTOP_KEY]) {
		_talkDelay = 0;
		if (_sfxMode == 2)
			stopTalk();
		return;
	} else if (_lastKeyHit == '[') { // [, eg volume down
		_sound_volume_master-=5;
		if (_sound_volume_master < 0)
			_sound_volume_master = 0;
		_imuse->set_master_volume(_sound_volume_master);
	} else if (_lastKeyHit == ']') { // ], eg volume down
		_sound_volume_master+=5;
		if (_sound_volume_master > 128)
			_sound_volume_master = 128;		
		_imuse->set_master_volume(_sound_volume_master);
	} else if (_lastKeyHit == '-') { // -, eg text speed down
		_defaultTalkDelay+=5;
		if (_defaultTalkDelay > 90)
			_defaultTalkDelay = 90;

		_vars[VAR_CHARINC] = _defaultTalkDelay / 20;
	} else if (_lastKeyHit == '+') { // +, eg text speed up
		_defaultTalkDelay-=5;
		if (_defaultTalkDelay < 5)
			_defaultTalkDelay = 5;

		_vars[VAR_CHARINC] = _defaultTalkDelay / 20;
	} else if (_lastKeyHit == 321) { // F7, display new GUI
		_newgui->saveloadDialog();
	}
			
	_mouseButStat = _lastKeyHit;
}

int Scumm::getKeyInput(int a)
{
	_mouseButStat = 0;

	_lastKeyHit = checkKeyHit();
	if (a == 0)
		convertKeysToClicks();

	if (mouse.x < 0)
		mouse.x = 0;
	if (mouse.x > _realWidth-1)
		mouse.x = _realWidth-1;
	if (mouse.y < 0)
		mouse.y = 0;
	if (mouse.y > _realHeight-1)
		mouse.y = _realHeight-1;

	if (_leftBtnPressed & msClicked && _rightBtnPressed & msClicked) {
		_mouseButStat = 0;
		_lastKeyHit = _vars[VAR_CUTSCENEEXIT_KEY];
	} else if (_leftBtnPressed & msClicked) {
		_mouseButStat = MBS_LEFT_CLICK;
	} else if (_rightBtnPressed & msClicked) {
		_mouseButStat = MBS_RIGHT_CLICK;
	}

	if (_features & GF_AFTER_V7) {
//    _vars[VAR_LEFTBTN_DOWN] = (_leftBtnPressed&msClicked) != 0;
		_vars[VAR_LEFTBTN_HOLD] = (_leftBtnPressed & msDown) != 0;
//    _vars[VAR_RIGHTBTN_DOWN] = (_rightBtnPressed&msClicked) != 0;
		_vars[VAR_RIGHTBTN_HOLD] = (_rightBtnPressed & msDown) != 0;
	}

	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;

	return _lastKeyHit;
}

void Scumm::convertKeysToClicks()
{
	if (_lastKeyHit && _cursorState > 0) {
		if (_lastKeyHit == 9) {
			_mouseButStat = MBS_RIGHT_CLICK;
		} else if (_lastKeyHit == 13) {
			_mouseButStat = MBS_LEFT_CLICK;
		} else
			return;
		_lastKeyHit = 0;
	}
}

Actor *Scumm::derefActor(int id)
{
	return &_actors[id];
}

Actor *Scumm::derefActorSafe(int id, const char *errmsg)
{
	if (id < 1 || id >= NUM_ACTORS) {
		if (_debugMode)
		warning
			("Invalid actor %d in %s (script %d, opcode 0x%x) - This is potentially a BIG problem.",
			 id, errmsg, vm.slot[_curExecScript].number, _opcode);
		return NULL;
	}
	return derefActor(id);
}

void Scumm::makeCursorColorTransparent(int a)
{
	int i, size;

	size = _cursorWidth * _cursorHeight;

	for (i = 0; i < size; i++)
		if (_grabbedCursor[i] == (byte)a)
			_grabbedCursor[i] = 0xFF;

	updateCursor();
}

void Scumm::setStringVars(int slot)
{
	StringTab *st = &string[slot];
	st->xpos = st->t_xpos;
	st->ypos = st->t_ypos;
	st->center = st->t_center;
	st->overhead = st->t_overhead;
	st->no_talk_anim = st->t_no_talk_anim;
	st->right = st->t_right;
	st->color = st->t_color;
	st->charset = st->t_charset;
}

void Scumm::unkMiscOp9()
{
	warning("stub unkMiscOp9()");
}

void Scumm::startManiac()
{
	warning("stub startManiac()");
}

void Scumm::destroy()
{
	freeResources();

	free(_objectStateTable);
	free(_objectRoomTable);
	free(_objectOwnerTable);
	free(_inventory);
	free(_arrays);
	free(_verbs);
	free(_objs);
	free(_vars);
	free(_bitVars);
	free(_newNames);
	free(_classData);
}

const int new_dir_table[4] = {
	270,
	90,
	180,
	0,
};

const int16 many_direction_tab[16] = {71, 109, 251, 289, -1, -1, -1, -1, 22, 72, 107, 157, 202, 252, 287, 337};

int newDirToOldDir(int dir)
{
	if (dir >= 71 && dir <= 109)
		return 1;
	if (dir >= 109 && dir <= 251)
		return 2;
	if (dir >= 251 && dir <= 289)
		return 0;
	return 3;
}

int oldDirToNewDir(int dir)
{
	return new_dir_table[dir];
}


int numSimpleDirDirections(int dirType)
{
	return dirType ? 8 : 4;
}


/* Convert an angle to a simple direction */
int toSimpleDir(int dirType, int dir)
{
	int num = dirType ? 8 : 4;
	const int16 *dirtab = &many_direction_tab[dirType * 8];
	for (int i = 1; i < num; i++, dirtab++) {
		if (dir >= dirtab[0] && dir <= dirtab[1])
			return i;
	}
	return 0;

}

/* Convert a simple direction to an angle */
int fromSimpleDir(int dirType, int dir)
{
	if (!dirType)
		return dir * 90;
	else
		return dir * 45;
}


int normalizeAngle(int angle)
{
	int temp;

	temp = (angle + 360) % 360;

	return toSimpleDir(1, temp) * 45;
}

void NORETURN CDECL error(const char *s, ...)
{
	char buf[1024];
#if defined( USE_WINDBG ) || defined ( _WIN32_WCE )
	char buf2[1024];
#if defined( _WIN32_WCE )
	TCHAR buf2w[2048];
#endif
#endif

	va_list va;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);

	if (g_scumm && g_scumm->_currentScript != 0xFF) {
		ScriptSlot *ss = &g_scumm->vm.slot[g_scumm->_currentScript];
		fprintf(stderr, "Error(%d:%d:0x%X): %s!\n",
						g_scumm->_roomResource,
						ss->number,
						g_scumm->_scriptPointer - g_scumm->_scriptOrgPointer, buf);
#if defined( USE_WINDBG ) || defined( _WIN32_WCE )
		sprintf(buf2, "Error(%d:%d:0x%X): %s!\n",
			g_scumm->_roomResource,
			ss->number,
			g_scumm->_scriptPointer - g_scumm->_scriptOrgPointer,
			buf);
#if defined ( _WIN32_WCE )	
			MultiByteToWideChar(CP_ACP, 0, buf2, strlen(buf2) + 1, buf2w, sizeof(buf2w));
			GraphicsOff();
			MessageBox(NULL, buf2w, TEXT("ScummVM error"), MB_OK);
#else
			OutputDebugString(buf2);
#endif
#endif

	} else {
		fprintf(stderr, "Error: %s!\n", buf);
#if defined( USE_WINDBG ) || defined( _WIN32_WCE )
		sprintf(&buf[strlen(buf)], "\n");
#if defined ( _WIN32_WCE )	
			MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf) + 1, buf2w, sizeof(buf2w));
			GraphicsOff();
			MessageBox(NULL, buf2w, TEXT("ScummVM error"), MB_OK);
#else
			OutputDebugString(buf);
#endif
#endif
	}
	// Doesn't wait for any keypress!! Is it intended to?
	exit(1);
}

void CDECL warning(const char *s, ...)
{
	char buf[1024];
	va_list va;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);

	fprintf(stderr, "WARNING: %s!\n", buf);
#if defined( USE_WINDBG )
	sprintf(&buf[strlen(buf)], "\n");
	OutputDebugString(buf);
#endif
}

uint16 _debugLevel = 1;

void CDECL debug(int level, const char *s, ...)
{
	char buf[1024];
	va_list va;

	if (level > _debugLevel)
		return;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);
	printf("%s\n", buf);

#if defined( USE_WINDBG )
	sprintf(&buf[strlen(buf)], "\n");
	OutputDebugString(buf);
#endif

	fflush(stdout);
}

void checkHeap()
{
#if defined(WIN32)
	if (_heapchk() != _HEAPOK) {
		error("Heap is invalid!");
	}
#endif
}

ScummDebugger g_debugger;

void Scumm::waitForTimer(int msec_delay) {
	OSystem::Event event;
	uint32 start_time;

	if (_fastMode&2)
		msec_delay = 0;
	else if (_fastMode&1)
		msec_delay = 10;

	start_time = _system->get_msecs();

	for(;;) {
		while (_system->poll_event(&event)) {

			// if newgui is running, copy event to EventList, and let the GUI handle it itself
			// we might consider this approach for ScummLoop as well, and clean up the current mess
			if (_newgui->isActive()) {
				if (event.event_code == OSystem::EVENT_MOUSEMOVE) {
					mouse.x = event.mouse.x;
					mouse.y = event.mouse.y;
					_system->set_mouse_pos(event.mouse.x, event.mouse.y);
#if !defined(__MORPHOS__)
					_system->update_screen();
#endif
				}
				_newgui->handleEvent(event);
				continue;
			}

			switch(event.event_code) {
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.keycode >= '0' && event.kbd.keycode<='9'
					&& (event.kbd.flags == OSystem::KBD_SHIFT ||
						event.kbd.flags == OSystem::KBD_CTRL)) {
					_saveLoadSlot = event.kbd.keycode - '0';
					sprintf(_saveLoadName, "Quicksave %d", _saveLoadSlot);
					_saveLoadFlag = (event.kbd.flags == OSystem::KBD_SHIFT) ? 1 : 2;
					_saveLoadCompatible = false;
				} else if (event.kbd.flags==OSystem::KBD_CTRL) {
					if (event.kbd.keycode=='f')
						_fastMode ^= 1;
					else if (event.kbd.keycode=='g')
						_fastMode ^= 2;
					else if ((event.kbd.keycode=='d') && (!_system->property(OSystem::PROP_GET_FULLSCREEN, 0)))
						g_debugger.attach(this);
					else if (event.kbd.keycode=='s')
						resourceStats();
				} else
					_keyPressed = event.kbd.ascii;	// Normal key press, pass on to the game.
				break;

			case OSystem::EVENT_MOUSEMOVE:
				mouse.x = event.mouse.x;
				mouse.y = event.mouse.y;
				_system->set_mouse_pos(event.mouse.x, event.mouse.y);
#if !defined(__MORPHOS__)
				_system->update_screen();
#endif
				break;

			case OSystem::EVENT_LBUTTONDOWN:
				_leftBtnPressed |= msClicked|msDown;
#ifdef _WIN32_WCE
				mouse.x = event.mouse.x;
				mouse.y = event.mouse.y;
#endif
				break;

			case OSystem::EVENT_RBUTTONDOWN:
				_rightBtnPressed |= msClicked|msDown;
#ifdef _WIN32_WCE
				mouse.x = event.mouse.x;
				mouse.y = event.mouse.y;
#endif
				break;

			case OSystem::EVENT_LBUTTONUP:
				_leftBtnPressed &= ~msDown;
				break;

			case OSystem::EVENT_RBUTTONUP:
				_rightBtnPressed &= ~msDown;
				break;
			}
		}
#ifdef COMPRESSED_SOUND_FILE
		if (updateMP3CD() == -1)
#endif
			_system->update_cdrom(); /* Loop CD Audio if needed */
		if (_system->get_msecs() >= start_time + msec_delay)
			break;
		_system->delay_msecs(10);
	}
}


void Scumm::updatePalette() {
	if (_palDirtyMax == -1)
		return;
	
	int first = _palDirtyMin;
	int num = _palDirtyMax - first + 1;
	int i;
	byte *data = _currentPalette + first * 3;

	byte palette_colors[1024],*p = palette_colors;

	// Sam & Max film noir mode
	if (_gameId == GID_SAMNMAX && readVar(0x8000))
		desaturatePalette();
	
	for (i = 0; i != num; i++, data += 3, p+=4) {
		p[0] = data[0];
		p[1] = data[1];
		p[2] = data[2];
		p[3] = 0;
	}
	
	_system->set_palette(palette_colors, first, num);

	_palDirtyMax = -1;
	_palDirtyMin = 256;
}

void Scumm::mainRun()
{
	int delta = 0;
	int last_time = _system->get_msecs(); 
	int new_time;

	for(;;) {
		
		updatePalette();
		
		_system->update_screen();		
		new_time = _system->get_msecs();
		waitForTimer(delta * 15 + last_time - new_time);
		last_time = _system->get_msecs();
		if (_gui->isActive()) {
			_gui->loop();
			delta = 5;
		} else if (_newgui->isActive()) {
			_newgui->loop();
			delta = 5;
		} else {
			delta = scummLoop(delta);
			if (delta < 1)	// Ensure we don't get into a loop
				delta = 1;  // by not decreasing sleepers.
		}
	}
}

void Scumm::launch()
{
	charset._vm = this;
	gdi._vm = this;
	_fileHandle = NULL;

	_maxHeapThreshold = 450000;
	_minHeapThreshold = 400000;

	/* Create a primary virtual screen */
	_videoBuffer = (byte *)calloc(328*200, 1);

	allocResTypeData(rtBuffer, MKID('NONE'), 10, "buffer", 0);
	initVirtScreen(0, 0, 0, 320, 200, false, false);

	if (_features & GF_AFTER_V7)
		setupScummVarsNew();
	else
		setupScummVarsOld();

	if ((_features & GF_AFTER_V7) || (_gameId == GID_SAMNMAX))
		NUM_ACTORS = 30;
	else
		NUM_ACTORS = 13;

	if (_features & GF_AFTER_V7)
		OF_OWNER_ROOM = 0xFF;
	else
		OF_OWNER_ROOM = 0x0F;

	// if (_gameId==GID_MONKEY2 && _bootParam == 0)
	//	_bootParam = 10001;

	if (_gameId == GID_INDY4 && _bootParam == 0) {
		_bootParam = -7873;
	}

	readIndexFile();

	initRandSeeds();

	if (_features & GF_NEW_OPCODES)
		setupOpcodes2();
	else
		setupOpcodes();

	scummInit();

	if (!(_features & GF_AFTER_V7))
		_vars[VAR_VERSION] = 21;

	_vars[VAR_DEBUGMODE] = _debugMode;

	if (_gameId == GID_MONKEY)
		_vars[74] = 1225;

	setupSound();

	runScript(1, 0, 0, &_bootParam);

//  _scummTimer = 0;
}

Scumm *Scumm::createFromDetector(GameDetector *detector, OSystem *syst)
{
	Scumm *scumm;
	OSystem::Property prop;

	if (detector->_features & GF_OLD256)
		scumm = new Scumm_v3;
	else if (detector->_features & GF_SMALL_HEADER)	// this force loomCD as v4
		scumm = new Scumm_v4;
	else if (detector->_features & GF_AFTER_V7)
		scumm = new Scumm_v7;
	else if (detector->_features & GF_AFTER_V6)	// this force SamnmaxCD as v6
		scumm = new Scumm_v6;
	else
		scumm = new Scumm_v5;

	scumm->_system = syst;

	
	if (detector->_gameId == GID_ZAK256) {	// FmTowns is 320x240
		scumm->_realWidth = 320;
		scumm->_realHeight = 240;
	} else {
		scumm->_realWidth = 320;
		scumm->_realHeight = 200;
	}

	/* This initializes SDL */
	syst->init_size(scumm->_realWidth, scumm->_realHeight);
	prop.cd_num = detector->_cdrom;
	syst->property(OSystem::PROP_OPEN_CD, &prop);

	/* bind the mixer to the system => mixer will be invoked
	 * automatically when samples need to be generated */	
	if (!scumm->_mixer->bind_to_system(syst)) {         
		warning("Sound initialization failed");   
		if (detector->_use_adlib) {   
			detector->_use_adlib = false;   
			detector->_midi_driver = MD_NULL;   
			warning("Adlib music was selected, switching to midi null driver");   
		}   
	} 
	scumm->_mixer->set_volume(kDefaultSFXVolume);
	scumm->_mixer->set_music_volume(kDefaultMusicVolume);

	/* HACK !!! */
	g_scumm = scumm;
	g_system = scumm->_system;
	g_mixer = &scumm->_mixer[0];
	/* END HACK */

	scumm->_debugMode = detector->_debugMode;
	scumm->_bootParam = detector->_bootParam;
	scumm->_gameDataPath = detector->_gameDataPath;
	scumm->_exe_name = detector->_exe_name;
	scumm->_gameId = detector->_gameId;
	scumm->_gameText = detector->_gameText;
	scumm->_features = detector->_features;
	scumm->_soundCardType = detector->_soundCardType;
	scumm->_noSubtitles = detector->_noSubtitles;
	scumm->_cdrom = detector->_cdrom;
	scumm->_defaultTalkDelay = detector->_talkSpeed;
	scumm->_sound_volume_sfx = detector->_sfx_volume;	
	scumm->_sound_volume_music = detector->_music_volume;	
	{
		IMuse *imuse;

		scumm->_use_adlib = detector->_use_adlib;

		if (detector->_use_adlib) {
			imuse = IMuse::create_adlib(syst, scumm->_mixer);
		} else {
			imuse = IMuse::create_midi(syst, detector->createMidi());
		}
		
		if (detector->_gameTempo != 0)
			imuse->property(IMuse::PROP_TEMPO_BASE, detector->_gameTempo);
		
		imuse->set_music_volume(scumm->_sound_volume_music);
		scumm->_imuse = imuse;
	}

	if (detector->_save_slot != -1) {
		scumm->_saveLoadSlot = detector->_save_slot;
		scumm->_saveLoadFlag = 2;
		scumm->_saveLoadCompatible = false;
	}

	return scumm;
}

void Scumm::go() {
	launch();
	setupGUIColors();
	mainRun();
}


byte Scumm::getDefaultGUIColor(int color)
{
	/* FIXME: strange IF line? */
	if ((_features & GF_AFTER_V7) || (_features & GF_SMALL_HEADER))
		return 0;
	if (_features & GF_AFTER_V6) {
		if (color == 8)
			color = 1;
		return readArray(110, 0, color);
	} else {
		return getStringAddress(21)[color];
	}
}

void Scumm::setupGUIColors() {

	/* FIXME: strange IF line? */
	if (_gameId && !(_features & GF_SMALL_HEADER)	&& !(_features & GF_AFTER_V7)) {
		_gui->_bgcolor = getDefaultGUIColor(0);
		_gui->_color = getDefaultGUIColor(1);
		_gui->_textcolor = getDefaultGUIColor(2);
		_gui->_textcolorhi = getDefaultGUIColor(6);
		_gui->_shadowcolor = getDefaultGUIColor(8);

		_newgui->_bgcolor = _gui->_bgcolor;
		_newgui->_color = _gui->_color;
		_newgui->_textcolor = _gui->_textcolor;
		_newgui->_textcolorhi = _gui->_textcolorhi;
		_newgui->_shadowcolor = _gui->_shadowcolor;
	}
}
