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
 * Change Log:
 * $Log$
 * Revision 1.5  2001/10/10 16:29:59  strigeus
 * temporary fix to prevent freeze in stan's room until sound is there
 *
 * Revision 1.4  2001/10/10 10:02:33  strigeus
 * alternative mouse cursor
 * basic save&load
 *
 * Revision 1.3  2001/10/09 19:02:28  strigeus
 * command line parameter support
 *
 * Revision 1.2  2001/10/09 18:35:02  strigeus
 * fixed object parent bug
 * fixed some signed/unsigned comparisons
 *
 * Revision 1.1.1.1  2001/10/09 14:30:13  strigeus
 *
 * initial revision
 *
 *
 */

#include "stdafx.h"
#include "scumm.h"

void Scumm::initThings() {
	readIndexFile(1);

	allocResTypeData(5, MKID('NONE'),	0x50, "inventory", 0);
	allocResTypeData(12,MKID('NONE'),10, "temp", 0);
	allocResTypeData(11,MKID('NONE'),5, "scale table", 0);
	allocResTypeData(9, MKID('NONE'),13,"actor name", 0);
	allocResTypeData(10, MKID('NONE'),10,"buffer", 0);
 	allocResTypeData(8, MKID('NONE'),100,"verb", 0);
	allocResTypeData(7, MKID('NONE'),0x32,"string", 0);
	allocResTypeData(13, MKID('NONE'),0x32,"flobject", 0);
	allocResTypeData(14, MKID('NONE'),10,"boxes", 0);

	readIndexFile(2);
	initRandSeeds();
}


void Scumm::initRandSeeds() {
	_randSeed1 = 0xA943DE35;
	_randSeed2 = 0x37A9ED27;
}

uint Scumm::getRandomNumber(uint max) {
	/* TODO: my own random number generator */
	_randSeed1 = 0xDEADBEEF * (_randSeed1 + 1);
	_randSeed1 = (_randSeed1>>13) | (_randSeed1<<19);
	return _randSeed1%(max+1);
}

void Scumm::scummInit() {
	int i;
	Actor *a;

	debug(9, "scummInit");
	readIndexFile(3);
	loadCharset(1);
	initScreens(0, 16, 320, 144);

	setShake(0);
	setCursor(0);

	for (i=1,a=getFirstActor(); ++a,i<13; i++) {
		a->number = i;
		initActor(a, 1);
	}

	memset(vm.vars, 0, sizeof(vm.vars));
	memset(vm.bitvars, 0, sizeof(vm.bitvars));

	_defaultTalkDelay = 60;
	vm.vars[37] = 4;

	_numNestedScripts = 0;
	vm.cutSceneStackPointer = 0;

	memset(vm.cutScenePtr, 0, sizeof(vm.cutScenePtr));
	memset(vm.cutSceneData, 0, sizeof(vm.cutSceneData));

	for (i=0; i<_maxVerbs; i++) {
		verbs[i].verbid = 0;
		verbs[i].right = 319;
		verbs[i].oldleft = -1;
		verbs[i].type = 0;
		verbs[i].color = 2;
		verbs[i].hicolor = 0;
		verbs[i].charset_nr = 1;
		verbs[i].curmode = 0;
		verbs[i].saveid = 0;
		verbs[i].center=0;
		verbs[i].key = 0;
	}

	camera._leftTrigger = 10;
	camera._rightTrigger = 30;
	camera._mode = 0;
	camera._follows = 0;


	virtscr[0].xstart = 0;

	vm.vars[9] = 11;

	_lightsValueA = _lightsValueB = 7;

	vm.vars[59] = 3;

	mouse.x = 104;
	mouse.y = 56;

	_ENCD_offs = 0;
	_EXCD_offs = 0;

	_unkTabIndex = 0xFF;
	_currentScript = 0xFF;

	_currentRoom = 0;
	_numObjectsInRoom = 0;
	_actorToPrintStrFor = 0;

	charset._bufPos = 0;
	_haveMsg = 0;

	_screenStartStrip = 0;

	vm.vars[25] = 0;

	_talkDelay = 0;
	_keepText = false;

	_cursorState = 0;
	_userPut = 0;
	
	_newEffect = 129;
	_fullRedraw = 1;

	clearDrawObjectQueue();

	for (i=0; i<6; i++) {
		textslot.x[i] = 2;
		textslot.y[i] = 5;
		textslot.right[i] = 319;
		textslot.color[i] = 0xF;
		textslot.center[i] = 0;
		textslot.charset[i] = 0;
	}

	_numInMsgStack = 0;

	createResource(12, 6, 500);

	initScummVars();

	vm.vars[54] = -0x50;

	getGraphicsPerformance();	
}


