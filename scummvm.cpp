/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
#include "gui.h"

void Scumm::initThingsV5() {
	readIndexFileV5(1);

	_numVariables = 800;
	_numBitVariables = 2048;
	_numLocalObjects = 200;
	_numVerbs = 100;
	_numInventory = 80;
	_numVerbs = 100;
	_numArray = 0x32;
	_numFlObject = 0x32;
	
	allocateArrays();
	
	readIndexFileV5(2);
	initRandSeeds();

	setupOpcodes();
}

void Scumm::initThingsV6() {
	setupOpcodes2();
	readIndexFileV6();
}

void Scumm::initRandSeeds() {
	_randSeed1 = 0xA943DE35;
	_randSeed2 = 0x37A9ED27;
}

uint Scumm::getRandomNumber(uint max) {
	/* TODO: my own random number generator */
	_randSeed1 = 0xDEADBEEF * (_randSeed1 + 1);
	_randSeed1 = (_randSeed1>>13) | (_randSeed1<<19);
	return _randSeed1%max;
}

void Scumm::scummInit() {
	int i;
	Actor *a;

	debug(9, "scummInit");
//	readIndexFile(3);
	loadCharset(1);
	initScreens(0, 16, 320, 144);

	setShake(0);
	setupCursor();

	for (i=1,a=getFirstActor(); ++a,i<NUM_ACTORS; i++) {
		a->number = i;
		initActor(a, 1);
	}

	_defaultTalkDelay = 60;
	_vars[VAR_CHARINC] = 4;

	_numNestedScripts = 0;
	vm.cutSceneStackPointer = 0;

	memset(vm.cutScenePtr, 0, sizeof(vm.cutScenePtr));
	memset(vm.cutSceneData, 0, sizeof(vm.cutSceneData));

	for (i=0; i<_maxVerbs; i++) {
		_verbs[i].verbid = 0;
		_verbs[i].right = 319;
		_verbs[i].oldleft = -1;
		_verbs[i].type = 0;
		_verbs[i].color = 2;
		_verbs[i].hicolor = 0;
		_verbs[i].charset_nr = 1;
		_verbs[i].curmode = 0;
		_verbs[i].saveid = 0;
		_verbs[i].center=0;
		_verbs[i].key = 0;
	}

	camera._leftTrigger = 10;
	camera._rightTrigger = 30;
	camera._mode = 0;
	camera._follows = 0;

	virtscr[0].xstart = 0;

	_vars[VAR_V5_DRAWFLAGS] = 11;

	_vars[VAR_59] = 3;

	mouse.x = 104;
	mouse.y = 56;

	_ENCD_offs = 0;
	_EXCD_offs = 0;

	_currentScript = 0xFF;
	_sentenceIndex = 0xFF;

	_currentRoom = 0;
	_numObjectsInRoom = 0;
	_actorToPrintStrFor = 0;

	charset._bufPos = 0;
	_haveMsg = 0;

	_screenStartStrip = 0;

	_vars[VAR_TALK_ACTOR] = 0;

	_talkDelay = 0;
	_keepText = false;

	_cursorState = 0;
	_userPut = 0;
	
	_newEffect = 129;
	_fullRedraw = true;

	clearDrawObjectQueue();

	for (i=0; i<6; i++) {
		string[i].t_xpos = 2;
		string[i].t_ypos = 5;
		string[i].t_right = 319;
		string[i].t_color = 0xF;
		string[i].t_center = 0;
		string[i].t_charset = 0;
	}

	_numInMsgStack = 0;

	createResource(rtTemp, 6, 500);

	initScummVars();

	if (_majorScummVersion==5)
		_vars[VAR_V5_TALK_STRING_Y] = -0x50;

	getGraphicsPerformance();	
}


void Scumm::initScummVars() {
	_vars[VAR_CURRENTDRIVE] = _currentDrive;
	_vars[VAR_FIXEDDISK] = checkFixedDisk();
	_vars[VAR_SOUNDCARD] = _soundCardType;
	_vars[VAR_VIDEOMODE] = 0x13;
	_vars[VAR_HEAPSPACE] = 630;
	_vars[VAR_MOUSEPRESENT] = _mousePresent;
	_vars[VAR_SOUNDPARAM] = _soundParam;
	_vars[VAR_SOUNDPARAM2] = _soundParam2;
	_vars[VAR_SOUNDPARAM3] = _soundParam3;
	if (_majorScummVersion==6)
		_vars[VAR_V6_EMSSPACE] = 10000;
}

