/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "actor.h"
#include "bundle.h"
#include "charset.h"
#include "debugger.h"
#include "dialogs.h"
#include "imuse.h"
#include "imuse_digi.h"
#include "intern.h"
#include "object.h"
#include "resource.h"
#include "sound.h"
#include "string.h"
#include "verbs.h"
#include "common/gameDetector.h"
#include "common/config-file.h"
#include "gui/console.h"
#include "gui/newgui.h"
#include "gui/message.h"
#include "sound/mixer.h"
#include "sound/mididrv.h"

#ifdef _WIN32_WCE
extern void drawError(char*);
#endif

// Use g_scumm from error() ONLY
Scumm *g_scumm = 0;
ScummDebugger g_debugger;

extern NewGui *g_gui;
extern uint16 _debugLevel;

static const VersionSettings scumm_settings[] = {
	/* Scumm Version 1 */
//	{"maniac64",      "Maniac Mansion (C64)",                          GID_MANIAC64, 1, 0, 0,},
//	{"zak64",         "Zak McKracken and the Alien Mindbenders (C64)", GID_ZAK64, 1, 0, 0,},

	/* Scumm Version 2 */
	{"maniac", "Maniac Mansion", GID_MANIAC, 2, 0, 0,
	 GF_AFTER_V2 | GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALLING, "00.LFL"},
	{"zak",         "Zak McKracken and the Alien Mindbenders",      GID_ZAK,     2, 0, 0,
	 GF_AFTER_V2 | GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALLING, "00.LFL"},

	/* Scumm Version 3 */
	{"indy3EGA", "Indiana Jones and the Last Crusade", GID_INDY3, 3, 0, 0,
	 GF_AFTER_V3 | GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALLING | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE, "00.LFL"},
	{"indy3", "Indiana Jones and the Last Crusade (256)", GID_INDY3_256, 3, 0, 22,
	 GF_AFTER_V3 | GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALLING | GF_OLD256 | GF_ADLIB_DEFAULT, "00.LFL"},
	{"zak256", "Zak McKracken and the Alien Mindbenders (256)", GID_ZAK256, 3, 0, 0,
	 GF_AFTER_V3 | GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALLING | GF_OLD256 | GF_AUDIOTRACKS, "00.LFL"},
	{"loom", "Loom", GID_LOOM, 3, 5, 40,
	 GF_AFTER_V3 | GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALLING | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE, "00.LFL"},

	/* Scumm Version 4 */
	{"monkeyEGA", "Monkey Island 1 (EGA)", GID_MONKEY_EGA, 4, 0, 62,
	 GF_AFTER_V4 | GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_ADLIB_DEFAULT, "000.LFL"},
	{"pass", "Passport to Adventure", GID_MONKEY_EGA, 4, 0, 62,
	 GF_AFTER_V4 | GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_ADLIB_DEFAULT, "000.LFL"},

	/* Scumm version 5 */
	{"monkeyVGA", "Monkey Island 1 (256 color Floppy version)", GID_MONKEY_VGA,  5, 0, 16,
	 GF_AFTER_V4 | GF_SMALL_HEADER | GF_USE_KEY | GF_ADLIB_DEFAULT, "000.LFL"},
	{"loomcd", "Loom (256 color CD version)", GID_LOOM256, 5, 1, 42,
	 GF_AFTER_V4 | GF_SMALL_HEADER | GF_USE_KEY | GF_AUDIOTRACKS | GF_ADLIB_DEFAULT, "000.LFL"},
	{"monkey", "Monkey Island 1", GID_MONKEY, 5, 2, 2,
	 GF_AFTER_V5 | GF_USE_KEY | GF_AUDIOTRACKS | GF_ADLIB_DEFAULT, 0},
	{"monkey1", "Monkey Island 1 (alt)", GID_MONKEY, 5, 2, 2,
	 GF_AFTER_V5 | GF_USE_KEY | GF_AUDIOTRACKS | GF_ADLIB_DEFAULT, 0},
	{"monkey2", "Monkey Island 2: LeChuck's revenge", GID_MONKEY2, 5, 2, 2,
	 GF_AFTER_V5 | GF_USE_KEY | GF_ADLIB_DEFAULT, 0},
	{"mi2demo", "Monkey Island 2: LeChuck's revenge (Demo)", GID_MONKEY2, 5, 2, 2,
	 GF_AFTER_V5 | GF_USE_KEY | GF_ADLIB_DEFAULT, 0},
	{"atlantis", "Indiana Jones and the Fate of Atlantis", GID_INDY4, 5, 5, 0,
	 GF_AFTER_V5 | GF_USE_KEY | GF_ADLIB_DEFAULT, 0},
	{"playfate", "Indiana Jones and the Fate of Atlantis (Demo)", GID_INDY4, 5, 5, 0,
	 GF_AFTER_V5 | GF_USE_KEY | GF_ADLIB_DEFAULT, 0},
	{"fate", "Indiana Jones and the Fate of Atlantis (Demo)", GID_INDY4, 5, 5, 0,
	 GF_AFTER_V5 | GF_USE_KEY | GF_ADLIB_DEFAULT, 0},

	/* Scumm Version 6 */
	{"puttputt", "Putt-Putt Joins The Parade (DOS)", GID_SAMNMAX, 6, 1, 1,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"puttdemo", "Putt-Putt Joins The Parade (DOS Demo)", GID_PUTTDEMO, 6, 1, 1,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT | GF_HUMONGOUS, 0},
	{"moondemo", "Putt-Putt Goes To The Moon (DOS Demo)", GID_SAMNMAX, 6, 1, 1,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"puttmoon", "Putt-Putt Goes To The Moon (DOS)", GID_SAMNMAX, 6, 1, 1,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"funpack", "Putt-Putt's Fun Pack", GID_SAMNMAX, 6, 1, 1,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"fbpack", "Fatty Bear's Fun Pack", GID_SAMNMAX, 6, 1, 1,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"fbear", "Fatty Bear's Birthday Surprise (DOS)", GID_SAMNMAX, 6, 1, 1,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"fbdemo", "Fatty Bear's Birthday Surprise (DOS Demo)", GID_SAMNMAX, 6, 1, 1,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"tentacle", "Day Of The Tentacle", GID_TENTACLE, 6, 4, 2,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT, 0},
	{"dottdemo", "Day Of The Tentacle (Demo)", GID_TENTACLE, 6, 3, 2,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_ADLIB_DEFAULT, 0},
	{"samnmax", "Sam & Max", GID_SAMNMAX, 6, 4, 2,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER, 0},
	{"samdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, 3, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY  | GF_DRAWOBJ_OTHER_ORDER | GF_ADLIB_DEFAULT, 0},
	{"snmdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, 5, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY  | GF_DRAWOBJ_OTHER_ORDER | GF_ADLIB_DEFAULT, 0},
	{"snmidemo", "Sam & Max (Interactive WIP Demo)", GID_SAMNMAX, 6, 5, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY  | GF_DRAWOBJ_OTHER_ORDER | GF_ADLIB_DEFAULT, 0},

	{"test", "Test demo game", GID_SAMNMAX, 6, 6, 6, GF_NEW_OPCODES | GF_AFTER_V6, 0},

	/* Scumm Version 7 */
	{"ft", "Full Throttle", GID_FT, 7, 3, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_AFTER_V7 | GF_NEW_COSTUMES, 0},
	{"dig", "The Dig", GID_DIG, 7, 5, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_AFTER_V7 | GF_NEW_COSTUMES, 0},

	/* Scumm Version 8 */
	{"comi", "The Curse of Monkey Island", GID_CMI, 8, 1, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_AFTER_V7 | GF_AFTER_V8 | GF_NEW_COSTUMES | GF_DEFAULT_TO_1X_SCALER, 0},

	{NULL, NULL, 0, 0, 0, 0, 0, NULL}
};

const VersionSettings *Engine_SCUMM_targetList() {
	return scumm_settings;
}

Engine *Engine_SCUMM_create(GameDetector *detector, OSystem *syst) {
	Engine *engine;

	if (detector->_amiga)
		detector->_features |= GF_AMIGA;

	if (detector->_features & GF_AFTER_V8)
		engine = new Scumm_v8(detector, syst);
	else if (detector->_features & GF_AFTER_V7)
		engine = new Scumm_v7(detector, syst);
	else if (detector->_features & GF_AFTER_V6)
		engine = new Scumm_v6(detector, syst);
	else if (detector->_features & GF_AFTER_V5)
		engine = new Scumm_v5(detector, syst);
	else if (detector->_features & GF_AFTER_V4)
		engine = new Scumm_v4(detector, syst);
	else if (detector->_features & GF_AFTER_V3)
		engine = new Scumm_v3(detector, syst);
	else if (detector->_features & GF_AFTER_V2)
		engine = new Scumm_v2(detector, syst);
	else
		error("Engine_SCUMM_create(): Unknown version of game engine");

	return engine;
}

Scumm::Scumm (GameDetector *detector, OSystem *syst) 
	: Engine(detector, syst), _pauseDialog(0), _optionsDialog(0), _saveLoadDialog(0) {
	OSystem::Property prop;

	// Use g_scumm from error() ONLY
	g_scumm = this;

	_debugMode = detector->_debugMode;
	_debugLevel = detector->_debugLevel;
	_dumpScripts = detector->_dumpScripts;
	_bootParam = detector->_bootParam;
	_exe_name = (char*)detector->_gameRealName.c_str();
	_game_name = (char*)detector->_gameFileName.c_str();
	_gameId = detector->_gameId;
	_features = detector->_features;
	_noSubtitles = detector->_noSubtitles;
	_defaultTalkDelay = detector->_talkSpeed;
	_use_adlib = detector->_use_adlib;
	_language = detector->_language;
	memset(&res, 0, sizeof(res));
	_allocatedSize = 0;
	_roomResource = 0;
	_lastLoadedRoom = 0;
	_expire_counter = 0;
	_dynamicRoomOffsets = 0;
	_shakeEnabled = 0;
	_hexdumpScripts = 0;
	_showStack = 0;

	if (_gameId == GID_ZAK256) {	// FmTowns is 320x240
		_realWidth = 320;
		_realHeight = 240;
	} else if (_gameId == GID_CMI) {
		_realWidth = 640;
		_realHeight = 480;
	} else {
		_realWidth = 320;
		_realHeight = 200;
	}

	gdi._numStrips = _realWidth / 8;

	_newgui = g_gui;
	_bundle = new Bundle();
	_sound = new Sound(this);
	_timer = Engine::_timer;

	_sound->_sound_volume_master = detector->_master_volume;
	_sound->_sound_volume_sfx = detector->_sfx_volume;	
	_sound->_sound_volume_music = detector->_music_volume;	

	// Override global scaler with any game-specific define
	if (g_config->get("gfx_mode")) {
		prop.gfx_mode = detector->parseGraphicsMode(g_config->get("gfx_mode"));
		syst->property(OSystem::PROP_SET_GFX_MODE, &prop);
	}

	/* Initialize backend */
	syst->init_size(_realWidth, _realHeight);
	prop.cd_num = detector->_cdrom;
	if (prop.cd_num >= 0 && (_features & GF_AUDIOTRACKS))
		syst->property(OSystem::PROP_OPEN_CD, &prop);

	// Override global fullscreen setting with any game-specific define
	if (g_config->getBool("fullscreen", false)) {
		if (!syst->property(OSystem::PROP_GET_FULLSCREEN, 0))
			syst->property(OSystem::PROP_TOGGLE_FULLSCREEN, 0);
	}

#ifndef __GP32__ //ph0x FIXME, "quick dirty hack"
	/* Bind the mixer to the system => mixer will be invoked
	 * automatically when samples need to be generated */	
	_silentDigitalImuse = false;
	if (!_mixer->bindToSystem(syst)) {         
		warning("Sound initialization failed");
		if (detector->_use_adlib) {
			_use_adlib = false;   
			detector->_use_adlib = false;   
			detector->_midi_driver = MD_NULL;   
			warning("Adlib music was selected, switching to midi null driver");   
		}   
		_silentDigitalImuse = true;
		_noDigitalSamples = true;
	} 
	_mixer->setVolume(kDefaultSFXVolume * kDefaultMasterVolume / 255);
	_mixer->setMusicVolume(kDefaultMusicVolume);

	// Init iMuse
	if (_features & GF_AFTER_V7) {
		_imuseDigital = new IMuseDigital(this);
		_imuse = NULL;
	} else {
		_imuseDigital = NULL;
		_imuse = IMuse::create_midi(syst, detector->createMidi());
		if (_imuse) {
			if (detector->_gameTempo != 0)
				_imuse->property(IMuse::PROP_TEMPO_BASE, detector->_gameTempo);
			_imuse->set_music_volume(_sound->_sound_volume_music);
		}
	}
#endif // ph0x-hack

	// Load game from specified slot, if any
	if (detector->_save_slot != -1) {
		_saveLoadSlot = detector->_save_slot;
		_saveLoadFlag = 2;
		_saveLoadCompatible = false;
	}
	loadLanguageBundle();
	_audioNames = NULL;
}

Scumm::~Scumm ()
{
	delete [] _actors;
	
	delete _charset;
	delete _pauseDialog;
	delete _optionsDialog;
	delete _saveLoadDialog;

	delete _bundle;
	delete _sound;
	delete _imuse;
	delete _imuseDigital;
	delete _languageBuffer;
	delete _audioNames;
}

void Scumm::scummInit() {
	int i;
	Actor *a;

	tempMusic = 0;
	debug(9, "scummInit");

	if (_features & GF_AFTER_V7) {
		initScreens(0, 0, _realWidth, _realHeight);
	} else {
		initScreens(0, 16, _realWidth, 144);
	}

	if (_features & GF_16COLOR) {
		byte pal16[16 * 4] = {
			0,   0,  0,  0,   0,   0,  168, 0,   0,   168, 0,  0,   0,   168, 168, 0,
			168, 0,  0,  0,   168, 0,  168, 0,   168, 84,  0,  0,   168, 168, 168, 0,
			84,  84, 84, 0,   84,  84, 168, 0,   0,   252, 0,  0,   0,   252, 252, 0,
			252, 84, 84, 0,   252, 0,  252, 0,   252, 252, 0,  0,   252, 252, 252, 0
		};
		_system->set_palette(pal16, 0, 16);
	}
	
	if (_features & GF_AFTER_V3)
		_charset = new CharsetRendererOld256(this);
	else if (_features & GF_AFTER_V8)
		_charset = new CharsetRendererNut(this);
	else
		_charset = new CharsetRendererClassic(this);

	memset(_charsetData, 0, sizeof(_charsetData));

	if (!(_features & GF_SMALL_NAMES) && !(_features & GF_AFTER_V8))
		loadCharset(1);
	if (_features & GF_OLD_BUNDLE)
		loadCharset(0);	// FIXME - HACK ?

	setShake(0);
	setupCursor();
	
	// Allocate and Initialize actors
	_actors = new Actor[NUM_ACTORS];
	for (i = 1; i < NUM_ACTORS; i++) {
		a = derefActor(i);
		a->number = i;
		a->initActorClass(this);
		a->initActor(1);
	}

	_numNestedScripts = 0;
	vm.cutSceneStackPointer = 0;

	memset(vm.cutScenePtr, 0, sizeof(vm.cutScenePtr));
	memset(vm.cutSceneData, 0, sizeof(vm.cutSceneData));

	for (i = 0; i < _maxVerbs; i++) {
		_verbs[i].verbid = 0;
		_verbs[i].right = _realWidth - 1;
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
		// Setup light
		_flashlightXStrips = 7;
		_flashlightYStrips = 7;
		_flashlight.buffer = NULL;
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

	_charsetBufPos = 0;
	_haveMsg = 0;

	_varwatch = -1;
	_screenStartStrip = 0;

	_talkDelay = 0;
	_keepText = false;

	_currentCursor = 0;
	_cursor.state = 0;
	_userPut = 0;

	_newEffect = 129;
	_fullRedraw = true;

	clearDrawObjectQueue();

	for (i = 0; i < 6; i++) {
		if (_features & GF_AFTER_V3) { // GF_OLD256 or GF_AFTER_V3 ?
			_string[i].t_xpos = 0;
			_string[i].t_ypos = 0;
		} else {
			_string[i].t_xpos = 2;
			_string[i].t_ypos = 5;
		}
		_string[i].t_right = _realWidth - 1;
		_string[i].t_color = 0xF;
		_string[i].t_center = 0;
		_string[i].t_charset = 0;
	}

	_numInMsgStack = 0;

	createResource(rtTemp, 6, 500);

	initScummVars();

	getGraphicsPerformance();

	_sound->_current_cache = 0;

	_lastSaveTime = _system->get_msecs();
}


void Scumm::initScummVars() {
	if (!(_features & GF_AFTER_V6))
		_vars[VAR_V5_TALK_STRING_Y] = -0x50;

	if (!(_features & GF_AFTER_V7)) {
		_vars[VAR_CURRENTDRIVE] = 0;
		_vars[VAR_FIXEDDISK] = true;
		_vars[VAR_SOUNDCARD] = 3;
		_vars[VAR_VIDEOMODE] = 0x13;
		_vars[VAR_HEAPSPACE] = 1400;
		_vars[VAR_MOUSEPRESENT] = true; // FIXME - used to be 0, but that seems odd?!?
//		if (_features & GF_HUMONGOUS) // FIXME uncomment when XMI support is added
//			_vars[VAR_SOUNDPARAM] = 1; // soundblaster for music
//		else
			_vars[VAR_SOUNDPARAM] = 0;
		_vars[VAR_SOUNDPARAM2] = 0;
		_vars[VAR_SOUNDPARAM3] = 0;

		if (_features & GF_AFTER_V6)
			_vars[VAR_V6_EMSSPACE] = 10000;

		_vars[VAR_V5_DRAWFLAGS] = 11;
		_vars[VAR_59] = 3;

		// Setup light
		_vars[VAR_CURRENT_LIGHTS] = LIGHTMODE_actor_base | LIGHTMODE_actor_color | LIGHTMODE_screen;
	} else {
		_vars[VAR_V6_EMSSPACE] = 10000;
	}

	if (_features & GF_AFTER_V8) {	// Fixme: How do we deal with non-cd installs?
		_vars[VAR_CURRENTDISK] = 1;
		_vars[VAR_LANGUAGE] = _language;
	}

	_vars[VAR_CHARINC] = 4;
	_vars[VAR_TALK_ACTOR] = 0;
}

void Scumm::checkRange(int max, int min, int no, const char *str) {
	if (no < min || no > max) {
		char buf[1024];
		sprintf(buf, str, no);
		error("Value %d is out of bounds (%d,%d) in script %d (%s)", no, min,
					max, vm.slot[_curExecScript].number, buf);
	}
}

int Scumm::scummLoop(int delta) {
	static int counter = 0;

#ifndef _WIN32_WCE
	if (_debugger)
		_debugger->on_frame();
#endif

	// Randomize the PRNG by calling it at regular intervals. This ensures
	// that it will be in a different state each time you run the program.
	_rnd.getRandomNumber(2);

	_vars[VAR_TMR_1] += delta;
	_vars[VAR_TMR_2] += delta;
	_vars[VAR_TMR_3] += delta;
	_vars[VAR_TMR_4] += delta;

	if (delta > 15)
		delta = 15;

	decreaseScriptDelay(delta);

	// If _talkDelay is -1, that means the text should never time out.
	// This is used for drawing verb texts, e.g. the Full Throttle
	// dialogue choices.

	if (_talkDelay != -1) {
		_talkDelay -= delta;
		if (_talkDelay < 0)
			_talkDelay = 0;
	}

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

	if (_gameId == GID_MONKEY_VGA) {
		// FIXME: Is all this really necessary now?
		if (delta == 1)
			_vars[VAR_MI1_TIMER]++;
		else if (++counter != 2)
			_vars[VAR_MI1_TIMER] += 5;
		else {
			counter = 0;
			_vars[VAR_MI1_TIMER] += 6;
		}
	} else if (_features & GF_AUDIOTRACKS) {
		_vars[VAR_MI1_TIMER] = _sound->readCDTimer();
	} else if ((_features & GF_OLD256) || (_features & GF_16COLOR)) {

		if(tempMusic == 3) {
			tempMusic = 0;
			_vars[VAR_MUSIC_FLAG]++;
		} else {
			tempMusic++;
		}
	}

	// Trigger autosave all 5 minutes.
	if (!_saveLoadFlag && _system->get_msecs() > _lastSaveTime + 5 * 60 * 1000) {
		_saveLoadSlot = 0;
		sprintf(_saveLoadName, "Autosave %d", _saveLoadSlot);
		_saveLoadFlag = 1;
		_saveLoadCompatible = false;
	}

	_vars[VAR_GAME_LOADED] = 0;
	if (_saveLoadFlag) {
load_game:
		bool success;
		const char *errMsg = "Succesfully saved/loaded game state in file:\n\n%s";
		char filename[256];

		if (_saveLoadFlag == 1) {
			success = saveState(_saveLoadSlot, _saveLoadCompatible);
			if (!success)
				errMsg = "Failed to save game state to file:\n\n%s";

			// Ender: Disabled for small_header games, as can overwrite game
			//  variables (eg, Zak256 cashcard values). Temp disabled for V8
			// because of odd timing issue with scripts and the variable reset
			if (success && _saveLoadCompatible && !(_features & GF_SMALL_HEADER) && !(_features & GF_AFTER_V8))
				_vars[VAR_GAME_LOADED] = 201;
		} else {
			success = loadState(_saveLoadSlot, _saveLoadCompatible);
			if (!success)
				errMsg = "Failed to load game state from file:\n\n%s";

			// Ender: Disabled for small_header games, as can overwrite game
			//  variables (eg, Zak256 cashcard values).
			if (success && _saveLoadCompatible && !(_features & GF_SMALL_HEADER))
				_vars[VAR_GAME_LOADED] = 203;
		}

		makeSavegameName(filename, _saveLoadSlot, _saveLoadCompatible);
		if (!success) {
			displayError(false, errMsg, filename);
		} else if (_saveLoadFlag == 1 && _saveLoadSlot != 0 && !_saveLoadCompatible) {
			// Display "Save succesful" message, except for auto saves
			char buf[1024];
			sprintf(buf, errMsg, filename);
			
			Dialog *dialog = new MessageDialog(_newgui, buf, 1500, false);
			runDialog(dialog);
			delete dialog;
		}
		_saveLoadFlag = 0;
		_lastSaveTime = _system->get_msecs();
	}

	if (_completeScreenRedraw) {
		_completeScreenRedraw = false;
		gdi.clearUpperMask();
		_charset->_hasMask = false;
		for (int i = 0; i < _maxVerbs; i++)
			drawVerb(i, 0);
		verbMouseOver(0);
		_verbRedraw = false;
		_fullRedraw = true;
	}

	runAllScripts();
	checkExecVerbs();
	checkAndRunSentenceScript();

	// HACK: If a load was requested, immediately perform it. This avoids
	// drawing the current room right after the load is request but before
	// it is performed. That was annoying esp. if you loaded while a SMUSH 
	// cutscene was playing.
	if (_saveLoadFlag && _saveLoadFlag != 1) {
		goto load_game;
	}
	
	if (_currentRoom == 0) {
		gdi._cursorActive = 0;
		CHARSET_1();
		drawDirtyScreenParts();
		_sound->processSoundQues();
		camera._last = camera._cur;
	} else {
		walkActors();
		moveCamera();
		fixObjectFlags();
		CHARSET_1();

		if (camera._cur.x != camera._last.x || _BgNeedsRedraw || _fullRedraw
				|| (_features & GF_AFTER_V7 && camera._cur.y != camera._last.y)) {
			redrawBGAreas();
		}

		processDrawQue();

		if (_verbRedraw) {
			redrawVerbs();
		}
		
		setActorRedrawFlags(true, true);
		resetActorBgs();

		if (!(_vars[VAR_CURRENT_LIGHTS] & LIGHTMODE_screen) &&
		      _vars[VAR_CURRENT_LIGHTS] & LIGHTMODE_flashlight) {
			drawFlashlight();
			setActorRedrawFlags(true, false);
		}

		processActors();
		_fullRedraw = false;
		cyclePalette();
		palManipulate();

		if (_doEffect) {
			_doEffect = false;
			fadeIn(_newEffect);
			clearClickedStatus();
		}

		if (!_verbRedraw && _cursor.state > 0) {
			verbMouseOver(checkMouseOver(mouse.x, mouse.y));
		}
		_verbRedraw = false;

		drawBlastObjects();
		if (_features & GF_AFTER_V8)
			processUpperActors();
		drawBlastTexts();
		drawDirtyScreenParts();
		removeBlastObjects();
		removeBlastTexts();

		if (!(_features & GF_AFTER_V6))
			playActorSounds();

		_sound->processSoundQues();
		camera._last = camera._cur;
	}

	if (!(++_expire_counter)) {
		increaseResourceCounter();
	}

	animateCursor();
	
	/* show or hide mouse */
	_system->show_mouse(_cursor.state > 0);

	_vars[VAR_TIMER] = 0;
	return _vars[VAR_TIMER_NEXT];

}

void Scumm::startScene(int room, Actor * a, int objectNr) {
	int i, where;
	Actor *at;

	CHECK_HEAP;
	debug(1, "Loading room %d", room);

	clearMsgQueue();

	fadeOut(_switchRoomEffect2);
	_newEffect = _switchRoomEffect;

	ScriptSlot *ss =  &vm.slot[_currentScript];

	if (_currentScript != 0xFF) {
		if (ss->where == WIO_ROOM || ss->where == WIO_FLOBJECT) {
			if (ss->cutsceneOverride != 0)
				error("Object %d stopped with active cutscene/override in exit", ss->number);
			_currentScript = 0xFF;
		} else if (ss->where == WIO_LOCAL) {
			// HACK to make Indy3 Demo work
			if (ss->cutsceneOverride != 0 && !(_gameId == GID_INDY3 && _roomResource == 3))
				error("Script %d stopped with active cutscene/override in exit", ss->number);
			_currentScript = 0xFF;
		}
	}

	if (!(_features & GF_SMALL_HEADER))  // Disable for SH games. Overwrites
		_vars[VAR_NEW_ROOM] = room; // gamevars, eg Zak cashcards

	runExitScript();
	killScriptsAndResources();
	clearEnqueue();
	stopCycle(0);

	for (i = 1; i < NUM_ACTORS; i++) {
		at = derefActor(i);
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

	if (room >= 0x80 &&  !(_features & GF_AFTER_V7))
		_roomResource = _resourceMapper[room & 0x7F];
	else
		_roomResource = room;

	_vars[VAR_ROOM_RESOURCE] = _roomResource;

	if (room != 0)
		ensureResourceLoaded(rtRoom, room);

	clearRoomObjects();

	if (_currentRoom == 0) {
		_ENCD_offs = _EXCD_offs = 0;
		_numObjectsInRoom = 0;
		return;
	}

	initRoomSubBlocks();
	if (_features & GF_OLD_BUNDLE)
		loadRoomObjectsOldBundle();
	else if (_features & GF_SMALL_HEADER)
		loadRoomObjectsSmall();
	else
		loadRoomObjects();

	if (!(_features & GF_AFTER_V7)) {
		camera._mode = CM_NORMAL;
		camera._cur.x = camera._dest.x = _realWidth / 2;
		camera._cur.y = camera._dest.y = _realHeight / 2;
	}

	if (_features & GF_AFTER_V6) {
		_vars[VAR_V6_SCREEN_WIDTH] = _scrWidth;
		_vars[VAR_V6_SCREEN_HEIGHT] = _scrHeight;
	}

	_vars[VAR_CAMERA_MIN_X] = _realWidth / 2;
	_vars[VAR_CAMERA_MAX_X] = _scrWidth - (_realWidth / 2);

	if (_features & GF_AFTER_V7) {
		_vars[VAR_CAMERA_MIN_Y] = _realHeight / 2;
		_vars[VAR_CAMERA_MAX_Y] = _scrHeight - (_realHeight / 2);
		setCameraAt(_realWidth / 2, _realHeight / 2);
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
			a = derefActorSafe(camera._follows, "startScene: follows");
			setCameraAt(a->x, a->y);
		}
	}

	_doEffect = true;

	CHECK_HEAP;
}

void Scumm::initRoomSubBlocks() {
	int i, offs;
	byte *ptr;
	byte *roomptr, *searchptr, *roomResPtr;
	RoomHeader *rmhd;

	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_CLUT_offs = 0;
	_PALS_offs = 0;

	nukeResource(rtMatrix, 1);
	nukeResource(rtMatrix, 2);

	for (i = 1; i < _maxScaleTable; i++)
		nukeResource(rtScaleTable, i);

	memset(_localScriptList, 0, sizeof(_localScriptList));

	memset(_extraBoxFlags, 0, sizeof(_extraBoxFlags));

	// Determine the room and room script base address
	roomResPtr = roomptr = getResourceAddress(rtRoom, _roomResource);
	if (_features & GF_AFTER_V8)
		roomResPtr = getResourceAddress(rtRoomScripts, _roomResource);
	if (!roomptr || !roomResPtr)
		error("Room %d: data not found (" __FILE__  ":%d)", _roomResource, __LINE__);

	//
	// Determine the room dimensions (width/height)
	//
	if (_features & GF_OLD_BUNDLE)
		rmhd = (RoomHeader *)(roomptr + 4);
	else
		rmhd = (RoomHeader *)findResourceData(MKID('RMHD'), roomptr);
	
	if (_features & GF_AFTER_V8) {
		_scrWidth = READ_LE_UINT32(&(rmhd->v8.width));
		_scrHeight = READ_LE_UINT32(&(rmhd->v8.height));
	} else if (_features & GF_AFTER_V7) {
		_scrWidth = READ_LE_UINT16(&(rmhd->v7.width));
		_scrHeight = READ_LE_UINT16(&(rmhd->v7.height));
	} else {
		_scrWidth = READ_LE_UINT16(&(rmhd->old.width));
		_scrHeight = READ_LE_UINT16(&(rmhd->old.height));
	}

	//
	// Find the room image data
	//
	if (_features & GF_OLD_BUNDLE)
		_IM00_offs = READ_LE_UINT16(roomptr + 0x0A);
	else if (_features & GF_SMALL_HEADER)
		_IM00_offs = findResourceData(MKID('IM00'), roomptr) - roomptr;
	else if (_features & GF_AFTER_V8) {
		ptr = findResource(MKID('IMAG'), roomptr);
		assert(ptr);
		ptr = findResource(MKID('WRAP'), ptr);
		assert(ptr);
		ptr = findResource(MKID('OFFS'), ptr);
		assert(ptr);
		// Get the address of the first SMAP (corresponds to IM00)
		ptr += READ_LE_UINT32(ptr + 8);
		_IM00_offs = ptr - roomptr;
	} else
		_IM00_offs =
			findResource(MKID('IM00'), findResource(MKID('RMIM'), roomptr)) - roomptr;

	//
	// Look for an exit script
	//
	int EXCD_len = -1;
	if (_features & GF_AFTER_V2) {
		_EXCD_offs = READ_LE_UINT16(roomptr + 0x18);
		EXCD_len = READ_LE_UINT16(roomptr + 0x1A) - _EXCD_offs + _resourceHeaderSize;	// HACK
	} else if (_features & GF_OLD_BUNDLE) {
		_EXCD_offs = READ_LE_UINT16(roomptr + 0x19);
		EXCD_len = READ_LE_UINT16(roomptr + 0x1B) - _EXCD_offs + _resourceHeaderSize;	// HACK
	} else {
		ptr = findResourceData(MKID('EXCD'), roomResPtr);
		if (ptr)
			_EXCD_offs = ptr - roomResPtr;
	}
	if (_dumpScripts && _EXCD_offs)
		dumpResource("exit-", _roomResource, roomResPtr + _EXCD_offs - _resourceHeaderSize, EXCD_len);

	//
	// Look for an entry script
	//
	int ENCD_len = -1;
	if (_features & GF_AFTER_V2) {
		_ENCD_offs = READ_LE_UINT16(roomptr + 0x1A);
	} else if (_features & GF_OLD_BUNDLE) {
		_ENCD_offs = READ_LE_UINT16(roomptr + 0x1B);
		// FIXME - the following is a hack which assumes that immediately after
		// the entry script the first local script follows.
		int num_objects = *(roomResPtr + 20);
		int num_sounds = *(roomResPtr + 23);
		int num_scripts = *(roomResPtr + 24);
		ptr = roomptr + 29 + num_objects * 4 + num_sounds + num_scripts;
		if (*ptr)
			ENCD_len = READ_LE_UINT16(ptr + 1) - _ENCD_offs + _resourceHeaderSize; // HACK
	} else {
		ptr = findResourceData(MKID('ENCD'), roomResPtr);
		if (ptr)
			_ENCD_offs = ptr - roomResPtr;
	}
	if (_dumpScripts && _ENCD_offs)
		dumpResource("entry-", _roomResource, roomResPtr + _ENCD_offs - _resourceHeaderSize, ENCD_len);

	//
	// Load box data
	//
	if (_features & GF_SMALL_HEADER) {
		if (_features & GF_AFTER_V2)
			ptr = roomptr + *(roomptr + 0x15);
		else if (_features & GF_OLD_BUNDLE)
			ptr = roomptr + READ_LE_UINT16(roomptr + 0x15);
		else
			ptr = findResourceData(MKID('BOXD'), roomptr);
		if (ptr) {
			byte numOfBoxes = *ptr;
			int size;
			if (_features & GF_AFTER_V3)
				size = numOfBoxes * (SIZEOF_BOX - 2) + 1;
			else
				size = numOfBoxes * SIZEOF_BOX + 1;

			createResource(rtMatrix, 2, size);
			memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
			ptr += size;
			if (_features & GF_AFTER_V2)
				size = (READ_LE_UINT16(roomptr + 0x0A) - *(roomptr + 0x15)) - size;
			else if (_features & GF_OLD_BUNDLE)
				// FIXME. This is an evil HACK!!!
				size = (READ_LE_UINT16(roomptr + 0x0A) - READ_LE_UINT16(roomptr + 0x15)) - size;
			else
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

	//
	// Load scale data
	//
	if (_features & GF_OLD_BUNDLE)
		ptr = 0;	// TODO ?
	else
		ptr = findResourceData(MKID('SCAL'), roomptr);
	if (ptr) {
		offs = ptr - roomptr;
		if (_features & GF_AFTER_V8) {
			for (i = 1; i < _maxScaleTable; i++, offs += 16) {
				int scale1 = READ_LE_UINT32(roomptr + offs);
				int y1 = READ_LE_UINT32(roomptr + offs + 4);
				int scale2 = READ_LE_UINT32(roomptr + offs + 8);
				int y2 = READ_LE_UINT32(roomptr + offs + 12);
				setScaleSlot(i, 0, y1, scale1, 0, y2, scale2);
			}
		} else {
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
	}

	//
	// Setup local scripts
	//

	// Determine the room script base address
	roomResPtr = roomptr = getResourceAddress(rtRoom, _roomResource);
	if (_features & GF_AFTER_V8)
		roomResPtr = getResourceAddress(rtRoomScripts, _roomResource);
	searchptr = roomResPtr;

	if (_features & GF_OLD_BUNDLE) {
		int num_objects = *(roomResPtr + 20);
		int num_sounds;
		int num_scripts;

		if (_features & GF_AFTER_V2) {
			num_sounds = *(roomResPtr + 22);
			num_scripts = *(roomResPtr + 23);
			ptr = roomptr + 28 + num_objects * 4;
			while (num_sounds--)
				loadResource(rtSound, *ptr++);
			while (num_scripts--)
				loadResource(rtScript, *ptr++);
		} else if (_features & GF_AFTER_V3) {
			num_sounds = *(roomResPtr + 23);
			num_scripts = *(roomResPtr + 24);
			ptr = roomptr + 29 + num_objects * 4 + num_sounds + num_scripts;
			while (*ptr) {
				int id = *ptr;

				_localScriptList[id - _numGlobalScripts] = READ_LE_UINT16(ptr + 1);
				ptr += 3;
				
				if (_dumpScripts) {
					char buf[32];
					sprintf(buf, "room-%d-", _roomResource);

					// HACK: to determine the sizes of the local scripts, we assume that
					// a) their order in the data file is the same as in the index
					// b) the last script at the same time is the last item in the room "header"
					int len = - _localScriptList[id - _numGlobalScripts] + _resourceHeaderSize;
					if (*ptr)
						len += READ_LE_UINT16(ptr + 1);
					else
						len += READ_LE_UINT16(roomResPtr);
					dumpResource(buf, id, roomResPtr + _localScriptList[id - _numGlobalScripts] - _resourceHeaderSize, len);
				}
			}
		}
	} else if (_features & GF_SMALL_HEADER) {
		while ((ptr = findResourceSmall(MKID('LSCR'), searchptr)) != NULL) {
			int id = 0;
			ptr += _resourceHeaderSize;	/* skip tag & size */
			id = ptr[0];

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}

			_localScriptList[id - _numGlobalScripts] = ptr + 1 - roomptr;
			searchptr = NULL;
		}
	} else {
		while ((ptr = findResource(MKID('LSCR'), searchptr)) != NULL) {
			int id = 0;

			ptr += _resourceHeaderSize;	/* skip tag & size */

			if (_features & GF_AFTER_V8) {
				id = READ_LE_UINT32(ptr);
				checkRange(NUM_LOCALSCRIPT + _numGlobalScripts, _numGlobalScripts, id, "Invalid local script %d");
				_localScriptList[id - _numGlobalScripts] = ptr + 4 - roomResPtr;
			} else if (_features & GF_AFTER_V7) {
				id = READ_LE_UINT16(ptr);
				checkRange(NUM_LOCALSCRIPT + _numGlobalScripts, _numGlobalScripts, id, "Invalid local script %d");
				_localScriptList[id - _numGlobalScripts] = ptr + 2 - roomResPtr;
			} else {
				id = ptr[0];
				_localScriptList[id - _numGlobalScripts] = ptr + 1 - roomResPtr;
			}

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}

			searchptr = NULL;
		}
	}

	// FIXME - we could simply always call findResourceData here, it will
	// do the right thing even if GF_SMALL_HEADER is set. But then, we have
	// to change setPaletteFromPtr() (easy). The problematic bit is save game
	// compatibility - _CLUT_offs is stored in the save game after all.
	// Of course we could just decide to not use _CLUT_offs anymore, and change
	// setPaletteFromRes() to invoke findResourceData() each time
	// (and also getPalettePtr()).
	if (_features & GF_OLD_BUNDLE)
		ptr = 0; // TODO ? do 16 bit games use a palette?!?
	else if (_features & GF_SMALL_HEADER)
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

	// Color cycling
	if (_features & GF_OLD_BUNDLE)
		ptr = 0; // TODO / FIXME ???
	else
		ptr = findResourceData(MKID('CYCL'), roomptr);
	if (ptr)
		initCycl(ptr);

	// Transparent color
	if (_features & GF_OLD_BUNDLE)
		gdi._transparentColor = 255;	// TODO - FIXME
	else {
		ptr = findResourceData(MKID('TRNS'), roomptr);
		if (ptr)
			gdi._transparentColor = ptr[0];
		else if (_features & GF_AFTER_V8)
			gdi._transparentColor = 5;	// FIXME
		else
			gdi._transparentColor = 255;
	}

	initBGBuffers(_scrHeight);
}

void Scumm::setScaleItem(int slot, int a, int b, int c, int d) {
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

void Scumm::setScaleSlot(int slot, int x1, int y1, int scale1, int x2, int y2, int scale2) {
	assert(1 <= slot && slot <= 20);
	_scaleSlots[slot-1].x2 = x2;
	_scaleSlots[slot-1].y2 = y2;
	_scaleSlots[slot-1].scale2 = scale2;
	_scaleSlots[slot-1].x1 = x1;
	_scaleSlots[slot-1].y1 = y1;
	_scaleSlots[slot-1].scale1 = scale1;
}

void Scumm::dumpResource(char *tag, int idx, byte *ptr, int length) {
	char buf[256];
	File out;

	uint32 size;
	if (length >= 0)
		size = length;
	else if (_features & GF_OLD_BUNDLE)
		size = READ_LE_UINT16(ptr);
	else if (_features & GF_SMALL_HEADER)
		size = READ_LE_UINT32(ptr);
	else
		size = READ_BE_UINT32_UNALIGNED(ptr + 4);

#if defined(MACOS_CARBON)
	sprintf(buf, ":dumps:%s%d.dmp", tag, idx);
#else
	sprintf(buf, "dumps/%s%d.dmp", tag, idx);
#endif

	out.open(buf, "", 1);
	if (out.isOpen() == false) {
		out.open(buf, "", 2);
		if (out.isOpen() == false)
			return;
		out.write(ptr, size);
	}
	out.close();
}

void Scumm::clearClickedStatus() {
	checkKeyHit();
	_mouseButStat = 0;
	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;
}

int Scumm::checkKeyHit() {
	int a = _keyPressed;
	_keyPressed = 0;
	return a;
}

void Scumm::pauseGame(bool user) {
	pauseDialog();
}

void Scumm::setOptions() {
	//_newgui->optionsDialog();
}

int Scumm::runDialog(Dialog *dialog) {
	// Pause sound put
	bool old_soundsPaused = _sound->_soundsPaused;
	_sound->pauseSounds(true);

	// Pause playing smush movie
	_smushPlay = false;

	// Open & run the dialog
	int result = dialog->runModal();

	// Restore old cursor
	updateCursor();

	// Restore playing smush movie
	_smushPlay = true;

	// Resume sound output
	_sound->pauseSounds(old_soundsPaused);
	
	// Return the result
	return result;
}

void Scumm::pauseDialog() {
	if (!_pauseDialog)
		_pauseDialog = new PauseDialog(_newgui, this);
	runDialog(_pauseDialog);
}

void Scumm::saveloadDialog() {
	if (!_saveLoadDialog)
		_saveLoadDialog = new SaveLoadDialog(_newgui, this);
	runDialog(_saveLoadDialog);
}

void Scumm::optionsDialog() {
	if (!_optionsDialog)
		_optionsDialog = new OptionsDialog(_newgui, this);
	runDialog(_optionsDialog);
}

char Scumm::displayError(bool showCancel, const char *message, ...) {
	char buf[1024], result;
	va_list va;

	va_start(va, message);
	vsprintf(buf, message, va);
	va_end(va);

	Dialog *dialog = new MessageDialog(_newgui, buf, 0, true, showCancel);
	result = runDialog(dialog);
	delete dialog;

	return result;
}

void Scumm::shutDown(int i) {
	/* TODO: implement this */
	warning("shutDown: not implemented");
}

void Scumm::processKbd() {
	int saveloadkey;
	getKeyInput();

	if ((_features & GF_OLD256) || (_gameId == GID_CMI) || (_features & GF_OLD_BUNDLE)) /* FIXME: Support ingame screen */
		saveloadkey = 319;
	else
		saveloadkey = _vars[VAR_SAVELOADDIALOG_KEY];

	_virtual_mouse_x = mouse.x + virtscr[0].xstart;

	if(_features & GF_AFTER_V7)
		_virtual_mouse_y = mouse.y + camera._cur.y - (_realHeight / 2);
	else
		_virtual_mouse_y = mouse.y;

	if (!(_features & GF_AFTER_V3))
		_virtual_mouse_y += virtscr[0].topline;
	else
		_virtual_mouse_y -= 16;

	if (_virtual_mouse_y < 0)
		_virtual_mouse_y = -1;

	if (_features & GF_AFTER_V3) {
		if (_virtual_mouse_y >= virtscr[0].height + virtscr[0].topline)
			_virtual_mouse_y = -1;
	} else {
		if (_virtual_mouse_y >= virtscr[0].height)
			_virtual_mouse_y = -1;
	}

	if (!_lastKeyHit)
		return;

	if (_keyScriptNo && (_keyScriptKey == _lastKeyHit)) {
		runScript(_keyScriptNo, 0, 0, 0);
		return;
	}

	if (_lastKeyHit == KEY_SET_OPTIONS) {
		setOptions();
		return;
	}

	if (_lastKeyHit == _vars[VAR_RESTART_KEY]) {
		warning("Restart not implemented");
//		pauseGame(true);
		return;
	}

	if (_lastKeyHit == _vars[VAR_PAUSE_KEY]) {
		pauseGame(true);
		/* pause */
		return;
	}

	if (_lastKeyHit == _vars[VAR_CUTSCENEEXIT_KEY]) {
		if (_insaneState) {
			_videoFinished = true;
		} else
			exitCutscene();
	} else if (_lastKeyHit == saveloadkey && _currentRoom != 0) {
		if (_features & GF_AFTER_V7)
			runScript(_vars[VAR_UNK_SCRIPT], 0, 0, 0);

		saveloadDialog();		// Display NewGui

		if (_features & GF_AFTER_V7)
			runScript(_vars[VAR_UNK_SCRIPT_2], 0, 0, 0);
		return;
	} else if (_lastKeyHit == _vars[VAR_TALKSTOP_KEY]) {
		_talkDelay = 0;
		if (_sound->_sfxMode & 2)
			stopTalk();
		return;
	} else if (_lastKeyHit == '[') { // [ Music volume down
		int vol = _sound->_sound_volume_music;
		if (!(vol & 0xF) && vol)
			vol -= 16;
		vol = vol & 0xF0;
		_sound->_sound_volume_music = vol;
		if (_imuse)
			_imuse->set_music_volume (vol);
	} else if (_lastKeyHit == ']') { // ] Music volume up
		int vol = _sound->_sound_volume_music;
		vol = (vol + 16) & 0xFF0;
		if (vol > 255) vol = 255;
		_sound->_sound_volume_music = vol;
		if (_imuse)
			_imuse->set_music_volume (vol);
	} else if (_lastKeyHit == '-') { // - text speed down
		_defaultTalkDelay += 5;
		if (_defaultTalkDelay > 90)
			_defaultTalkDelay = 90;

		_vars[VAR_CHARINC] = _defaultTalkDelay / 20;
	} else if (_lastKeyHit == '+') { // + text speed up
		_defaultTalkDelay -= 5;
		if (_defaultTalkDelay < 5)
			_defaultTalkDelay = 5;

		_vars[VAR_CHARINC] = _defaultTalkDelay / 20;
	} else if (_lastKeyHit == '~' || _lastKeyHit == '#') { // Debug console
		g_debugger.attach(this, NULL);
	}

	_mouseButStat = _lastKeyHit;
}

int Scumm::getKeyInput() {
	_mouseButStat = 0;

	_lastKeyHit = checkKeyHit();
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
		_lastKeyHit = (uint)_vars[VAR_CUTSCENEEXIT_KEY];
	} else if (_leftBtnPressed & msClicked) {
		_mouseButStat = MBS_LEFT_CLICK;
	} else if (_rightBtnPressed & msClicked) {
		_mouseButStat = MBS_RIGHT_CLICK;
	}

	if (_features & GF_AFTER_V8) {
		_vars[VAR_MOUSE_BUTTONS] = 0;
		_vars[VAR_MOUSE_HOLD] = 0;
		_vars[VAR_RIGHTBTN_HOLD] = 0;

		if (_leftBtnPressed & msClicked)
			_vars[VAR_MOUSE_BUTTONS] += 1;

		if (_rightBtnPressed & msClicked)
			_vars[VAR_MOUSE_BUTTONS] += 2;

		if (_leftBtnPressed & msDown)
			_vars[VAR_MOUSE_HOLD] += 1;

		if (_rightBtnPressed & msDown) {
			_vars[VAR_RIGHTBTN_HOLD] = 1;
			_vars[VAR_MOUSE_HOLD] += 2;
		}
	} else if (_features & GF_AFTER_V7) {
//    _vars[VAR_LEFTBTN_DOWN] = (_leftBtnPressed&msClicked) != 0;
		_vars[VAR_LEFTBTN_HOLD] = (_leftBtnPressed & msDown) != 0;
//    _vars[VAR_RIGHTBTN_DOWN] = (_rightBtnPressed&msClicked) != 0;
		_vars[VAR_RIGHTBTN_HOLD] = (_rightBtnPressed & msDown) != 0;
	}

	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;

	return _lastKeyHit;
}

void Scumm::convertKeysToClicks() {
	if (_lastKeyHit && _cursor.state > 0) {
		if (_lastKeyHit == 9) {
			_mouseButStat = MBS_RIGHT_CLICK;
		} else if (_lastKeyHit == 13) {
			_mouseButStat = MBS_LEFT_CLICK;
		} else
			return;
		_lastKeyHit = 0;
	}
}

Actor *Scumm::derefActor(int id) {
	return &_actors[id];
}

Actor *Scumm::derefActorSafe(int id, const char *errmsg) {
	if (id < 1 || id >= NUM_ACTORS) {
		if (_debugLevel > 1)
		warning
			("Invalid actor %d in %s (script %d, opcode 0x%x) - This is potentially a BIG problem.",
			 id, errmsg, vm.slot[_curExecScript].number, _opcode);
		return NULL;
	}
	return derefActor(id);
}

void Scumm::setStringVars(int slot) {
	StringTab *st = &_string[slot];
	st->xpos = st->t_xpos;
	st->ypos = st->t_ypos;
	st->center = st->t_center;
	st->overhead = st->t_overhead;
	st->no_talk_anim = st->t_no_talk_anim;
	st->right = st->t_right;
	st->color = st->t_color;
	st->charset = st->t_charset;
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

//
// Convert an old style direction to a new style one (angle),
//
int newDirToOldDir(int dir) {
	if (dir >= 71 && dir <= 109)
		return 1;
	if (dir >= 109 && dir <= 251)
		return 2;
	if (dir >= 251 && dir <= 289)
		return 0;
	return 3;
}

//
// Convert an new style (angle) direction to an old style one.
//
int oldDirToNewDir(int dir) {
	assert(0 <= dir && dir <= 3);
	const int new_dir_table[4] = { 270, 90, 180, 0 };
	return new_dir_table[dir];
}

//
// Convert an angle to a simple direction.
//
int toSimpleDir(int dirType, int dir) {
	if (dirType) {
		const int16 directions[] = { 22,  72, 107, 157, 202, 252, 287, 337 };
		for (int i = 0; i < 7; i++)
			if (dir >= directions[i] && dir <= directions[i+1])
				return i+1;
	} else {
		const int16 directions[] = { 71, 109, 251, 289 };
		for (int i = 0; i < 3; i++)
			if (dir >= directions[i] && dir <= directions[i+1])
				return i+1;
	}

	return 0;
}

//
// Convert a simple direction to an angle
//
int fromSimpleDir(int dirType, int dir) {
	if (dirType)
		return dir * 45;
	else
		return dir * 90;
}

//
// Normalize the given angle - that means, ensure it is positive, and
// change it to the closest multiple of 45 degree by abusing toSimpleDir.
//
int normalizeAngle(int angle) {
	int temp;

	temp = (angle + 360) % 360;

	return toSimpleDir(1, temp) * 45;
}

void Scumm::errorString(const char *buf1, char *buf2) {
	if (_currentScript != 0xFF) {
		ScriptSlot *ss = &vm.slot[_currentScript];
		sprintf(buf2, "(%d:%d:0x%X): %s", _roomResource,
			ss->number, _scriptPointer - _scriptOrgPointer, buf1);
	} else {
		strcpy(buf2, buf1);
	}

        // Unless an error -originated- within the debugger, spawn the debugger. Otherwise
        // exit out normally.
        if (!_debugger) {
                printf("%s", buf2);	// (Print it again in-case debugger segfaults)
                g_debugger.attach(this, buf2);
                g_debugger.on_frame();
        }

}

void Scumm::waitForTimer(int msec_delay) {
	uint32 start_time;

	if (_fastMode & 2)
		msec_delay = 0;
	else if (_fastMode & 1)
		msec_delay = 10;

	start_time = _system->get_msecs();

	for (;;) {
		parseEvents();

		_sound->updateCD(); // Loop CD Audio if needed
		if (_system->get_msecs() >= start_time + msec_delay)
			break;
		_system->delay_msecs(10);
	}
}

void Scumm::parseEvents() {
	OSystem::Event event;

	while (_system->poll_event(&event)) {

		switch(event.event_code) {
		case OSystem::EVENT_KEYDOWN:
			if (event.kbd.keycode >= '0' && event.kbd.keycode<='9'
				&& (event.kbd.flags == OSystem::KBD_ALT ||
					event.kbd.flags == OSystem::KBD_CTRL)) {
				_saveLoadSlot = event.kbd.keycode - '0';

				//  don't overwrite autosave (slot 0)
				if (_saveLoadSlot == 0)
					_saveLoadSlot = 10;

				sprintf(_saveLoadName, "Quicksave %d", _saveLoadSlot);
				_saveLoadFlag = (event.kbd.flags == OSystem::KBD_ALT) ? 1 : 2;
				_saveLoadCompatible = false;
			} else if (event.kbd.flags==OSystem::KBD_CTRL) {
				if (event.kbd.keycode == 'f')
					_fastMode ^= 1;
				else if (event.kbd.keycode == 'g')
					_fastMode ^= 2;
				else if ((event.kbd.keycode == 'd') && (!_system->property(OSystem::PROP_GET_FULLSCREEN, 0)))
					g_debugger.attach(this, NULL);
				else if (event.kbd.keycode == 's')
					resourceStats();
				else
					_keyPressed = event.kbd.ascii;	// Normal key press, pass on to the game.
			} else if (event.kbd.flags & OSystem::KBD_ALT) {
				// The result must be 273 for Alt-W
				// because that's what MI2 looks for in
				// its "instant win" cheat.
				_keyPressed = event.kbd.keycode + 154;
			} else 
				_keyPressed = event.kbd.ascii;	// Normal key press, pass on to the game.
			break;

		case OSystem::EVENT_MOUSEMOVE:
			mouse.x = event.mouse.x;
			mouse.y = event.mouse.y;
			_system->set_mouse_pos(event.mouse.x, event.mouse.y);
			_system->update_screen();
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
}

void Scumm::updatePalette() {
	if (_palDirtyMax == -1)
		return;

	bool noir_mode = (_gameId == GID_SAMNMAX && readVar(0x8000));
	int first = _palDirtyMin;
	int num = _palDirtyMax - first + 1;
	int i;

	byte palette_colors[1024];
	byte *p = palette_colors;

	for (i = _palDirtyMin; i <= _palDirtyMax; i++) {
		byte *data;

		if (_features & GF_SMALL_HEADER)
			data = _currentPalette + _shadowPalette[i] * 3;
		else
			data = _currentPalette + i * 3;

		// Sam & Max film noir mode. Convert the colours to grayscale
		// before uploading them to the backend.

		if (noir_mode) {
			int r, g, b;
			byte brightness;

			r = data[0];
			g = data[1];
			b = data[2];

			brightness = (byte)((0.299 * r + 0.587 * g + 0.114 * b) + 0.5);

			*p++ = brightness;
			*p++ = brightness;
			*p++ = brightness;
			*p++ = 0;
		} else {
			*p++ = data[0];
			*p++ = data[1];
			*p++ = data[2];
			*p++ = 0;
		}
	}
	
	_system->set_palette(palette_colors, first, num);

	_palDirtyMax = -1;
	_palDirtyMin = 256;
}

void Scumm::mainRun() {
	int delta = 0;
	int last_time = _system->get_msecs(); 
	int new_time;

	for(;;) {
		updatePalette();
		
		_system->update_screen();		
		new_time = _system->get_msecs();
		waitForTimer(delta * 15 + last_time - new_time);
		last_time = _system->get_msecs();
		delta = scummLoop(delta);
		if (delta < 1)	// Ensure we don't get into a loop
			delta = 1;  // by not decreasing sleepers.
	}
}

void Scumm::launch() {
	gdi._vm = this;

	_maxHeapThreshold = 900000;
	_minHeapThreshold = 400000;

	_verbRedraw = false;

	allocResTypeData(rtBuffer, MKID('NONE'), 10, "buffer", 0);
	initVirtScreen(0, 0, 0, _realWidth, _realHeight, false, false);

	setupScummVars();

	setupOpcodes();

	if (_features & GF_AFTER_V8)
		NUM_ACTORS = 80;
	else if ((_features & GF_AFTER_V7) || (_gameId == GID_SAMNMAX))
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

	if (_features & GF_OLD_BUNDLE)
		_resourceHeaderSize = 4; // FIXME - to be rechecked
	else if (_features & GF_SMALL_HEADER)
		_resourceHeaderSize = 6;
	else
		_resourceHeaderSize = 8;

	readIndexFile();

	scummInit();

	if (!(_features & GF_AFTER_V7))
		_vars[VAR_VERSION] = 21;

	_vars[VAR_DEBUGMODE] = _debugMode;

	if (_gameId == GID_MONKEY)
		_vars[74] = 1225;

	_sound->setupSound();

	runScript(1, 0, 0, &_bootParam);
}

void Scumm::go() {
	launch();
	mainRun();
}