void Scumm::initScummVars() {
	vm.vars[VAR_CURRENTDRIVE] = _currentDrive;
	vm.vars[VAR_FIXEDDISK] = checkFixedDisk();
	vm.vars[VAR_SOUNDCARD] = _soundCardType;
	vm.vars[VAR_VIDEOMODE] = _videoMode;
	vm.vars[VAR_HEAPSPACE] = _heapSpace;
	vm.vars[VAR_MOUSEPRESENT] = _mousePresent;
	vm.vars[VAR_SOUNDPARAM] = _soundParam;
	vm.vars[VAR_SOUNDPARAM2] = _soundParam2;
	vm.vars[VAR_SOUNDPARAM3] = _soundParam3;
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

	_fileHandle = NULL;
	
	_bootParam = 0;
	_debugMode = 1;

	parseCommandLine(argc, argv);
	
	initGraphics(this);

 	initThings();
	scummInit();

	vm.vars[VAR_VERSION] = 21; 
	vm.vars[VAR_DEBUGMODE] = _debugMode;

	runScript(1,0,0,&_bootParam);
	_scummTimer = 0;

	do {
		if (_playBackFile) {
			while ((_scummTimer>>2) < vm.vars[VAR_PLAYBACKTIMER]) {}
			_scummTimer = vm.vars[VAR_PLAYBACKTIMER] << 2;
		}
		
		updateScreen(this);

		vm.vars[VAR_TIMER] = _scummTimer >> 2;
		do {
			waitForTimer(this);
			tmr = _scummTimer >> 2;
			if (_fastMode)
				tmr += 15;
		} while (tmr < vm.vars[VAR_TIMER_NEXT]);
		_scummTimer = 0;

		vm.vars[VAR_TMR_1] += tmr;
		vm.vars[VAR_TMR_2] += tmr;
		vm.vars[VAR_TMR_3] += tmr;
		vm.vars[VAR_TMR_4] += tmr;

		if (tmr > 15)
			tmr = 15;

		decreaseScriptDelay(tmr);

		_talkDelay -= tmr;
		if (_talkDelay<0) _talkDelay=0;

		processKbd();

		/* XXX: memory low check skipped */
		vm.vars[VAR_CAMERA_CUR_POS] = camera._curPos;
		vm.vars[VAR_HAVE_MSG] = _haveMsg;
		vm.vars[VAR_VIRT_MOUSE_X] = _virtual_mouse_x;
		vm.vars[VAR_VIRT_MOUSE_Y] = _virtual_mouse_y;
		vm.vars[VAR_MOUSE_X] = mouse.x;
		vm.vars[VAR_MOUSE_Y] = mouse.y;
		vm.vars[VAR_DEBUGMODE] = _debugMode;

		if (_saveLoadFlag) {
			char buf[256];
			sprintf(buf, "savegame.%d", _saveLoadSlot);
			if (_saveLoadFlag==1) {
				saveState(buf);
			} else {
				loadState(buf);
			}
			_saveLoadFlag = 0;
		}

		if (_completeScreenRedraw) {
			_completeScreenRedraw = 0;
			clearUpperMask();
			charset._hasMask = false;
			redrawVerbs();
			_fullRedraw = 1;
			for (i=0,a=getFirstActor(); i<13; i++,a++)
				a->needRedraw = 1;
		}

		checkHeap();
		runAllScripts();
		checkHeap();
		checkExecVerbs();
		checkHeap();
		checkAndRunVar33();
		checkHeap();

		if (_currentRoom==0) {
			gdi.unk4 = 0;
			CHARSET_1();
			unkVirtScreen2();
			unkSoundProc22();
			camera._lastPos = camera._curPos;
			continue;
		}

		checkHeap();
		walkActors();
		checkHeap();
		moveCamera();
		checkHeap();
		fixObjectFlags();
		checkHeap();
		CHARSET_1();
		checkHeap();
		if (camera._curPos != camera._lastPos || _BgNeedsRedraw || _fullRedraw) {
			redrawBGAreas();
			checkHeap();
		}
		processDrawQue();
		checkHeap();
		setActorRedrawFlags();
		checkHeap();
		resetActorBgs();
		checkHeap();

		if (!(vm.vars[VAR_DRAWFLAGS]&2) && vm.vars[VAR_DRAWFLAGS]&4) {
			error("Flashlight not implemented in this version");
		}

		processActors(); /* process actors makes the heap invalid */
		checkHeap();
		clear_fullRedraw();
		checkHeap();
		cyclePalette();
		checkHeap();
		palManipulate();
		checkHeap();
		
		if (dseg_4F8A) {
			screenEffect(_newEffect);
			dseg_4F8A = 0;
			clearClickedStatus();
		}
		
		if (_cursorState > 0) {
			verbMouseOver(checkMouseOver(mouse.x, mouse.y));
		}

		gdi.unk4 = _cursorState > 0;

		unkVirtScreen2();

		playActorSounds();
		unkSoundProc22();
		camera._lastPos = camera._curPos;
	} while (1);
}

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
				default:
				  goto ShowHelpAndExit;
				}
				s++;
			}