void Scumm::checkRange(int max, int min, int no, const char *str) {
	if (no < min || no > max) {
		error("Value %d is out of bounds (%d,%d) msg %s", no, min,max, str);
	}
}

void Scumm::scummMain(int argc, char **argv) {
	int tmr, i;
	Actor *a;

	charset._vm = this;
	cost._vm = this;
	gdi._vm = this;

	_fileHandle = NULL;
	
	_debugMode = 1;

	_maxHeapThreshold = 450000;
	_minHeapThreshold = 400000;
	
	parseCommandLine(argc, argv);

	if (_exe_name==NULL)
		error("Specify the name of the game to start on the command line");

	if (!detectGame()) {
		warning("Game detection failed. Using default settings");
		_majorScummVersion = 5;
	}

	if (_gameId==GID_INDY4 && _bootParam==0) {
		_bootParam = -7873;
	}

	if (_gameId==GID_MONKEY2 && _bootParam==0) {
		_bootParam = 10001;
	}


	initGraphics(this, _fullScreen);

	if (_majorScummVersion==6)
		initThingsV6();
	else
		initThingsV5();

	scummInit();

	_vars[VAR_VERSION] = 21; 
	_vars[VAR_DEBUGMODE] = _debugMode;

	if (_gameId==GID_MONKEY) {
		_vars[74] = 1225;
	}

	setupSound();

	runScript(1,0,0,&_bootParam);
//	_scummTimer = 0;
}

int Scumm::scummLoop(int delta) {
	if (_debugger)
		_debugger->on_frame();
	
	_vars[VAR_TMR_1] += delta;
	_vars[VAR_TMR_2] += delta;
	_vars[VAR_TMR_3] += delta;
	_vars[VAR_TMR_4] += delta;

	if (delta > 15)
		delta = 15;

	decreaseScriptDelay(delta);

	_talkDelay -= delta;
	if (_talkDelay<0) _talkDelay=0;

	processKbd();

	_vars[VAR_CAMERA_CUR_POS] = camera._curPos;
	_vars[VAR_HAVE_MSG] = _haveMsg;
	_vars[VAR_VIRT_MOUSE_X] = _virtual_mouse_x;
	_vars[VAR_VIRT_MOUSE_Y] = _virtual_mouse_y;
	_vars[VAR_MOUSE_X] = mouse.x;
	_vars[VAR_MOUSE_Y] = mouse.y;
	_vars[VAR_DEBUGMODE] = _debugMode;

	if (_gameId==GID_MONKEY)
		_vars[VAR_MI1_TIMER]+=40;

	if (_saveLoadFlag) {
		if (_saveLoadFlag==1) {
			saveState(_saveLoadSlot, _saveLoadCompatible);
			if (_saveLoadCompatible)
				_vars[VAR_GAME_LOADED] = 201;
		} else {
			loadState(_saveLoadSlot, _saveLoadCompatible);
			if (_saveLoadCompatible) {
				_vars[VAR_GAME_LOADED] = 203;
			}
		}
		_saveLoadFlag = 0;
	}

	if (_completeScreenRedraw) {
		int i;
		Actor *a;
		_completeScreenRedraw = false;
		gdi.clearUpperMask();
		charset._hasMask = false;
		redrawVerbs();
		_fullRedraw = true;
		for (i=0,a=getFirstActor(); i<NUM_ACTORS; i++,a++)
			a->needRedraw = 1;
	}

	runAllScripts();
	checkExecVerbs();
	checkAndRunVar33();

	if (_currentRoom==0) {
		gdi._cursorActive = 0;
		CHARSET_1();
		drawDirtyScreenParts();
		processSoundQues();
		camera._lastPos = camera._curPos;
	} else {
		walkActors();
		moveCamera();
		fixObjectFlags();
		CHARSET_1();
		if (camera._curPos != camera._lastPos || _BgNeedsRedraw || _fullRedraw) {
			redrawBGAreas();
		}
		processDrawQue();
		setActorRedrawFlags();
		resetActorBgs();

		if (!(_vars[VAR_V5_DRAWFLAGS]&2) && _vars[VAR_V5_DRAWFLAGS]&4) {
			error("Flashlight not implemented in this version");
		}

		processActors(); /* process actors makes the heap invalid */
		clear_fullRedraw();
		cyclePalette();
		palManipulate();

		if (_doEffect) {
			_doEffect = false;
			screenEffect(_newEffect);
			clearClickedStatus();
		}

		if (_cursorState > 0) {
			verbMouseOver(checkMouseOver(mouse.x, mouse.y));
		}

		gdi._cursorActive = _cursorState > 0;

		drawEnqueuedObjects();
		drawDirtyScreenParts();
		removeEnqueuedObjects();

		if (_majorScummVersion==5)
			playActorSounds();

		processSoundQues();
		camera._lastPos = camera._curPos;
	}

	if (!(++_expire_counter)) {
		increaseResourceCounter();
	}

	_vars[VAR_TIMER] = 0;
	return _vars[VAR_TIMER_NEXT];

}

