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
#include "string.h"
#include "sound.h"

extern void launcherLoop();
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

uint Scumm::getRandomNumberRng(uint min, uint max) {
	return getRandomNumber(max-min+1)+min;
}


void Scumm::scummInit() {
	int i;
	Actor *a;

	debug(9, "scummInit");

        if(_features & GF_SMALL_HEADER)
                _resourceHeaderSize = 6;
        else
                _resourceHeaderSize = 8;

        if(!(_features & GF_SMALL_NAMES))
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

	if(!(_features & GF_AFTER_V7)) {
		camera._leftTrigger = 10;
		camera._rightTrigger = 30;
		camera._mode = 0;
	}	
	camera._follows = 0;

	virtscr[0].xstart = 0;

	if(!(_features & GF_AFTER_V7)) {
		_vars[VAR_V5_DRAWFLAGS] = 11;
		_vars[VAR_59] = 3;
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

	for (i=0; i<6; i++) {
		if(_features & GF_OLD256)
		{
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

	if (!(_features&GF_AFTER_V6))
		_vars[VAR_V5_TALK_STRING_Y] = -0x50;

	getGraphicsPerformance();	
}


void Scumm::initScummVars() {
	if(!(_features & GF_AFTER_V7)) {
		_vars[VAR_CURRENTDRIVE] = _currentDrive;
		_vars[VAR_FIXEDDISK] = checkFixedDisk();
		_vars[VAR_SOUNDCARD] = _soundCardType;
		_vars[VAR_VIDEOMODE] = 0x13;
		_vars[VAR_HEAPSPACE] = 630;
		_vars[VAR_MOUSEPRESENT] = _mousePresent;
		_vars[VAR_SOUNDPARAM] = _soundParam;
		_vars[VAR_SOUNDPARAM2] = _soundParam2;
		_vars[VAR_SOUNDPARAM3] = _soundParam3;
		if (_features&GF_AFTER_V6)
			_vars[VAR_V6_EMSSPACE] = 10000;
	}
}

void Scumm::checkRange(int max, int min, int no, const char *str) {
	if (no < min || no > max) {
                error("Value %d is out of bounds (%d,%d) int script(%d) msg %s", no, min,max, vm.slot[_curExecScript].number, str);
	}
}

void Scumm::scummMain(int argc, char **argv) {	
	charset._vm = this;
	gdi._vm = this;

	_fileHandle = NULL;
	
	_debugMode = 0;  // off by default...
	_noSubtitles = 0;  // use by default - should this depend on soundtrack?
	_scale = 2;  // double size by default

	_maxHeapThreshold = 450000;
	_minHeapThreshold = 400000;

	_gameDataPath = NULL;
    _gameTempo = 0;
	_videoMode = 0;
    _soundCardType = 3;

	#ifdef WIN32
		_midi_driver = MIDI_WINDOWS;
	#else
		_midi_driver = MIDI_NULL;
	#endif
	parseCommandLine(argc, argv);

	if (_exe_name != NULL) {
	  /* No game selection menu */
	  if (!detectGame()) {
	    warning("Game detection failed. Using default settings");
	    _features = GF_DEFAULT;
	  }
	} else {
	  _gameText = "Please choose a game";
	}
	
	/* Init graphics and create a primary virtual screen */
	initGraphics(this, _fullScreen, _scale);
	allocResTypeData(rtBuffer, MKID('NONE'),10,"buffer", 0);
	initVirtScreen(0, 0, 200, false, false);	

	if (_exe_name==NULL) {
	  launcherLoop();
	  setWindowName(this);
	}

	if (!detectGame()) {
		warning("Game detection failed. Using default settings");
		_features = GF_DEFAULT;
	}

	if (!_gameDataPath) {
		warning("No path was provided. Assuming that data file are in the current directory");
                _gameDataPath = (char *)malloc(sizeof(char) * 2);
		strcpy(_gameDataPath, "");        
	}

	if(_features & GF_AFTER_V7)
		setupScummVarsNew();
	else
		setupScummVarsOld();

	
	if ((_features & GF_AFTER_V7) || (_gameId == GID_SAMNMAX)) 
		NUM_ACTORS = 30;
	else
		NUM_ACTORS = 13;

	if(_features & GF_AFTER_V7)
		OF_OWNER_ROOM = 0xFF;
	else
		OF_OWNER_ROOM = 0x0F;

	
	if (_gameId==GID_INDY4 && _bootParam==0) {
		_bootParam = -7873;
	}

//	if (_gameId==GID_MONKEY2 && _bootParam==0) {
//		_bootParam = 10001;
//	}	

    if (_features & GF_SMALL_HEADER)
        readIndexFileSmall();
    else
        readIndexFile();

	initRandSeeds();

	if (_features & GF_NEW_OPCODES) 
		setupOpcodes2();
	else
		setupOpcodes();

	scummInit();

	if(!(_features & GF_AFTER_V7))
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



#ifndef _WIN32_WCE


	if (_debugger)
		_debugger->on_frame();



#endif
	
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

	if(_features & GF_AFTER_V7) {
		_vars[VAR_CAMERA_POS_X] = camera._cur.x;
		_vars[VAR_CAMERA_POS_Y] = camera._cur.y;
	} else {
		_vars[VAR_CAMERA_POS_X] = camera._cur.x;
	}
	_vars[VAR_HAVE_MSG] = _haveMsg;
	_vars[VAR_VIRT_MOUSE_X] = _virtual_mouse_x;
	_vars[VAR_VIRT_MOUSE_Y] = _virtual_mouse_y;
	_vars[VAR_MOUSE_X] = mouse.x;
	_vars[VAR_MOUSE_Y] = mouse.y;
	_vars[VAR_DEBUGMODE] = _debugMode;

	if (_features & GF_AUDIOTRACKS)
		_vars[VAR_MI1_TIMER]+=5;
	else
		if(_features & GF_OLD256)
			_vars[VAR_MUSIC_FLAG]++;

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
		camera._last = camera._cur;
	} else {
		walkActors();
		moveCamera();
		fixObjectFlags();
		CHARSET_1();
		if(!(_features & GF_AFTER_V7)) {
			if (camera._cur.x != camera._last.x || _BgNeedsRedraw || _fullRedraw) {
				redrawBGAreas();
			}
		} else {
			if (camera._cur.x != camera._last.x || camera._cur.y != camera._last.y || _BgNeedsRedraw || _fullRedraw) {
				redrawBGAreas();
			}
		}
		processDrawQue();
		setActorRedrawFlags();
		resetActorBgs();

//		if (!(_vars[VAR_V5_DRAWFLAGS]&2) && _vars[VAR_V5_DRAWFLAGS]&4) {
//			error("Flashlight not implemented in this version");
//		}

		processActors();
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

		if (!(_features&GF_AFTER_V6))
			playActorSounds();

		processSoundQues();
		camera._last = camera._cur;
	}

	if (!(++_expire_counter)) {
		increaseResourceCounter();
	}

	_vars[VAR_TIMER] = 0;
	return _vars[VAR_TIMER_NEXT];

}


#define USAGE_STRING	"ScummVM - Scumm Interpreter\n" \
						"Syntax:\n" \
						"\tscummvm [-v] [-d] [-n] [-b<num>] [-t<num>] [-s<num>] [-p<path>] [-m<num>] [-f] game\n" \
						"Flags:\n" \
						"\tv       - show version info and exit\n" \
						"\td       - enable debug output\n" \
						"\tn       - no subtitles for speech\n" \
						"\tb<num>  - start in room <num>\n" \
						"\tt<num>  - set music tempo. Suggested: 1F0000\n" \
						"\ts<num>  - set scale factor to <num> (1, 2, or 3 - 2 by default)\n" \
						"\tp<path> - look for game in <path>\n" \
						"\tm<num>  - set music volume to <num> (0-100)\n" \
						"\te<num>  - set music engine. see readme.txt for details\n" \
						"\tr       - emulate roland mt32 instruments\n" \
						"\tf       - fullscreen mode\n" \
						"\tg       - graphics mode. 1 for 2xSai anti-aliasing\n"

void Scumm::parseCommandLine(int argc, char **argv) {
	#if !defined(__APPLE__CW)
	int i;
	char *s;

	// check for arguments
	if (argc < 2)
	{
		printf( USAGE_STRING );
		//exit(1);
	}

	/* Parse the arguments */
	for (i=1; i < argc; i++) {
		s = argv[i];
		
		if (s && s[0]=='-') {
			s++;
			while (*s) {
				switch(tolower(*s)) {
				case 'b': 
                	if (*(s+1) == '\0')
                		goto ShowHelpAndExit;
					_bootParam = atoi(s+1);
					goto NextArg;
				case 'f':
					_fullScreen = true;
					break;
				case 'd':
					_debugMode = true;
					break;
				case 'n':
					_noSubtitles = true;
					break;
				case 's':
                	if (*(s+1) == '\0')
                		goto ShowHelpAndExit;
					_scale = atoi(s+1);
					if (_scale == 0 || _scale > 3)
					{
						// bad scale - only 1, 2, 3 work for now
						printf("Invalid scale '%s' - valid values are 1, 2, 3\n", s+1);
						exit(1);
					}
					goto NextArg;
				case 'v':
					printf("ScummVM " SCUMMVM_VERSION "\nBuilt on " __DATE__ " " __TIME__ "\n");
					#ifdef SCUMMVM_PLATFORM_VERSION
					printf("    " SCUMMVM_PLATFORM_VERSION "\n");
					#endif
					exit(1);
				case 'p':
                	if (*(s+1) == '\0')
                		goto ShowHelpAndExit;
					_gameDataPath = s+1;
                    goto NextArg;
                case 't':
                	if (*(s+1) == '\0')
                		goto ShowHelpAndExit;
                    _gameTempo = atoi(s+1);
                    goto NextArg;
                case 'm': {
                	if (*(s+1) == '\0')
                		goto ShowHelpAndExit;
					SoundEngine *se = (SoundEngine*)_soundEngine;
					
					if (se)						
						se->set_music_volume(atoi(s+1));					
                    goto NextArg;
				}
                case 'r': {
					SoundEngine *se = (SoundEngine*)_soundEngine;
					
					if (se)						
						se->_mt32emulate = true;
					break;
				}
				case 'e':
					if (*(s+1) == '\0')
						goto ShowHelpAndExit;
					_midi_driver = atoi(s+1);
					goto NextArg;
				case 'g':
                	if (*(s+1) == '\0')
                		goto ShowHelpAndExit;
					_videoMode = atoi(s+1);
					goto NextArg;

				default:
ShowHelpAndExit:;
					printf( USAGE_STRING );
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
	#else
	
	//sprintf(_gameDataPath, ":%s:", *argv);
	//_gameDataPath = *argv;
	_midi_driver = 4;
	_exe_name = *argv;
	#endif

}


struct VersionSettings {
	const char *filename;
	const char *gamename;
	byte id,major,middle,minor;
	uint32 features;
};

/*
        This is a list of all known SCUMM games. Commented games are not
        supported at this time */

static const VersionSettings version_settings[] = {
        /* Scumm Version 1 */
//      {"maniac",      "Maniac Mansion (C64)",                         GID_MANIAC64, 1, 0, 0,},
//      {"zak",         "Zak McKracken and the Alien Mindbenders (C64)", GID_ZAK64, 1, 0, 0,},

        /* Scumm Version 2 */
//      {"maniac",      "Maniac Mansion", GID_MANIAC, 2, 0, 0,},
//      {"zak",         "Zak McKracken and the Alien Mindbenders",      GID_ZAK,     2, 0, 0,},
//      {"indy3",       "Indiana Jones and the Last Crusade",           GID_INDY3,   2, 0, 0,},

        /* Scumm Version 3 */
        {"indy3",       "Indiana Jones and the Last Crusade (256)",     GID_INDY3_256,  3, 0, 22, GF_SMALL_HEADER|GF_USE_KEY|GF_SMALL_NAMES|GF_OLD256|GF_NO_SCALLING},
        {"zak256",      "Zak McKracken and the Alien Mindbenders (256)",GID_ZAK256,     3, 0, 0,  GF_SMALL_HEADER|GF_USE_KEY|GF_SMALL_NAMES|GF_OLD256|GF_AUDIOTRACKS|GF_NO_SCALLING},
        {"loom",        "Loom",                                         GID_LOOM,       3, 5, 40, GF_SMALL_HEADER|GF_USE_KEY|GF_SMALL_NAMES|GF_OLD_BUNDLE|GF_16COLOR|GF_NO_SCALLING},

        /* Scumm Version 4 */
        {"monkeyEGA",   "Monkey Island 1 (EGA)",                        GID_MONKEY_EGA, 4, 0, 67, GF_SMALL_HEADER|GF_USE_KEY|GF_16COLOR}, // EGA version

        /* Scumm version 5 */
        {"loomcd",      "Loom (256 color CD version)",                  GID_LOOM256,    5, 1, 42, GF_SMALL_HEADER|GF_USE_KEY|GF_AUDIOTRACKS},
        {"monkey",      "Monkey Island 1",                              GID_MONKEY,     5, 2, 2,  GF_USE_KEY|GF_AUDIOTRACKS},
		{"monkey1",     "Monkey Island 1 (alt)",                              GID_MONKEY,     5, 2, 2,  GF_USE_KEY|GF_AUDIOTRACKS},
        {"monkey2",     "Monkey Island 2: LeChuck's revenge",           GID_MONKEY2,    5, 2, 2,  GF_USE_KEY},
        {"atlantis",    "Indiana Jones 4 and the Fate of Atlantis",     GID_INDY4,      5, 5, 0,  GF_USE_KEY},
        {"playfate",    "Indiana Jones 4 and the Fate of Atlantis (Demo)", GID_INDY4,   5, 5, 0,  GF_USE_KEY},

        /* Scumm Version 6 */
        {"tentacle",    "Day Of The Tentacle",                          GID_TENTACLE, 6, 4, 2, GF_NEW_OPCODES|GF_AFTER_V6|GF_USE_KEY},
        {"dottdemo",    "Day Of The Tentacle (Demo)",                   GID_TENTACLE, 6, 3, 2, GF_NEW_OPCODES|GF_AFTER_V6|GF_USE_KEY},
        {"samnmax",     "Sam & Max",                                    GID_SAMNMAX,  6, 4, 2, GF_NEW_OPCODES|GF_AFTER_V6|GF_USE_KEY|GF_DRAWOBJ_OTHER_ORDER},
        {"snmdemo",     "Sam & Max (Demo)",                             GID_SAMNMAX,  6, 3, 0, GF_NEW_OPCODES|GF_AFTER_V6|GF_USE_KEY},

        /* Scumm Version 7 */
        {"ft",          "Full Throttle",                                GID_FT,       7, 3, 0, GF_NEW_OPCODES|GF_AFTER_V6|GF_AFTER_V7},
	    {"dig",			"The Dig",										GID_DIG,      7, 5, 0, GF_NEW_OPCODES|GF_AFTER_V6|GF_AFTER_V7},	
	
        /* Scumm Version 8 */
//      {"curse",       "The Curse of Monkey Island",                   GID_CMI,      8, 1, 0,},
	{NULL,NULL}
};

bool Scumm::detectGame() {
	const VersionSettings *gnl = version_settings;
	
	_gameId = 0;
	_gameText = NULL;
	do {
		if (!scumm_stricmp(_exe_name, gnl->filename)) {
			_gameId = gnl->id;
//			_majorScummVersion = gnl->major;
//			_middleScummVersion = gnl->middle;
//			_minorScummVersion = gnl->minor;
			_features = gnl->features;
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
	clearEnqueue();
	stopCycle(0);

	for(i=1,at=getFirstActor(); ++at,i<NUM_ACTORS; i++) {
		if (at->visible)
			hideActor(at);
	}
	
	if (!(_features & GF_AFTER_V7)) {
		for (i=0; i<0x100; i++)
			_shadowPalette[i] = i;
	}

	clearDrawObjectQueue();

	_vars[VAR_ROOM] = room;
	_fullRedraw = 1;

	increaseResourceCounter();

	_currentRoom = room;
	_vars[VAR_ROOM] = room;
	// printf("startscene with room 0x%x\n", room);
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
        if(_features & GF_SMALL_HEADER)
                loadRoomObjectsSmall();
        else
                loadRoomObjects();

	if(!(_features & GF_AFTER_V7)) {
		camera._mode = CM_NORMAL;
		camera._cur.x = camera._dest.x = 160;
	}

	if (_features&GF_AFTER_V6) {
		_vars[VAR_V6_SCREEN_WIDTH] = _scrWidth;
		_vars[VAR_V6_SCREEN_HEIGHT] = _scrHeight;
	}

	if(_features & GF_AFTER_V7) {
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
		if (where != WIO_ROOM && where!=WIO_FLOBJECT)
			error("startScene: Object %d is not in room %d", objectNr, _currentRoom);
		getObjectXYPos(objectNr);
		putActor(a, _xPos, _yPos, _currentRoom);
		fixActorDirection(a, _dir + 180);
		a->moving = 0;
	}

	showActors();

	_egoPositioned = false;
	runEntryScript();

	if(!(_features & GF_AFTER_V7)) {
		if (a && !_egoPositioned) {
			getObjectXYPos(objectNr);
			putActor(a, _xPos, _yPos, _currentRoom);
			a->moving = 0;
		}
	} else {
		if (camera._follows) {
			Actor *a = derefActorSafe(camera._follows, "startScene: follows");
			setCameraAt(a->x, a->y);
		}
	}

	_doEffect = true;

	CHECK_HEAP
}

void Scumm::initRoomSubBlocks() {
	int i,offs;
	byte *ptr;
	byte *roomptr,*searchptr;
	RoomHeader *rmhd;

	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_CLUT_offs = 0;
	_PALS_offs = 0;

	nukeResource(rtMatrix, 1);
	nukeResource(rtMatrix, 2);
	
	for (i=1; i<_maxScaleTable; i++)
		nukeResource(rtScaleTable, i);

	roomptr = getResourceAddress(rtRoom, _roomResource);
	
	rmhd = (RoomHeader*)findResourceData(MKID('RMHD'), roomptr);
	
	if(_features & GF_AFTER_V7) {
		_scrWidth = READ_LE_UINT16(&(rmhd->v7.width));
		_scrHeight = READ_LE_UINT16(&(rmhd->v7.height));
	} else {
                _scrWidth = READ_LE_UINT16(&(rmhd->old.width));
		_scrHeight = READ_LE_UINT16(&(rmhd->old.height));
	}
				

        if( _features & GF_SMALL_HEADER)
               _IM00_offs = findResourceData(MKID('IM00'), roomptr) - roomptr;
        else
               _IM00_offs = findResource(MKID('IM00'), findResource(MKID('RMIM'), roomptr)) - roomptr;
	
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

	if(_features & GF_SMALL_HEADER) {
		ptr = findResourceData(MKID('BOXD'), roomptr);
		if (ptr) {
			byte numOfBoxes=*(ptr);
                        int size;
			if (_features & GF_OLD256)
				size = numOfBoxes * (SIZEOF_BOX-2) + 1;
			else
				size = numOfBoxes * SIZEOF_BOX + 1;


			createResource(rtMatrix, 2, size);
			memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
			ptr += size;
			size = getResourceDataSize(ptr-size-6) - size;

			if(size>=0) {			// do this :)
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
		for (i=1; i<_maxScaleTable; i++, offs+=8) {
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
	if(_features & GF_SMALL_HEADER) {
		while( (ptr = findResourceSmall(MKID('LSCR'), searchptr)) != NULL ) {
			int id = 0;
			ptr += _resourceHeaderSize; /* skip tag & size */
	#ifdef DUMP_SCRIPTS
			do {
				char buf[32];
				sprintf(buf,"room-%d-",_roomResource);
				dumpResource(buf, id, ptr - 6);
			} while (0);
	#endif
			id = ptr[0];
			_localScriptList[id - _numGlobalScripts] = ptr + 1 - roomptr;
			searchptr = NULL;
		}
	} else {
                while( (ptr = findResource(MKID('LSCR'), searchptr)) != NULL ) {
                        int id = 0;

                        ptr += _resourceHeaderSize; /* skip tag & size */
                        
			if(_features & GF_AFTER_V7) {
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
                                sprintf(buf,"room-%d-",_roomResource);
                                dumpResource(buf, id, ptr - 8);
                        } while (0);
#endif
                        searchptr = NULL;
                }
        } 

	if( _features & GF_SMALL_HEADER)
		ptr = findResourceSmall(MKID('EPAL'), roomptr);
        else
                ptr = findResource(MKID('EPAL'), roomptr);

	if (ptr)
		_EPAL_offs = ptr - roomptr;

	if( _features & GF_SMALL_HEADER)
		ptr = findResourceSmall(MKID('CLUT'), roomptr);
	else
                ptr = findResourceData(MKID('CLUT'), roomptr);

	if (ptr) {
		_CLUT_offs = ptr - roomptr;
		setPaletteFromRes();
	}

	if (_features&GF_AFTER_V6) {
		ptr = findResource(MKID('PALS'), roomptr);
		if (ptr) {
			_PALS_offs = ptr - roomptr;
			setPalette(0);
		}
	}

        if( _features & GF_SMALL_HEADER)
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

	initBGBuffers();

	memset(_extraBoxFlags, 0, sizeof(_extraBoxFlags));
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

void Scumm::dumpResource(char *tag, int idx, byte *ptr) {
	char buf[256];
	FILE *out;
	
        uint32 size;
        if( _features & GF_SMALL_HEADER )
                size = READ_LE_UINT32(ptr);
        else
                size = READ_BE_UINT32_UNALIGNED(ptr+4);

	sprintf(buf, "dumps/%s%d.dmp", tag,idx);

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

void Scumm::setOptions() {
	((Gui*)_gui)->options();
}

void Scumm::shutDown(int i) {
	/* TODO: implement this */
	warning("shutDown: not implemented");
}

void Scumm::processKbd() {
	getKeyInput(0);

	_virtual_mouse_x = mouse.x + virtscr[0].xstart;
	_virtual_mouse_y = mouse.y;
	
	if(!(_features & GF_OLD256))
		_virtual_mouse_y+=virtscr[0].topline;
	else
		_virtual_mouse_y-=16;
	
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

	if (_lastKeyHit==KEY_SET_OPTIONS) {
		setOptions();
		return;
	}

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

	if (_leftBtnPressed&msClicked && _rightBtnPressed&msClicked) {
		_mouseButStat = 0;
		_lastKeyHit = _vars[VAR_CUTSCENEEXIT_KEY];
	} else if (_leftBtnPressed&msClicked) {
		_mouseButStat = MBS_LEFT_CLICK;
	} else if (_rightBtnPressed&msClicked) {
		_mouseButStat = MBS_RIGHT_CLICK;
	}

	if(_features & GF_AFTER_V7) {
//		_vars[VAR_LEFTBTN_DOWN] = (_leftBtnPressed&msClicked) != 0;
		_vars[VAR_LEFTBTN_HOLD] = (_leftBtnPressed&msDown) != 0;
//		_vars[VAR_RIGHTBTN_DOWN] = (_rightBtnPressed&msClicked) != 0;
		_vars[VAR_RIGHTBTN_HOLD] = (_rightBtnPressed&msDown) != 0;
	}

	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;

	return _lastKeyHit;
}

void Scumm::convertKeysToClicks() {
	if (_lastKeyHit && _cursorState>0) {
		if (_lastKeyHit==9) {
			_mouseButStat = MBS_RIGHT_CLICK;
		}	else if (_lastKeyHit==13) {
			_mouseButStat = MBS_LEFT_CLICK;	
		} else
			return;
		_lastKeyHit = 0;
	}
}

Actor *Scumm::derefActorSafe(int id, const char *errmsg) {
	if (id<1 || id>=NUM_ACTORS) {				
                warning("Invalid actor %d in %s (script %d) - This is potentially a BIG problem.", id, errmsg, vm.slot[_curExecScript].number);
				return NULL;
	}
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

int Scumm::newDirToOldDir(int dir) {
	if (dir>=71 && dir<=109)
		return 1;
	if (dir>=109 && dir<=251)
		return 2;
	if (dir>=251 && dir<=289)
		return 0;
	return 3;
}

const int new_dir_table[4] = {
	270,
	90,
	180,
	0,
};

int Scumm::oldDirToNewDir(int dir) {
	return new_dir_table[dir];
}


int Scumm::numSimpleDirDirections(int dirType) {
	return dirType ? 8 : 4;
}

/* Convert an angle to a simple direction */
int Scumm::toSimpleDir(int dirType, int dir) {
	int num = dirType ? 8 : 4, i;
	const uint16 *dirtab = &many_direction_tab[dirType*8+2];
	for(i=1;i<num;i++,dirtab++) {
		if (dir >= dirtab[0] && dir <= dirtab[1])
			return i;
	}
	return 0;

}

/* Convert a simple direction to an angle */
int Scumm::fromSimpleDir(int dirType, int dir) {
	if (!dirType)dir+=dir;
	return dir * 45;
}


int Scumm::normalizeAngle(int angle) {
	return (angle+360)%360;
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
		fprintf(stderr, "Error(%d:%d:0x%X): %s!\n",
			scumm._roomResource,
			ss->number,
			scumm._scriptPointer - scumm._scriptOrgPointer,
			buf);
	} else {
		fprintf(stderr, "Error: %s!\n", buf);
	}
	// Doesn't wait for any keypress!! Is it intended to?
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