NextArg:;
		} else {
ShowHelpAndExit:;
			printf(
				"ScummVM - Scumm Interpreter\n"
				"Syntax:\n"
				"\tscummvm [-b<num>]\n"
				"Flags:\n"
				"\tb<num> - start in that room\n");
			exit(1);
		}
	}
}


void Scumm::startScene(int room, Actor *a, int objectNr) {
	int i;
	Actor *at;

	checkHeap();

	clearMsgQueue();

	unkVirtScreen4(_switchRoomEffect2);
	_newEffect = _switchRoomEffect;

	if (_currentScript!=0xFF) {
		if (vm.slot[_currentScript].type==1 || vm.slot[_currentScript].type==4) {
			if(vm.slot[_currentScript].cutsceneOverride!=0)
				error("Object %d stopped with active cutscene/override in exit", vm.slot[_currentScript].number);
			_currentScript = 0xFF;
		} else if (vm.slot[_currentScript].type==3) {
			if (vm.slot[_currentScript].cutsceneOverride!=0)
				error("Script %d stopped with active cutscene/override in exit", vm.slot[_currentScript].number);	
			_currentScript = 0xFF;
		}
	}

	vm.vars[VAR_NEW_ROOM] = room;
	runExitScript();
	killScriptsAndResources();
	stopCycle(0);
	
	for(i=1,at=getFirstActor(); ++at,i<13; i++) {
		if (at->visible)
			hideActor(at);
	}

	for (i=0; i<0x100; i++)
		cost._transEffect[i] = i;

	clearDrawObjectQueue();

	vm.vars[VAR_ROOM] = room;
	_fullRedraw = 1;

	_roomResource = _currentRoom = 0xFF;

	unkResourceProc();

	_currentRoom = room;
	vm.vars[VAR_ROOM] = room;

	if (room >= 0x80)
		_roomResource = _resourceMapper[room&0x7F];
	else
		_roomResource = room;

	vm.vars[VAR_ROOM_RESOURCE] = _roomResource;

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
	
	if (_roomResource == 0)
		return;

	vm.vars[VAR_CAMERA_MAX] = (_scrWidthIn8Unit<<3) - 160;
	vm.vars[VAR_CAMERA_MIN] = 160;

	memset(actorDrawBits, 0, sizeof(actorDrawBits));

	if (a) {
		if (whereIsObject(objectNr)!=1 &&
			whereIsObject(objectNr)!=4)
				error("startScene: Object %d is not in room %d", objectNr, _currentRoom);
		getObjectXYPos(objectNr);
		putActor(a, _xPos, _yPos, _currentRoom);
		startAnimActor(a, 0x3E, _dir^1);
		a->moving = 0;
	}

	showActors();
	dseg_3A76 = 0;
	runEntryScript();


	if (a && dseg_3A76==0) {
		getObjectXYPos(objectNr);
		putActor(a, _xPos, _yPos, _currentRoom);
		a->moving = 0;
	}

	dseg_4F8A = 1;

	checkHeap();
}