#if 0
void Scumm::scummMain(int argc, char **argv) {

	do {
		updateScreen(this);


	} while (1);
}
#endif

void Scumm::parseCommandLine(int argc, char **argv) {
	int i;
	char *s;

	/* Parse the arguments */
	for (i=1; i < argc; i++) {
		s = argv[i];
		
		if (s && s[0]=='-') {
			s++;
			while (*s) {
				switch(tolower(*s)) {
				case 'b': 
					_bootParam = atoi(s+1);
					goto NextArg;
				case 'f':
					_fullScreen = true;
					break;
				default:
ShowHelpAndExit:;
					printf(
						"ScummVM - Scumm Interpreter\n"
						"Syntax:\n"
						"\tscummvm [-b<num>] game\n"
						"Flags:\n"
						"\tb<num> - start in that room\n"
						"\tf - fullscreen mode\n");
					exit(1);
				}
				s++;
			}
NextArg:;
		} else {
			if (_exe_name) goto ShowHelpAndExit;
			_exe_name = s;
		}
	}

}


struct VersionSettings {
	const char *filename;
	const char *gamename;
	byte id,major,middle,minor;
};

static const VersionSettings version_settings[] = {
	{"monkey", "Monkey Island 1", GID_MONKEY, 5, 2, 2},
	{"monkey2", "Monkey Island 2: LeChuck's revenge", GID_MONKEY2, 5, 2, 2},
	{"atlantis", "Indiana Jones 4 and the Fate of Atlantis", GID_INDY4, 5, 5, 0},
	{"playfate", "Indiana Jones 4 and the Fate of Atlantis (Demo)", GID_INDY4, 5, 5, 0},
	{"tentacle", "Day Of The Tentacle", GID_TENTACLE, 6, 4, 2},
	{"dottdemo", "Day Of The Tentacle (Demo)", GID_TENTACLE, 6, 3, 2},
	{"samnmax", "Sam & Max", GID_SAMNMAX, 6, 4, 2},
	{"snmdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, 3, 0},
	{NULL,NULL}
};

bool Scumm::detectGame() {
	const VersionSettings *gnl = version_settings;
	
	_gameId = 0;
	_gameText = NULL;
	do {
		if (!scumm_stricmp(_exe_name, gnl->filename)) {
			_gameId = gnl->id;
			_majorScummVersion = gnl->major;
			_middleScummVersion = gnl->middle;
			_minorScummVersion = gnl->minor;
			_gameText = gnl->gamename;
			debug(1, "Detected game '%s', version %d.%d.%d", 
				gnl->gamename, gnl->major, gnl->middle, gnl->minor);
			return true;
		}
	} while ((++gnl)->filename);

	debug(1, "Failed game detection");

	return true;
}

char *Scumm::getGameName() {
	if (_gameText==NULL) {
		char buf[256];
		sprintf(buf, "Unknown game: \"%s\"", _exe_name);
		return strdup(buf);
	}
	return strdup(_gameText);
}

void Scumm::startScene(int room, Actor *a, int objectNr) {
	int i,where;
	Actor *at;

	CHECK_HEAP

	debug(1,"Loading room %d", room);

	clearMsgQueue();

	unkVirtScreen4(_switchRoomEffect2);
	_newEffect = _switchRoomEffect;

	if (_currentScript!=0xFF) {
		if (vm.slot[_currentScript].where==WIO_ROOM || 
			vm.slot[_currentScript].where==WIO_FLOBJECT) {
			if(vm.slot[_currentScript].cutsceneOverride!=0)
				error("Object %d stopped with active cutscene/override in exit", vm.slot[_currentScript].number);
			_currentScript = 0xFF;
		} else if (vm.slot[_currentScript].where==WIO_LOCAL) {
			if (vm.slot[_currentScript].cutsceneOverride!=0)
				error("Script %d stopped with active cutscene/override in exit", vm.slot[_currentScript].number);	
			_currentScript = 0xFF;
		}
	}

	_vars[VAR_NEW_ROOM] = room;
	runExitScript();
	killScriptsAndResources();
	stopCycle(0);
	
	for(i=1,at=getFirstActor(); ++at,i<NUM_ACTORS; i++) {
		if (at->visible)
			hideActor(at);
	}

	for (i=0; i<0x100; i++)
		cost._transEffect[i] = i;

	clearDrawObjectQueue();

	_vars[VAR_ROOM] = room;
	_fullRedraw = 1;

	_roomResource = _currentRoom = 0xFF;

	increaseResourceCounter();

	_currentRoom = room;
	_vars[VAR_ROOM] = room;

	if (room >= 0x80)
		_roomResource = _resourceMapper[room&0x7F];
	else
		_roomResource = room;

	_vars[VAR_ROOM_RESOURCE] = _roomResource;

	if (room!=0)
		ensureResourceLoaded(1, room);

	if (_currentRoom == 0) {
		_ENCD_offs = _EXCD_offs = 0;
		_numObjectsInRoom = 0;
		return;
	}

	initRoomSubBlocks();

	loadRoomObjects();

	camera._mode = 1;
	camera._curPos = camera._destPos = 160;

	if (_majorScummVersion==6) {
		_vars[VAR_V6_SCREEN_WIDTH] = _scrWidthIn8Unit<<3;
		_vars[VAR_V6_SCREEN_HEIGHT] = _scrHeight;
	}

	if (_roomResource == 0)
		return;

	_vars[VAR_CAMERA_MAX] = (_scrWidthIn8Unit<<3) - 160;
	_vars[VAR_CAMERA_MIN] = 160;

	memset(actorDrawBits, 0, sizeof(actorDrawBits));

	if (a) {
		where = whereIsObject(objectNr);
		if (where != WIO_ROOM && where!=WIO_FLOBJECT)
			error("startScene: Object %d is not in room %d", objectNr, _currentRoom);
		getObjectXYPos(objectNr);
		putActor(a, _xPos, _yPos, _currentRoom);
		startAnimActor(a, 0x3E, _dir^1);
		a->moving = 0;
	}

	showActors();

	_egoPositioned = false;
	runEntryScript();
	if (a && !_egoPositioned) {
		getObjectXYPos(objectNr);
		putActor(a, _xPos, _yPos, _currentRoom);
		a->moving = 0;
	}

	_doEffect = true;

	CHECK_HEAP
}

void Scumm::initRoomSubBlocks() {
	int i,offs;
	byte *ptr;
	byte *roomptr;

	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_CLUT_offs = 0;
	_PALS_offs = 0;

	nukeResource(rtMatrix, 1);
	nukeResource(rtMatrix, 2);
	
	for (i=1; i<_maxScaleTable; i++)
		nukeResource(rtScaleTable, i);

	roomptr = getResourceAddress(rtRoom, _roomResource);
	
	ptr = findResource(MKID('RMHD'), roomptr, 0);
	_scrWidthIn8Unit = READ_LE_UINT16(&((RoomHeader*)ptr)->width) >> 3;
	_scrHeight = READ_LE_UINT16(&((RoomHeader*)ptr)->height);

	_IM00_offs = findResource(MKID('IM00'), findResource(MKID('RMIM'), roomptr, 0), 0) - 
		roomptr;
	
	ptr = findResource(MKID('EXCD'), roomptr, 0);
	if (ptr) {
		_EXCD_offs = ptr - roomptr;
#ifdef DUMP_SCRIPTS
		dumpResource("exit-", _roomResource, ptr);
#endif
	}

	ptr = findResource(MKID('ENCD'), roomptr, 0);
	if (ptr) {
		_ENCD_offs = ptr - roomptr;
#ifdef DUMP_SCRIPTS
		dumpResource("entry-", _roomResource, ptr);
#endif
	}
	
	ptr = findResource(MKID('BOXD'), roomptr, 0);
	if (ptr) {
		int size = READ_BE_UINT32_UNALIGNED(ptr+4);
		createResource(rtMatrix, 2, size);
		roomptr = getResourceAddress(rtRoom, _roomResource);
		ptr = findResource(MKID('BOXD'), roomptr, 0);
		memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
	}

	ptr = findResource(MKID('BOXM'), roomptr, 0);
	if (ptr) {
		int size = READ_BE_UINT32_UNALIGNED(ptr+4);
		createResource(rtMatrix, 1, size);
		roomptr = getResourceAddress(rtRoom, _roomResource);
		ptr = findResource(MKID('BOXM'), roomptr, 0);
		memcpy(getResourceAddress(rtMatrix, 1), ptr, size);
	}

	ptr = findResource(MKID('SCAL'), roomptr, 0);
	if (ptr) {
		offs = ptr - roomptr;
		for (i=1; i<_maxScaleTable; i++, offs+=8) {
			int a = READ_LE_UINT16(roomptr + offs + 8);
			int b = READ_LE_UINT16(roomptr + offs + 10);
			int c = READ_LE_UINT16(roomptr + offs + 12);
			int d = READ_LE_UINT16(roomptr + offs + 14);
			if (a || b || c || d) {
				setScaleItem(i, b, a, d, c);
				roomptr = getResourceAddress(rtRoom, _roomResource);
			}
		}
	}
	memset(_localScriptList, 0, (0x100 - _numGlobalScripts) * 4);

	roomptr = getResourceAddress(rtRoom, _roomResource);
	for (i=0; ptr = findResource(MKID('LSCR'), roomptr, i++) ;) {
		_localScriptList[ptr[8] - _numGlobalScripts] = ptr - roomptr;
#ifdef DUMP_SCRIPTS
		do {
			char buf[32];
			sprintf(buf,"room-%d-",_roomResource);
			dumpResource(buf, ptr[8], ptr);
		} while (0);
#endif
	}
	

	ptr = findResource(MKID('EPAL'), roomptr, 0);
	if (ptr)
		_EPAL_offs = ptr - roomptr;
	
	ptr = findResource(MKID('CLUT'), roomptr, 0);
	if (ptr) {
		_CLUT_offs = ptr - roomptr;
		setPaletteFromRes();
	}

	if (_majorScummVersion==6) {
		ptr = findResource(MKID('PALS'), roomptr, 0);
		if (ptr) {
			_PALS_offs = ptr - roomptr;
			setPalette(0);
		}
	}
	
	initCycl(findResource(MKID('CYCL'), roomptr, 0) + 8);

	ptr = findResource(MKID('TRNS'), roomptr, 0);
	if (ptr)
		gdi._transparency = ptr[8];
	else
		gdi._transparency = 255;

	initBGBuffers();
}

void Scumm::setScaleItem(int slot, int a, int b, int c, int d) {
	byte *ptr;
	int cur,amounttoadd,i,tmp;

	ptr = createResource(rtScaleTable, slot, 200);

	if (a==c)
		return;
	
	cur = (b-d)*a;
	amounttoadd = d - b;
	
	for (i=200; i>0; i--) {
		tmp = cur / (c - a) + b;
		if (tmp<1) tmp=1;
		if (tmp>255) tmp=255;
		*ptr++ = tmp;
		cur += amounttoadd;
	}
}

void Scumm::dumpResource(char *tag, int index, byte *ptr) {
	char buf[256];
	FILE *out;
	
	uint32 size = READ_BE_UINT32_UNALIGNED(ptr+4);

	sprintf(buf, "dumps\\%s%d.dmp", tag,index);

	out = fopen(buf,"rb");
	if (!out) {
		out = fopen(buf, "wb");
		if (!out)
			return;
		fwrite(ptr, size, 1, out);
	}
	fclose(out);
}


void Scumm::clear_fullRedraw() {
	_fullRedraw = 0;
}

void Scumm::clearClickedStatus() {
	checkKeyHit();
	_mouseButStat = 0;
	_leftBtnPressed = 0;
	_rightBtnPressed = 0;
}

int Scumm::checkKeyHit() {
	int a = _keyPressed;
	_keyPressed = 0;
	return a;
}
	
void Scumm::unkRoomFunc3(int a, int b, int c, int d, int e) {
	warning("stub unkRoomFunc3(%d,%d,%d,%d,%d)",a,b,c,d,e);
}


void Scumm::unkRoomFunc4(int a, int b, int c, int d, int e) {
	/* TODO: implement this */
	warning("unkRoomFunc4: not implemented");
}

void Scumm::pauseGame(bool user) {
	((Gui*)_gui)->pause();
}

void Scumm::shutDown(int i) {
	/* TODO: implement this */
	warning("shutDown: not implemented");
}

void Scumm::processKbd() {
	getKeyInput(0);

	_virtual_mouse_x = mouse.x + virtscr[0].xstart;
	_virtual_mouse_y = mouse.y + virtscr[0].topline;
	if (_virtual_mouse_y < 0)
		_virtual_mouse_y = -1;
	if (_virtual_mouse_y >= virtscr[0].height)
		_virtual_mouse_y = -1;

	if (!_lastKeyHit)
		return;

	if (_lastKeyHit==_vars[VAR_RESTART_KEY]) {
		warning("Restart not implemented");
//		pauseGame(true);
		return;
	}

	if (_lastKeyHit==_vars[VAR_PAUSE_KEY]) {
		pauseGame(true);
		/* pause */
		return;
	}

	if (_lastKeyHit==_vars[VAR_CUTSCENEEXIT_KEY]) {
		exitCutscene();
	} else if (_lastKeyHit==_vars[VAR_SAVELOADDIALOG_KEY]) {
		((Gui*)_gui)->saveLoadDialog();
	} else if (_lastKeyHit==_vars[VAR_TALKSTOP_KEY]) {
		_talkDelay = 0;
		if (_sfxMode==2)
			stopTalk();
		return;
	}

	_mouseButStat = _lastKeyHit;
}

int Scumm::getKeyInput(int a) {
	_mouseButStat = 0;

	_lastKeyHit = checkKeyHit();
	if (a==0)
		convertKeysToClicks();

	if (mouse.x<0) mouse.x=0;
	if (mouse.x>319) mouse.x=319;
	if (mouse.y<0) mouse.y=0;
	if (mouse.y>199) mouse.y=199;

	if (_leftBtnPressed&1 && _rightBtnPressed&1) {
		_mouseButStat = 0;
		_lastKeyHit = _vars[VAR_CUTSCENEEXIT_KEY];
	} else if (_leftBtnPressed&1) {
		_mouseButStat = 0x8000;
	} else if (_rightBtnPressed&1) {
		_mouseButStat = 0x4000;
	}
	
	_leftBtnPressed &= ~1;
	_rightBtnPressed &= ~1;

	return _lastKeyHit;
}

void Scumm::convertKeysToClicks() {
	if (_lastKeyHit && _cursorState>0) {
		if (_lastKeyHit==9) {
			_mouseButStat = 0x4000;
		}	else if (_lastKeyHit==13) {
			_mouseButStat = 0x8000;	
		} else
			return;
		_lastKeyHit = 0;
	}
}

Actor *Scumm::derefActorSafe(int id, const char *errmsg) {
	if (id<1 || id>=NUM_ACTORS)
		error("Invalid actor %d in %s", id, errmsg);
	return derefActor(id);
}

void Scumm::makeCursorColorTransparent(int a) {
	int i,size;

	size = _cursorWidth * _cursorHeight;

	for(i=0; i<size; i++)
		if (_grabbedCursor[i] == (byte)a)
			_grabbedCursor[i] = 0xFF;
}

void Scumm::setStringVars(int slot) {
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

void Scumm::unkMiscOp9() {
	warning("stub unkMiscOp9()");
}

void Scumm::startManiac() {
	warning("stub startManiac()");
}

void Scumm::destroy() {
	freeResources();

	free(_objectFlagTable);
	free(_inventory);
	free(_arrays);
	free(_verbs);
	free(_objs);
	free(_vars);
	free(_bitVars);
	free(_newNames);
	free(_classData);
}


extern Scumm scumm;

void NORETURN CDECL error(const char *s, ...) {
	char buf[1024];
	va_list va;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);

	if (scumm._currentScript != 0xFF) {
		ScriptSlot *ss = &scumm.vm.slot[scumm._currentScript];
		fprintf(stderr, "Error(%d:%d:0x%X): %s!\nPress a key to quit.\n", 
			scumm._roomResource,
			ss->number,
			scumm._scriptPointer - scumm._scriptOrgPointer,
			buf);
	} else {
		fprintf(stderr, "Error: %s!\nPress a key to quit.\n", buf);
	}
	exit(1);
}

void CDECL warning(const char *s, ...) {
	char buf[1024];
	va_list va;
	
	va_start(va,s);
	vsprintf(buf, s, va);
	va_end(va);

	fprintf(stderr, "WARNING: %s!\n", buf);
}

void CDECL debug(int level, const char *s, ...) {
	char buf[1024];
	va_list va;

	if (level>5)
		return;

	va_start(va,s);
	vsprintf(buf, s, va);
	va_end(va);
	printf("%s\n", buf);
	fflush(stdout);
}

void checkHeap() {
#if defined(WIN32)
	if (_heapchk() != _HEAPOK) {
		error("Heap is invalid!");
	}
#endif
}