void Scumm::initRoomSubBlocks() {
	int i,offs;
	byte *ptr;
	byte *roomptr;

	_ENCD_offs = 0;
	_EXCD_offs = 0;

	nukeResource(0xE, 1);
	nukeResource(0xE, 2);
	
	for (i=1; i<_maxScaleTable; i++)
		nukeResource(0xB, i);

	roomptr = getResourceAddress(1, _roomResource);
	
	ptr = findResource(MKID('RMHD'), roomptr);
	_scrWidthIn8Unit = READ_LE_UINT16(&((RoomHeader*)ptr)->width) >> 3;
	_scrHeight = READ_LE_UINT16(&((RoomHeader*)ptr)->height);

	_IM00_offs = findResource(MKID('IM00'), findResource(MKID('RMIM'), roomptr)) - 
		roomptr;
	
	ptr = findResource(MKID('EXCD'), roomptr);
	if (ptr) {
		_EXCD_offs = ptr - roomptr;
#ifdef DUMP_SCRIPTS
		dumpResource("exit-", _roomResource, ptr);
#endif
	}

	ptr = findResource(MKID('ENCD'), roomptr);
	if (ptr) {
		_ENCD_offs = ptr - roomptr;
#ifdef DUMP_SCRIPTS
		dumpResource("entry-", _roomResource, ptr);
#endif
	}
	
	ptr = findResource(MKID('BOXD'), roomptr);
	if (ptr) {
		int size = READ_BE_UINT32_UNALIGNED(ptr+4);
		createResource(14, 2, size);
		roomptr = getResourceAddress(1, _roomResource);
		ptr = findResource(MKID('BOXD'), roomptr);
		memcpy(getResourceAddress(0xE, 2), ptr, size);
	}

	ptr = findResource(MKID('BOXM'), roomptr);
	if (ptr) {
		int size = READ_BE_UINT32_UNALIGNED(ptr+4);
		createResource(14, 1, size);
		roomptr = getResourceAddress(1, _roomResource);
		ptr = findResource(MKID('BOXM'), roomptr);
		memcpy(getResourceAddress(0xE, 1), ptr, size);
	}

	ptr = findResource(MKID('SCAL'), roomptr);
	if (ptr) {
		offs = ptr - roomptr;
		for (i=1; i<_maxScaleTable; i++, offs+=8) {
			int a = READ_LE_UINT16(roomptr + offs + 8);
			int b = READ_LE_UINT16(roomptr + offs + 10);
			int c = READ_LE_UINT16(roomptr + offs + 12);
			int d = READ_LE_UINT16(roomptr + offs + 14);
			if (a || b || c || d) {
				setScaleItem(i, b, a, d, c);
				roomptr = getResourceAddress(1, _roomResource);
			}
		}
	}
	memset(_localScriptList, 0, (0x100 - _numGlobalScriptsUsed) * 4);

	roomptr = getResourceAddress(1, _roomResource);
	ptr = findResource(MKID('LSCR'), roomptr);
	while (ptr) {
		_localScriptList[ptr[8] - _numGlobalScriptsUsed] = ptr - roomptr;

#ifdef DUMP_SCRIPTS
		do {
			char buf[32];
			sprintf(buf,"room-%d-",_roomResource);
			dumpResource(buf, ptr[8], ptr);
		} while (0);
#endif

		ptr = findResource(MKID('LSCR'), NULL);
	}

	_CLUT_offs = findResource(MKID('CLUT'), roomptr) - roomptr;
	ptr = findResource(MKID('EPAL'), roomptr);
	if (ptr)
		_EPAL_offs = ptr - roomptr;

	setPaletteFromRes();
	initCycl(findResource(MKID('CYCL'), roomptr) + 8);

	ptr = findResource(MKID('TRNS'), roomptr);
	if (ptr)
		gdi.transparency = ptr[8];
	else
		gdi.transparency = 255;

	initBGBuffers();
}

void Scumm::setScaleItem(int slot, int a, int b, int c, int d) {
	byte *ptr;
	int cur,amounttoadd,i,tmp;

	ptr = createResource(11, slot, 200);

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

	sprintf(buf, "f:\\descumm\\%s%d.dmp", tag,index);

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

void Scumm::unkRoomFunc2(int a, int b, int c, int d, int e) {
	byte *cptr, *cur;
	int num;
	byte color;

	if (_videoMode==0xE) {
		warning("stub unkRoomFunc2(%d,%d,%d,%d,%d)",a,b,c,d,e);
	}

	if (_videoMode==0x13) {
		cptr = getResourceAddress(1, _roomResource) + _CLUT_offs + 8 + a*3;
		cur = _currentPalette + a*3;
		if (a <= b) {
			num = b - a + 1;

			do {
				if (c != 0xFF) {
					color = *cptr++ * (c>>2) / 0xFF;
				} else {
					color = *cptr++ >> 2;
				}
				if(color>63) color = 63;
				*cur++=color;

				if (d != 0xFF) {
					color = *cptr++ * (d>>2) / 0xFF;
				} else {
					color = *cptr++ >> 2;
				}
				if(color>63) color = 63;
				*cur++=color;

				if (e != 0xFF) {
					color = *cptr++ * (e>>2) / 0xFF;
				} else {
					color = *cptr++ >> 2;
				}
				if(color>63) color = 63;
				*cur++=color;
			} while (--num);
		}
		setDirtyColors(a,b);
	}
}


void Scumm::unkRoomFunc4(int a, int b, int c, int d, int e) {
	/* TODO: implement this */
	warning("unkRoomFunc4: not implemented");
}


void Scumm::pauseGame(int i) {
	/* TODO: implement this */
	warning("pauseGame: not implemented");
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

	if (_lastKeyHit==vm.vars[VAR_RESTART_KEY]) {
		warning("Restart not implemented");
		pauseGame(1);
		return;
	}

	if (_lastKeyHit==vm.vars[VAR_PAUSE_KEY]) {
		warning("Pause not implemented");
		/* pause */
		return;
	}

	if (_lastKeyHit==vm.vars[VAR_CUTSCENEEXIT_KEY]) {
		uint32 offs = vm.cutScenePtr[vm.cutSceneStackPointer];
		if (offs) {
			ScriptSlot *ss = &vm.slot[vm.cutSceneScript[vm.cutSceneStackPointer]];
			ss->offs = offs;
			ss->status = 2;
			ss->freezeCount = 0;
			ss->cutsceneOverride--;
			vm.vars[VAR_OVERRIDE] = 1;
			vm.cutScenePtr[vm.cutSceneStackPointer] = 0;
		}
	}

	if (_lastKeyHit==vm.vars[VAR_TALKSTOP_KEY]) {
		_talkDelay = 0;
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
		_lastKeyHit = vm.vars[VAR_CUTSCENEEXIT_KEY];
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
	if (id<1 || id>=13)
		error("Invalid actor %d in %s", id, errmsg);
	return derefActor(id);
}

extern Scumm scumm;

void NORETURN CDECL error(const char *s, ...) {
	char buf[1024];
	va_list va;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);

	if (scumm._currentScript != 0xFF) {
		fprintf(stderr, "Error(%d): %s!\nPress a key to quit.\n", scumm.vm.slot[scumm._currentScript].number, buf);
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
#if 1

//if (_heapchk() != _HEAPOK) {
//		error("Heap is invalid!");
//	}
#endif
}

