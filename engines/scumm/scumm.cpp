/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/md5.h"
#include "common/system.h"

#include "gui/message.h"
#include "gui/newgui.h"

#include "scumm/akos.h"
#include "scumm/charset.h"
#include "scumm/costume.h"
#include "scumm/debugger.h"
#include "scumm/dialogs.h"
#include "scumm/file.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/smush/smush_mixer.h"
#include "scumm/insane/insane.h"
#include "scumm/intern.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"
#include "scumm/player_nes.h"
#include "scumm/player_v1.h"
#include "scumm/player_v2.h"
#include "scumm/player_v2a.h"
#include "scumm/player_v3a.h"
#include "scumm/he/resource_he.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/imuse/sysex.h"
#include "scumm/he/sprite_he.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

#include "sound/mixer.h"

#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

#if (defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
namespace Graphics {
	extern void initfonts();
}
#endif

using Common::File;

namespace Scumm {

// Use g_scumm from error() ONLY
ScummEngine *g_scumm = 0;


ScummEngine::ScummEngine(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: Engine(syst),
	  _game(gs),
	  _substResFileName(subst),
	  _debugger(0),
	  _currentScript(0xFF), // Let debug() work on init stage
	  gdi(this),
	  res(this),
	  _pauseDialog(0), _mainMenuDialog(0), _versionDialog(0),
	  _targetName(detector->_targetName) {

	// Copy MD5 checksum
	memcpy(_gameMD5, md5sum, 16);
	
	// Clean _substResFileNameBundle
	memset(&_substResFileNameBundle, 0, sizeof(_substResFileNameBundle));

	// Add default file directories.
	if (((_game.platform == Common::kPlatformAmiga) || (_game.platform == Common::kPlatformAtariST)) && (_game.version <= 4)) {
		// This is for the Amiga version of Indy3/Loom/Maniac/Zak
		File::addDefaultDirectory(_gameDataPath + "ROOMS/");
		File::addDefaultDirectory(_gameDataPath + "rooms/");
	}

	if ((_game.platform == Common::kPlatformMacintosh) && (_game.version == 3)) {
		// This is for the Mac version of Indy3/Loom
		File::addDefaultDirectory(_gameDataPath + "Rooms 1/");
		File::addDefaultDirectory(_gameDataPath + "Rooms 2/");
		File::addDefaultDirectory(_gameDataPath + "Rooms 3/");
	}

#ifndef DISABLE_SCUMM_7_8
#ifdef MACOSX
	if (_game.version == 8 && !memcmp(_gameDataPath.c_str(), "/Volumes/MONKEY3_", 17)) {
		// Special case for COMI on Mac OS X. The mount points on OS X depend
		// on the volume name. Hence if playing from CD, we'd get a problem.
		// So if loading of a resource file fails, we fall back to the (fixed)
		// CD mount points (/Volumes/MONKEY3_1 and /Volumes/MONKEY3_2).
		//
		// This check for whether we play from CD is very crude, though.

		File::addDefaultDirectory("/Volumes/MONKEY3_1/RESOURCE/");
		File::addDefaultDirectory("/Volumes/MONKEY3_1/resource/");
		File::addDefaultDirectory("/Volumes/MONKEY3_2/");
		File::addDefaultDirectory("/Volumes/MONKEY3_2/RESOURCE/");
		File::addDefaultDirectory("/Volumes/MONKEY3_2/resource/");
	} else
#endif
	if (_game.version == 8) {
		// This is for COMI
		File::addDefaultDirectory(_gameDataPath + "RESOURCE/");
		File::addDefaultDirectory(_gameDataPath + "resource/");
	}

	if (_game.version == 7) {
		// This is for Full Throttle & The Dig
		File::addDefaultDirectory(_gameDataPath + "VIDEO/");
		File::addDefaultDirectory(_gameDataPath + "video/");
		File::addDefaultDirectory(_gameDataPath + "DATA/");
		File::addDefaultDirectory(_gameDataPath + "data/");
	}
#endif


	// The 	kGenAsIs method is only used for 'container files', i.e. files
	// that contain the real game files bundled together in an archive format.
	// This is the case of the NES, C64 and Mac versions of certain games.
	// Note: All of these can also occur in 'extracted' form, in which case they
	// are treated like any other SCUMM game.
	if (_substResFileName.almostGameID && _substResFileName.genMethod == kGenAsIs) {

		if (_game.platform == Common::kPlatformNES) {
			// We read data directly from NES ROM instead of extracting it with
			// external tool
			assert(_game.id == GID_MANIAC);
			_fileHandle = new ScummNESFile();
			_containerFile = _substResFileName.expandedName;
		} else if (_game.platform == Common::kPlatformC64) {
			// Read data from C64 disk images.
			const char *tmpBuf1, *tmpBuf2;
			assert(_game.id == GID_MANIAC || _game.id == GID_ZAK);
			if (_game.id == GID_MANIAC) {
				tmpBuf1 = "maniac1.d64";
				tmpBuf2 = "maniac2.d64";
			} else {
				tmpBuf1 = "zak1.d64";
				tmpBuf2 = "zak2.d64";
			}
	
			_fileHandle = new ScummC64File(tmpBuf1, tmpBuf2, _game.id == GID_MANIAC);
			_containerFile = tmpBuf1;
		} else if (_game.platform == Common::kPlatformMacintosh) {
			// The mac versions of Indy4, Sam&Max, DOTT, FT and The Dig used a
			// special meta (container) file format to store the actual SCUMM data
			// files. The rescumm utility used to be used to extract those files. 
			// While that is still possible, we now support reading those files 
			// directly. The first step is to check whether one of them is present
			// (we do that here); the rest is handled by the  ScummFile class and 
			// code in openResourceFile() (and in the Sound class, for MONSTER.SOU
			// handling).
			assert(_game.version >= 5 && _game.heversion == 0);
			_fileHandle = new ScummFile();
			_containerFile = _substResFileName.expandedName;
		} else {
			error("kGenAsIs used with unsupported platform");
		}
		
		// If a container file is used, we can turn of file name substitution.
		_substResFileName.almostGameID = 0;
	} else {
		// Regular access, no container file involved
		_fileHandle = new ScummFile();
	}
	

	// Init all vars
	_imuse = NULL;
	_imuseDigital = NULL;
	_smixer = NULL;
	_musicEngine = NULL;
	_verbs = NULL;
	_objs = NULL;
	_storedFlObjects = NULL;
	_debugFlags = 0;
	_sound = NULL;
	memset(&vm, 0, sizeof(vm));
	_smushVideoShouldFinish = false;
	_smushPaused = false;
	_insaneRunning = false;
	_quit = false;
	_pauseDialog = NULL;
	_mainMenuDialog = NULL;
	_versionDialog = NULL;
	_fastMode = 0;
	_actors = NULL;
	_arraySlot = NULL;
	_inventory = NULL;
	_newNames = NULL;
	_scummVars = NULL;
	_roomVars = NULL;
	_varwatch = 0;
	_bitVars = NULL;
	_numVariables = 0;
	_numBitVariables = 0;
	_numRoomVariables = 0;
	_numLocalObjects = 0;
	_numGlobalObjects = 0;
	_numStoredFlObjects = 0;
	_numArray = 0;
	_numVerbs = 0;
	_numFlObject = 0;
	_numInventory = 0;
	_numRooms = 0;
	_numScripts = 0;
	_numSounds = 0;
	_numCharsets = 0;
	_numNewNames = 0;
	_numGlobalScripts = 0;
	_numCostumes = 0;
	_numImages = 0;
	_numLocalScripts = 60;
	_numSprites = 0;
	_numTalkies = 0;
	_numPalettes = 0;
	_numUnk = 0;
	_curActor = 0;
	_curVerb = 0;
	_curVerbSlot = 0;
	_curPalIndex = 0;
	_currentRoom = 0;
	_egoPositioned = false;
	_keyPressed = 0;
	_lastKeyHit = 0;
	_mouseAndKeyboardStat = 0;
	_leftBtnPressed = 0;
	_rightBtnPressed = 0;
	_bootParam = 0;
	_dumpScripts = false;
	_debugMode = 0;
	_heV7DiskOffsets = NULL;
	_heV7RoomIntOffsets = NULL;
	_objectOwnerTable = NULL;
	_objectRoomTable = NULL;
	_objectStateTable = NULL;
	_numObjectsInRoom = 0;
	_userPut = 0;
	_userState = 0;
	_activeInventory = 0;
	_activeObject = 0;
	_activeVerb = 0;
	_resourceHeaderSize = 8;
	_saveLoadFlag = 0;
	_saveLoadSlot = 0;
	_lastSaveTime = 0;
	_saveTemporaryState = false;
	memset(_saveLoadName, 0, sizeof(_saveLoadName));
	memset(_localScriptOffsets, 0, sizeof(_localScriptOffsets));
	_scriptPointer = NULL;
	_scriptOrgPointer = NULL;
	_opcode = 0;
	vm.numNestedScripts = 0;
	_lastCodePtr = NULL;
	_resultVarNumber = 0;
	_scummStackPos = 0;
	memset(_vmStack, 0, sizeof(_vmStack));
	_keyScriptKey = 0;
	_keyScriptNo = 0;
	_fileOffset = 0;
	memset(_resourceMapper, 0, sizeof(_resourceMapper));
	_lastLoadedRoom = 0;
	_roomResource = 0;
	OF_OWNER_ROOM = 0;
	_verbMouseOver = 0;
	_inventoryOffset = 0;
	_classData = NULL;
	_actorToPrintStrFor = 0;
	_sentenceNum = 0;
	memset(_sentence, 0, sizeof(_sentence));
	memset(_string, 0, sizeof(_string));
	_screenB = 0;
	_screenH = 0;
	_roomHeight = 0;
	_roomWidth = 0;
	_screenHeight = 0;
	_screenWidth = 0;
	memset(virtscr, 0, sizeof(virtscr));
	memset(&camera, 0, sizeof(CameraData));
	memset(_colorCycle, 0, sizeof(_colorCycle));
	memset(_colorUsedByCycle, 0, sizeof(_colorUsedByCycle));
	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_CLUT_offs = 0;
	_EPAL_offs = 0;
	_IM00_offs = 0;
	_PALS_offs = 0;
	_fullRedraw = false;
	_bgNeedsRedraw = false;
	_screenEffectFlag = false;
	_completeScreenRedraw = false;
	memset(&_cursor, 0, sizeof(_cursor));
	memset(_grabbedCursor, 0, sizeof(_grabbedCursor));
	_currentCursor = 0;
	_newEffect = 0;
	_switchRoomEffect2 = 0;
	_switchRoomEffect = 0;
	_scrollBuffer = NULL;

	_doEffect = false;
	_currentLights = 0;
	memset(&_flashlight, 0, sizeof(_flashlight));
	_bompActorPalettePtr = NULL;
	_shakeEnabled = false;
	_shakeFrame = 0;
	_screenStartStrip = 0;
	_screenEndStrip = 0;
	_screenTop = 0;
	_drawObjectQueNr = 0;
	memset(_drawObjectQue, 0, sizeof(_drawObjectQue));
	_palManipStart = 0;
	_palManipEnd = 0;
	_palManipCounter = 0;
	_palManipPalette = NULL;
	_palManipIntermediatePal = NULL;
	memset(gfxUsageBits, 0, sizeof(gfxUsageBits));
	_hePalettes = NULL;
	_shadowPalette = NULL;
	_shadowPaletteSize = 0;
	memset(_currentPalette, 0, sizeof(_currentPalette));
	memset(_darkenPalette, 0, sizeof(_darkenPalette));
	memset(_HEV7ActorPalette, 0, sizeof(_HEV7ActorPalette));
	_palDirtyMin = 0;
	_palDirtyMax = 0;
	_haveMsg = 0;
	_haveActorSpeechMsg = false;
	_useTalkAnims = false;
	_defaultTalkDelay = 0;
	_musicType = MDT_NONE;
	_tempMusic = 0;
	_saveSound = 0;
	memset(_extraBoxFlags, 0, sizeof(_extraBoxFlags));
	memset(_scaleSlots, 0, sizeof(_scaleSlots));
	_charset = NULL;
	_charsetColor = 0;
	memset(_charsetColorMap, 0, sizeof(_charsetColorMap));
	memset(_charsetData, 0, sizeof(_charsetData));
	_charsetBufPos = 0;
	memset(_charsetBuffer, 0, sizeof(_charsetBuffer));
	_copyProtection = false;
	_demoMode = false;
	_confirmExit = false;
	_voiceMode = 0;
	_talkDelay = 0;
	_NES_lastTalkingActor = 0;
	_NES_talkColor = 0;
	_keepText = false;
	_costumeLoader = NULL;
	_costumeRenderer = NULL;
	_2byteFontPtr = 0;
	_V1TalkingActor = 0;
	_NESStartStrip = 0;

	_actorClipOverride.top = 0;
	_actorClipOverride.bottom = 480;
	_actorClipOverride.left = 0;
	_actorClipOverride.right = 640;

	_skipDrawObject = 0;
	memset(_heTimers, 0, sizeof(_heTimers));

	memset(_akosQueue, 0, sizeof(_akosQueue));
	_akosQueuePos = 0;

	//
	// Init all VARS to 0xFF
	//
	VAR_LANGUAGE = 0xFF;
	VAR_KEYPRESS = 0xFF;
	VAR_SYNC = 0xFF;
	VAR_EGO = 0xFF;
	VAR_CAMERA_POS_X = 0xFF;
	VAR_HAVE_MSG = 0xFF;
	VAR_ROOM = 0xFF;
	VAR_OVERRIDE = 0xFF;
	VAR_MACHINE_SPEED = 0xFF;
	VAR_ME = 0xFF;
	VAR_NUM_ACTOR = 0xFF;
	VAR_CURRENT_LIGHTS = 0xFF;
	VAR_CURRENTDRIVE = 0xFF;	// How about merging this with VAR_CURRENTDISK?
	VAR_CURRENTDISK = 0xFF;
	VAR_TMR_1 = 0xFF;
	VAR_TMR_2 = 0xFF;
	VAR_TMR_3 = 0xFF;
	VAR_MUSIC_TIMER = 0xFF;
	VAR_ACTOR_RANGE_MIN = 0xFF;
	VAR_ACTOR_RANGE_MAX = 0xFF;
	VAR_CAMERA_MIN_X = 0xFF;
	VAR_CAMERA_MAX_X = 0xFF;
	VAR_TIMER_NEXT = 0xFF;
	VAR_VIRT_MOUSE_X = 0xFF;
	VAR_VIRT_MOUSE_Y = 0xFF;
	VAR_ROOM_RESOURCE = 0xFF;
	VAR_LAST_SOUND = 0xFF;
	VAR_CUTSCENEEXIT_KEY = 0xFF;
	VAR_OPTIONS_KEY = 0xFF;
	VAR_TALK_ACTOR = 0xFF;
	VAR_CAMERA_FAST_X = 0xFF;
	VAR_SCROLL_SCRIPT = 0xFF;
	VAR_ENTRY_SCRIPT = 0xFF;
	VAR_ENTRY_SCRIPT2 = 0xFF;
	VAR_EXIT_SCRIPT = 0xFF;
	VAR_EXIT_SCRIPT2 = 0xFF;
	VAR_VERB_SCRIPT = 0xFF;
	VAR_SENTENCE_SCRIPT = 0xFF;
	VAR_INVENTORY_SCRIPT = 0xFF;
	VAR_CUTSCENE_START_SCRIPT = 0xFF;
	VAR_CUTSCENE_END_SCRIPT = 0xFF;
	VAR_CHARINC = 0xFF;
	VAR_CHARCOUNT = 0xFF;
	VAR_WALKTO_OBJ = 0xFF;
	VAR_DEBUGMODE = 0xFF;
	VAR_HEAPSPACE = 0xFF;
	VAR_RESTART_KEY = 0xFF;
	VAR_PAUSE_KEY = 0xFF;
	VAR_MOUSE_X = 0xFF;
	VAR_MOUSE_Y = 0xFF;
	VAR_TIMER = 0xFF;
	VAR_TMR_4 = 0xFF;
	VAR_SOUNDCARD = 0xFF;
	VAR_VIDEOMODE = 0xFF;
	VAR_MAINMENU_KEY = 0xFF;
	VAR_FIXEDDISK = 0xFF;
	VAR_CURSORSTATE = 0xFF;
	VAR_USERPUT = 0xFF;
	VAR_SOUNDRESULT = 0xFF;
	VAR_TALKSTOP_KEY = 0xFF;
	VAR_FADE_DELAY = 0xFF;
	VAR_NOSUBTITLES = 0xFF;

	VAR_SOUNDPARAM = 0xFF;
	VAR_SOUNDPARAM2 = 0xFF;
	VAR_SOUNDPARAM3 = 0xFF;
	VAR_MOUSEPRESENT = 0xFF;
	VAR_MEMORY_PERFORMANCE = 0xFF;
	VAR_VIDEO_PERFORMANCE = 0xFF;
	VAR_ROOM_FLAG = 0xFF;
	VAR_GAME_LOADED = 0xFF;
	VAR_NEW_ROOM = 0xFF;
	VAR_VERSION_KEY = 0xFF;

	VAR_V5_TALK_STRING_Y = 0xFF;

	VAR_ROOM_WIDTH = 0xFF;
	VAR_ROOM_HEIGHT = 0xFF;
	VAR_SUBTITLES = 0xFF;
	VAR_V6_EMSSPACE = 0xFF;

	VAR_CAMERA_POS_Y = 0xFF;
	VAR_CAMERA_MIN_Y = 0xFF;
	VAR_CAMERA_MAX_Y = 0xFF;
	VAR_CAMERA_THRESHOLD_X = 0xFF;
	VAR_CAMERA_THRESHOLD_Y = 0xFF;
	VAR_CAMERA_SPEED_X = 0xFF;
	VAR_CAMERA_SPEED_Y = 0xFF;
	VAR_CAMERA_ACCEL_X = 0xFF;
	VAR_CAMERA_ACCEL_Y = 0xFF;
	VAR_CAMERA_DEST_X = 0xFF;
	VAR_CAMERA_DEST_Y = 0xFF;
	VAR_CAMERA_FOLLOWED_ACTOR = 0xFF;

	VAR_LEFTBTN_DOWN = 0xFF;
	VAR_RIGHTBTN_DOWN = 0xFF;
	VAR_LEFTBTN_HOLD = 0xFF;
	VAR_RIGHTBTN_HOLD = 0xFF;

	VAR_SAVELOAD_SCRIPT = 0xFF;
	VAR_SAVELOAD_SCRIPT2 = 0xFF;

	VAR_DEFAULT_TALK_DELAY = 0xFF;
	VAR_CHARSET_MASK = 0xFF;

	VAR_CUSTOMSCALETABLE = 0xFF;
	VAR_V6_SOUNDMODE = 0xFF;

	VAR_ACTIVE_VERB = 0xFF;
	VAR_ACTIVE_OBJECT1 = 0xFF;
	VAR_ACTIVE_OBJECT2 = 0xFF;
	VAR_VERB_ALLOWED = 0xFF;

	VAR_BLAST_ABOVE_TEXT = 0xFF;
	VAR_VOICE_MODE = 0xFF;
	VAR_MUSIC_BUNDLE_LOADED = 0xFF;
	VAR_VOICE_BUNDLE_LOADED = 0xFF;

	VAR_REDRAW_ALL_ACTORS = 0xFF;
	VAR_SKIP_RESET_TALK_ACTOR = 0xFF;

	VAR_SOUND_CHANNEL = 0xFF;
	VAR_TALK_CHANNEL = 0xFF;
	VAR_SOUNDCODE_TMR = 0xFF;
	VAR_RESERVED_SOUND_CHANNELS = 0xFF;

	VAR_MAIN_SCRIPT = 0xFF;

	VAR_NUM_SCRIPT_CYCLES = 0xFF;
	VAR_SCRIPT_CYCLE = 0xFF;

	VAR_NUM_GLOBAL_OBJS = 0xFF;
	VAR_KEY_STATE = 0xFF;
	VAR_MOUSE_STATE = 0xFF;

	// Use g_scumm from error() ONLY
	g_scumm = this;

	// Read settings from the detector & config manager
	_debugMode = (gDebugLevel >= 0);
	_dumpScripts = ConfMan.getBool("dump_scripts");
	_bootParam = ConfMan.getInt("boot_param");
	// Boot params often need debugging switched on to work
	if (_bootParam)
		_debugMode = true;

	// Allow the user to override the game name with a custom string.
	// This allows some game versions to work which use filenames
	// differing from the regular version(s) of that game.
	_baseName = ConfMan.hasKey("basename") ? ConfMan.get("basename") : gs.gameid;

	_copyProtection = ConfMan.getBool("copy_protection");
	_demoMode = ConfMan.getBool("demo_mode");
	if (ConfMan.hasKey("nosubtitles")) {
		printf("Configuration key 'nosubtitles' is deprecated. Use 'subtitles' instead\n");
		if (!ConfMan.hasKey("subtitles"))
			ConfMan.set("subtitles", !ConfMan.getBool("nosubtitles"));
	}

	// Make sure that at least subtitles are enabled
	if (ConfMan.getBool("speech_mute") && !ConfMan.getBool("subtitles"))
		ConfMan.set("subtitles", 1);

	// TODO Detect subtitle only versions of scumm6 games
	if (ConfMan.getBool("speech_mute"))
		_voiceMode = 2;
	else
		_voiceMode = ConfMan.getBool("subtitles");

	_confirmExit = ConfMan.getBool("confirm_exit");

	if (ConfMan.hasKey("render_mode")) {
		_renderMode = Common::parseRenderMode(ConfMan.get("render_mode").c_str());
	} else {
		_renderMode = Common::kRenderDefault;
	}

	// Do some render mode restirctions
	if (_game.version == 1)
		_renderMode = Common::kRenderDefault;

	switch (_renderMode) {
	case Common::kRenderHercA:
	case Common::kRenderHercG:
		if (_game.version > 2 && _game.id != GID_MONKEY_EGA)
			_renderMode = Common::kRenderDefault;
		break;

	case Common::kRenderCGA:
	case Common::kRenderEGA:
	case Common::kRenderAmiga:
		if (!(_game.features & GF_16COLOR))
			_renderMode = Common::kRenderDefault;
		break;

	default:
		break;
	}

	_hexdumpScripts = false;
	_showStack = false;

	if (_game.platform == Common::kPlatformFMTowns && _game.version == 3) {	// FM-TOWNS V3 games use 320x240
		_screenWidth = 320;
		_screenHeight = 240;
	} else if (_game.version == 8 || _game.heversion >= 71) {
		// COMI uses 640x480. Likewise starting from version 7.1, HE games use
		// 640x480, too.
		_screenWidth = 640;
		_screenHeight = 480;
	} else if (_game.platform == Common::kPlatformNES) {
		_screenWidth = 256;
		_screenHeight = 240;
	} else {
		_screenWidth = 320;
		_screenHeight = 200;
	}

	_compositeBuf = (byte *)malloc(_screenWidth * _screenHeight);

	_herculesBuf = 0;
	if (_renderMode == Common::kRenderHercA || _renderMode == Common::kRenderHercG) {
		_herculesBuf = (byte *)malloc(Common::kHercW * Common::kHercH);
	}
}

ScummEngine::~ScummEngine() {
	if (_musicEngine) {
		_musicEngine->terminate();
		delete _musicEngine;
	}

#ifndef DISABLE_SCUMM_7_8
	if (_smixer) {
		_smixer->stop();
		delete _smixer;
	}
#endif
	_mixer->stopAll();

	delete [] _actors;
	delete [] _sortedActors;

	delete _2byteFontPtr;
	delete _charset;
	delete _pauseDialog;
	delete _mainMenuDialog;
	delete _versionDialog;
	delete _fileHandle;

	delete _sound;

	delete _costumeLoader;
	delete _costumeRenderer;

	free(_shadowPalette);

	free(_palManipPalette);
	free(_palManipIntermediatePal);

	res.freeResources();

	free(_objectStateTable);
	free(_objectRoomTable);
	free(_objectOwnerTable);
	free(_inventory);
	free(_verbs);
	free(_objs);
	free(_roomVars);
	free(_scummVars);
	free(_bitVars);
	free(_newNames);
	free(_classData);
	free(_arraySlot);

	free(_compositeBuf);
	free(_herculesBuf);

	delete _debugger;
}

ScummEngine_v4::ScummEngine_v4(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine_v5(detector, syst, gs, md5sum, subst) {
	_resourceHeaderSize = 6;
}

ScummEngine_v3::ScummEngine_v3(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine_v4(detector, syst, gs, md5sum, subst) {
}

ScummEngine_v3old::ScummEngine_v3old(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine_v3(detector, syst, gs, md5sum, subst) {
	_resourceHeaderSize = 4;
}

ScummEngine_v2::ScummEngine_v2(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine_v3old(detector, syst, gs, md5sum, subst) {

	VAR_SENTENCE_VERB = 0xFF;
	VAR_SENTENCE_OBJECT1 = 0xFF;
	VAR_SENTENCE_OBJECT2 = 0xFF;
	VAR_SENTENCE_PREPOSITION = 0xFF;
	VAR_BACKUP_VERB = 0xFF;

	VAR_CLICK_AREA = 0xFF;
	VAR_CLICK_VERB = 0xFF;
	VAR_CLICK_OBJECT = 0xFF;
}

ScummEngine_c64::ScummEngine_c64(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine_v2(detector, syst, gs, md5sum, subst) {

	_currentMode = 0;
}

ScummEngine_v6::ScummEngine_v6(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine(detector, syst, gs, md5sum, subst) {
	_blastObjectQueuePos = 0;
	memset(_blastObjectQueue, 0, sizeof(_blastObjectQueue));
	_blastTextQueuePos = 0;
	memset(_blastTextQueue, 0, sizeof(_blastTextQueue));

	_smushFrameRate = 0;

	VAR_VIDEONAME = 0xFF;
	VAR_RANDOM_NR = 0xFF;
	VAR_STRING2DRAW = 0xFF;

	VAR_TIMEDATE_YEAR = 0xFF;
	VAR_TIMEDATE_MONTH = 0xFF;
	VAR_TIMEDATE_DAY = 0xFF;
	VAR_TIMEDATE_HOUR = 0xFF;
	VAR_TIMEDATE_MINUTE = 0xFF;
	VAR_TIMEDATE_SECOND = 0xFF;
}

#ifndef DISABLE_HE
ScummEngine_v70he::ScummEngine_v70he(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine_v60he(detector, syst, gs, md5sum, subst) {
	if (_game.platform == Common::kPlatformMacintosh && (_game.heversion >= 72 && _game.heversion <= 73))
		_resExtractor = new MacResExtractor(this);
	else
		_resExtractor = new Win32ResExtractor(this);

	_wiz = new Wiz(this);

	_heV7RoomOffsets = NULL;

	_heSndSoundId = 0;
	_heSndOffset = 0;
	_heSndChannel = 0;
	_heSndFlags = 0;
	_heSndSoundFreq = 0;

	_skipProcessActors = 0;

	VAR_NUM_SOUND_CHANNELS = 0xFF;
	VAR_WIZ_TCOLOR = 0xFF;
}

ScummEngine_v70he::~ScummEngine_v70he() {
	delete _resExtractor;
	delete _wiz;
	free(_heV7DiskOffsets);
	free(_heV7RoomIntOffsets);
	free(_heV7RoomOffsets);
	free(_storedFlObjects);
}

ScummEngine_v71he::ScummEngine_v71he(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine_v70he(detector, syst, gs, md5sum, subst) {
	_auxBlocksNum = 0;
	memset(_auxBlocks, 0, sizeof(_auxBlocks));
	_auxEntriesNum = 0;
	memset(_auxEntries, 0, sizeof(_auxEntries));
}

ScummEngine_v72he::ScummEngine_v72he(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine_v71he(detector, syst, gs, md5sum, subst) {
	VAR_NUM_ROOMS = 0xFF;
	VAR_NUM_SCRIPTS = 0xFF;
	VAR_NUM_SOUNDS = 0xFF;
	VAR_NUM_COSTUMES = 0xFF;
	VAR_NUM_IMAGES = 0xFF;
	VAR_NUM_CHARSETS = 0xFF;
	VAR_POLYGONS_ONLY = 0xFF;
}

ScummEngine_v80he::ScummEngine_v80he(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine_v72he(detector, syst, gs, md5sum, subst) {
	_heSndResId = 0;
	_curSndId = 0;
	_sndPtrOffs = 0;
	_sndTmrOffs = 0;

	VAR_PLATFORM = 0xFF;
	VAR_WINDOWS_VERSION = 0xFF;
	VAR_CURRENT_CHARSET = 0xFF;
	VAR_COLOR_DEPTH = 0xFF;
}

ScummEngine_v90he::ScummEngine_v90he(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine_v80he(detector, syst, gs, md5sum, subst) {
	_sprite = new Sprite(this);

	VAR_NUM_SPRITE_GROUPS = 0xFF;
	VAR_NUM_SPRITES = 0xFF;
	VAR_NUM_PALETTES = 0xFF;
	VAR_NUM_UNK = 0xFF;

	VAR_U32_VERSION = 0xFF;
	VAR_U32_ARRAY_UNK = 0xFF;
}

ScummEngine_v90he::~ScummEngine_v90he() {
	delete _sprite;
	if (_game.heversion >= 98) {
		delete _logicHE;
	}
	if (_game.heversion >= 99) {
		free(_hePalettes);
	}
}
#endif

#ifndef DISABLE_SCUMM_7_8
ScummEngine_v7::ScummEngine_v7(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine_v6(detector, syst, gs, md5sum, subst) {
	_verbCharset = 0;
	_existLanguageFile = false;
	_languageBuffer = NULL;
	_languageIndex = NULL;
	clearSubtitleQueue();
}

ScummEngine_v7::~ScummEngine_v7() {
	free(_languageBuffer);
	free(_languageIndex);
}

ScummEngine_v8::ScummEngine_v8(GameDetector *detector, OSystem *syst, const GameSettings &gs, uint8 md5sum[16], SubstResFileNames subst)
	: ScummEngine_v7(detector, syst, gs, md5sum, subst) {
	_objectIDMap = 0;
}

ScummEngine_v8::~ScummEngine_v8() {
	delete [] _objectIDMap;
}
#endif

#pragma mark -
#pragma mark --- Initialization ---
#pragma mark -

int ScummEngine::init(GameDetector &detector) {

	// Initialize backend
	_system->beginGFXTransaction();
		bool defaultTo1XScaler = false;
		if (_renderMode == Common::kRenderHercA || _renderMode == Common::kRenderHercG) {
			_system->initSize(Common::kHercW, Common::kHercH, 1);
			defaultTo1XScaler = true;
		} else {
			// FIXME: The way we now handle the force_1x_overlay setting implies
			// that if you start scummvm into the launcher with force_1x_overlay
			// set to true, it'll get reset to the default value (usually 'false'
			// except for Symbian) before launching a game.
			// This may or may not be the desired behavior...
			_system->initSize(_screenWidth, _screenHeight, (ConfMan.getBool("force_1x_overlay") ? 1 : 2));
			defaultTo1XScaler = (_screenWidth > 320);
		}
		initCommonGFX(detector, defaultTo1XScaler);
	_system->endGFXTransaction();

	// On some systems it's not safe to run CD audio games from the CD.
	if (_game.features & GF_AUDIOTRACKS)
		checkCD();

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0 && (_game.features & GF_AUDIOTRACKS))
		_system->openCD(cd_num);

	// Create the sound manager
	_sound = new Sound(this);

	// Setup the music engine
	setupMusic(_game.midi);

	// Load localization data, if present
	_language = Common::parseLanguage(ConfMan.get("language"));
	loadLanguageBundle();

	// Load CJK font, if present
	loadCJKFont();

	// Create the charset renderer
	if (_game.platform == Common::kPlatformNES)
		_charset = new CharsetRendererNES(this);
	else if (_game.version <= 2)
		_charset = new CharsetRendererV2(this, _language);
	else if (_game.version == 3)
		_charset = new CharsetRendererV3(this);
#ifndef DISABLE_SCUMM_7_8
	else if (_game.version == 8)
		_charset = new CharsetRendererNut(this);
#endif
	else
		_charset = new CharsetRendererClassic(this);

	// Create the costume renderer
	if (_game.features & GF_NEW_COSTUMES) {
		_costumeRenderer = new AkosRenderer(this);
		_costumeLoader = new AkosCostumeLoader(this);
	} else if (_game.platform == Common::kPlatformC64 && _game.id == GID_MANIAC) {
		_costumeRenderer = new C64CostumeRenderer(this);
		_costumeLoader = new C64CostumeLoader(this);
	} else if (_game.platform == Common::kPlatformNES) {
		_costumeRenderer = new NESCostumeRenderer(this);
		_costumeLoader = new NESCostumeLoader(this);
	} else {
		_costumeRenderer = new ClassicCostumeRenderer(this);
		_costumeLoader = new ClassicCostumeLoader(this);
	}

#ifndef DISABLE_SCUMM_7_8
	// Create FT INSANE object
	if (_game.id == GID_FT)
		_insane = new Insane((ScummEngine_v6 *)this);
	else
#endif
		_insane = 0;

	// Load game from specified slot, if any
	if (ConfMan.hasKey("save_slot")) {
		requestLoad(ConfMan.getInt("save_slot"));
	}

	allocResTypeData(rtBuffer, 0, 10, "buffer", 0);

	setupScummVars();

	setupOpcodes();

	if (_game.version == 8)
		_numActors = 80;
	else if (_game.version == 7)
		_numActors = 30;
	else if (_game.id == GID_SAMNMAX)
		_numActors = 30;
	else if (_game.id == GID_MANIAC)
		_numActors = 25;
	else if (_game.heversion >= 80)
		_numActors = 62;
	else if (_game.heversion >= 72)
		_numActors = 30;
	else
		_numActors = 13;

	if (_game.version >= 7)
		OF_OWNER_ROOM = 0xFF;
	else
		OF_OWNER_ROOM = 0x0F;

	// if (_game.id==GID_MONKEY2 && _bootParam == 0)
	//	_bootParam = 10001;

	if (!_copyProtection && _game.id == GID_INDY4 && _bootParam == 0) {
		_bootParam = -7873;
	}

	if (!_copyProtection && _game.id == GID_SAMNMAX && _bootParam == 0) {
		_bootParam = -1;
	}

	readIndexFile();

#ifdef PALMOS_68K
	if (_game.features & GF_NEW_COSTUMES)
		res._maxHeapThreshold = gVars->memory[kMemScummNewCostGames];
	else
		res._maxHeapThreshold = gVars->memory[kMemScummOldCostGames];
#else
	if (_game.features & GF_NEW_COSTUMES) {
		// Since the new costumes are very big, we increase the heap limit, to avoid having
		// to constantly reload stuff from the data files.
		res._maxHeapThreshold = 6 * 1024 * 1024;
	} else {
		res._maxHeapThreshold = 550000;
	}
#endif
	res._minHeapThreshold = 400000;

	scummInit();
	initScummVars();

	if (_imuse) {
		_imuse->setBase(res.address[rtSound]);
	}

	if (_game.version >= 5)
		_sound->setupSound();

#if (defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
	Graphics::initfonts();
#endif

	// Create debugger
	if (!_debugger)
		_debugger = new ScummDebugger(this);

	return 0;
}

void ScummEngine::scummInit() {
	int i;

	_tempMusic = 0;
	debug(9, "scummInit");

	if ((_game.id == GID_MANIAC) && (_game.version == 1) && !(_game.platform == Common::kPlatformNES)) {
		if (_game.platform == Common::kPlatformC64)
			initScreens(8, 144);
		else
			initScreens(16, 152);
	} else if (_game.version >= 7 || _game.heversion >= 71) {
		initScreens(0, _screenHeight);
	} else {
		initScreens(16, 144);
	}

	_palManipCounter = 0;

	for (i = 0; i < 256; i++)
		_roomPalette[i] = i;
	if (_game.version == 1) {
		// Use 17 color table for v1 games to allow
		// correct color for inventory and sentence
		// line
		// Original games used some kind of dynamic
		// color table remapping between rooms
		if (_game.platform == Common::kPlatformC64) {
			setupC64Palette();
		} else if (_game.platform == Common::kPlatformNES) {
			setupNESPalette();
		} else {
			setupV1Palette();
		}
	} else if (_game.features & GF_16COLOR) {
		for (i = 0; i < 16; i++)
			_shadowPalette[i] = i;

		switch (_renderMode) {
		case Common::kRenderEGA:
			setupEGAPalette();
			break;

		case Common::kRenderAmiga:
			setupAmigaPalette();
			break;

		case Common::kRenderCGA:
			setupCGAPalette();
			break;

		case Common::kRenderHercA:
		case Common::kRenderHercG:
			setupHercPalette();
			break;

		default:
			if ((_game.platform == Common::kPlatformAmiga) || (_game.platform == Common::kPlatformAtariST))
				setupAmigaPalette();
			else
				setupEGAPalette();
		}
	}

	if (_game.version >= 4 && _game.version <= 7)
		loadCharset(1);

	if (_game.features & GF_OLD_BUNDLE)
		loadCharset(0);

	setShake(0);
	setupCursor();

	// Allocate and Initialize actors
	Actor::initActorClass(this);
	_actors = new Actor[_numActors];
	_sortedActors = new Actor * [_numActors];
	for (i = 0; i < _numActors; i++) {
		_actors[i]._number = i;
		_actors[i].initActor(1);

		// this is from IDB
		if ((_game.version == 1) || (_game.id == GID_MANIAC && _demoMode))
			_actors[i].setActorCostume(i);
	}

	if (_game.id == GID_MANIAC && _game.version == 1) {
		setupV1ActorTalkColor();
	} else if (_game.id == GID_MANIAC && _game.version == 2 && _demoMode) {
		// HACK Some palette changes needed for demo script
		// in Maniac Mansion (Enhanced)
		_actors[3].setPalette(3, 1);
		_actors[9]._talkColor = 15;
		_actors[10]._talkColor = 7;
		_actors[11]._talkColor = 2;
		_actors[13]._talkColor = 5;
		_actors[23]._talkColor = 14;
	}

	vm.numNestedScripts = 0;
	vm.cutSceneStackPointer = 0;

	memset(vm.cutScenePtr, 0, sizeof(vm.cutScenePtr));
	memset(vm.cutSceneData, 0, sizeof(vm.cutSceneData));

	for (i = 0; i < _numVerbs; i++) {
		_verbs[i].verbid = 0;
		_verbs[i].curRect.right = _screenWidth - 1;
		_verbs[i].oldRect.left = -1;
		_verbs[i].type = 0;
		_verbs[i].color = 2;
		_verbs[i].hicolor = 0;
		_verbs[i].charset_nr = 1;
		_verbs[i].curmode = 0;
		_verbs[i].saveid = 0;
		_verbs[i].center = 0;
		_verbs[i].key = 0;
	}

	if (_game.features & GF_NEW_CAMERA) {
		VAR(VAR_CAMERA_THRESHOLD_X) = 100;
		VAR(VAR_CAMERA_THRESHOLD_Y) = 70;
		VAR(VAR_CAMERA_ACCEL_X) = 100;
		VAR(VAR_CAMERA_ACCEL_Y) = 100;
	} else {
		if (_game.platform == Common::kPlatformNES) {
			camera._leftTrigger = 6;	// 6
			camera._rightTrigger = 21;	// 25
		} else {
			camera._leftTrigger = 10;
			camera._rightTrigger = (_game.heversion >= 71) ? 70 : 30;
		}
		camera._mode = 0;
	}
	camera._follows = 0;

	virtscr[0].xstart = 0;

	if (_game.version <= 5) {
		// Setup light
		_flashlight.xStrips = 7;
		_flashlight.yStrips = 7;
		_flashlight.buffer = NULL;
	}

	_mouse.x = 104;
	_mouse.y = 56;

	_ENCD_offs = 0;
	_EXCD_offs = 0;

	_currentScript = 0xFF;
	_sentenceNum = 0;

	_currentRoom = 0;
	_numObjectsInRoom = 0;
	_actorToPrintStrFor = 0;

	_charsetBufPos = 0;
	_haveMsg = 0;
	_haveActorSpeechMsg = false;

	_varwatch = -1;
	_screenStartStrip = 0;

	_defaultTalkDelay = 3;
	_talkDelay = 0;
	_keepText = false;

	_currentCursor = 0;
	_cursor.state = 0;
	_userPut = 0;

	_newEffect = 129;
	_fullRedraw = true;

	clearDrawObjectQueue();

	if (_game.platform == Common::kPlatformNES)
		decodeNESBaseTiles();

	for (i = 0; i < 6; i++) {
		if (_game.version == 3) { // FIXME - what is this?
			_string[i]._default.xpos = 0;
			_string[i]._default.ypos = 0;
		} else {
			_string[i]._default.xpos = 2;
			_string[i]._default.ypos = 5;
		}
		_string[i]._default.right = _screenWidth - 1;
		_string[i]._default.height = 0;
		_string[i]._default.color = 0xF;
		_string[i]._default.center = 0;
		_string[i]._default.charset = 0;
	}

	// all keys are released
	for (i = 0; i < 512; i++)
		_keyDownMap[i] = false;

	_lastSaveTime = _system->getMillis();
}

void ScummEngine_c64::scummInit() {
	ScummEngine_v2::scummInit();
	initC64Verbs();
}

void ScummEngine_v2::scummInit() {
	ScummEngine::scummInit();

	if (_game.platform == Common::kPlatformNES) {
		initNESMouseOver();
		_switchRoomEffect2 = _switchRoomEffect = 6;
	} else {
		initV2MouseOver();
		// Seems in V2 there was only a single room effect (iris),
		// so we set that here.
		_switchRoomEffect2 = 1;
		_switchRoomEffect = 5;
	}

	_inventoryOffset = 0;
}

void ScummEngine_v4::scummInit() {
	ScummEngine::scummInit();

	// WORKAROUND for bug in boot script of Loom (CD)
	// The boot script sets the characters of string 21, 
	// before creating the string.resource.
	if (_game.id == GID_LOOM) {
		res.createResource(rtString, 21, 12);
	}
}

void ScummEngine_v6::scummInit() {
	ScummEngine::scummInit();
	setDefaultCursor();
}

void ScummEngine_v60he::scummInit() {
	ScummEngine_v6::scummInit();

	// HACK cursor hotspot is wrong
	// Original games used
	// setCursorHotspot(8, 7);
	if (_game.id == GID_FUNPACK)
		setCursorHotspot(16, 16);
}

#ifndef DISABLE_HE
void ScummEngine_v72he::scummInit() {
	ScummEngine_v60he::scummInit();

	_stringLength = 1;
	memset(_stringBuffer, 0, sizeof(_stringBuffer));
}

void ScummEngine_v90he::scummInit() {
	ScummEngine_v72he::scummInit();

	_heObject = 0;
	_heObjectNum = 0;
	_hePaletteNum = 0;

	_sprite->resetTables(0);
	memset(&_wizParams, 0, sizeof(_wizParams));

	if (_game.heversion >= 98) {
		switch (_game.id) {
		case GID_PUTTRACE:
			_logicHE = new LogicHErace(this);
			break;

		case GID_FUNSHOP:
			_logicHE = new LogicHEfunshop(this);
			break;

		case GID_FOOTBALL:
			_logicHE = new LogicHEfootball(this);
			break;

		case GID_SOCCER:
			_logicHE = new LogicHEsoccer(this);
			break;

		default:
			_logicHE = new LogicHE(this);
			break;
		}
	}
}

void ScummEngine_v99he::scummInit() {
	ScummEngine_v90he::scummInit();

	_hePalettes = (uint8 *)malloc((_numPalettes + 1) * 1024);
	memset(_hePalettes, 0, (_numPalettes + 1) * 1024);

	byte basename[256];
	char buf1[128];

	strcpy((char *)basename, _baseName.c_str());
	if (_substResFileName.almostGameID != 0) {
		generateSubstResFileName((char *)basename, buf1, sizeof(buf1));
		strcpy((char *)basename, buf1);
	}

	// Array 129 is set to base name
	int len = resStrLen(basename);
	ArrayHeader *ah = defineArray(129, kStringArray, 0, 0, 0, len);
	memcpy(ah->data, basename, len);
}
#endif

void ScummEngine::setupMusic(int midi) {
	int midiDriver = MidiDriver::detectMusicDriver(midi);
	_native_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));
	
	switch (midiDriver) {
	case MD_NULL:
		_musicType = MDT_NONE;
		break;
	case MD_PCSPK:
	case MD_PCJR:
		_musicType = MDT_PCSPK;
		break;
	case MD_TOWNS:
		_musicType = MDT_TOWNS;
		break;
	case MD_ADLIB:
		_musicType = MDT_ADLIB;
		break;
	default:
		_musicType = MDT_MIDI;
		break;
	}
	
	// FIXME: MD_TOWNS should not be _midi_native in the first place!! iMuse code needs to be restructured.
	if ((_game.id == GID_TENTACLE) || (_game.id == GID_SAMNMAX) || (midiDriver == MD_TOWNS))
		_enable_gs = false;
	else
		_enable_gs = ConfMan.getBool("enable_gs");

	/* Bind the mixer to the system => mixer will be invoked
	 * automatically when samples need to be generated */
	if (!_mixer->isReady()) {
		warning("Sound mixer initialization failed\n");
		if (_musicType == MDT_ADLIB || _musicType == MDT_PCSPK)	{
			midiDriver = MD_NULL;
			_musicType = MDT_NONE;
			warning("MIDI driver depends on sound mixer, switching to null MIDI driver\n");
		}
	}

	// Init iMuse
	if (_game.features & GF_DIGI_IMUSE) {
#ifndef DISABLE_SCUMM_7_8
		_musicEngine = _imuseDigital = new IMuseDigital(this, 10);
		_smixer = new SmushMixer(_mixer);
#endif
	} else if (_game.platform == Common::kPlatformC64) {
		// TODO
		_musicEngine = NULL;
	} else if (_game.platform == Common::kPlatformNES) {
		_musicEngine = new Player_NES(this);
	} else if ((_game.platform == Common::kPlatformAmiga) && (_game.version == 2)) {
		_musicEngine = new Player_V2A(this);
	} else if ((_game.platform == Common::kPlatformAmiga) && (_game.version == 3)) {
		_musicEngine = new Player_V3A(this);
	} else if ((_game.platform == Common::kPlatformAmiga) && (_game.version <= 4)) {
		_musicEngine = NULL;
	} else if (_game.id == GID_MANIAC && (_game.version == 1)) {
		_musicEngine = new Player_V1(this, midiDriver != MD_PCSPK);
	} else if (_game.version <= 2) {
		_musicEngine = new Player_V2(this, midiDriver != MD_PCSPK);
	} else if ((_musicType == MDT_PCSPK) && ((_game.version > 2) && (_game.version <= 4))) {
		_musicEngine = new Player_V2(this, midiDriver != MD_PCSPK);
	} else if (_game.version >= 3 && _game.heversion <= 61 && _game.platform != Common::kPlatform3DO) {
		MidiDriver *nativeMidiDriver = 0;
		MidiDriver *adlibMidiDriver = 0;

		if (_musicType != MDT_ADLIB)
			nativeMidiDriver = MidiDriver::createMidi(midiDriver);
		if (nativeMidiDriver != NULL && _native_mt32)
			nativeMidiDriver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
		bool multi_midi = ConfMan.getBool("multi_midi") && _musicType != MDT_NONE && (midi & MDT_ADLIB);
		if (_musicType == MDT_ADLIB || multi_midi) {
			adlibMidiDriver = MidiDriver_ADLIB_create(_mixer);
			adlibMidiDriver->property(MidiDriver::PROP_OLD_ADLIB, (_game.features & GF_SMALL_HEADER) ? 1 : 0);
		}

		_musicEngine = _imuse = IMuse::create(_system, nativeMidiDriver, adlibMidiDriver);
		if (_imuse) {
			_imuse->addSysexHandler
				(/*IMUSE_SYSEX_ID*/ 0x7D,
				 (_game.id == GID_SAMNMAX) ? sysexHandler_SamNMax : sysexHandler_Scumm);
			_imuse->property(IMuse::PROP_GAME_ID, _game.id);
			if (ConfMan.hasKey("tempo"))
				_imuse->property(IMuse::PROP_TEMPO_BASE, ConfMan.getInt("tempo"));
			_imuse->property(IMuse::PROP_NATIVE_MT32, _native_mt32);
			_imuse->property(IMuse::PROP_GS, _enable_gs);
			if (_game.heversion >= 60 || midi == MDT_TOWNS) {
				_imuse->property(IMuse::PROP_LIMIT_PLAYERS, 1);
				_imuse->property(IMuse::PROP_RECYCLE_PLAYERS, 1);
			}
			if (midi == MDT_TOWNS)
				_imuse->property(IMuse::PROP_DIRECT_PASSTHROUGH, 1);
		}
	}

	setupVolumes();
}

void ScummEngine::setupVolumes() {

	// Sync the engine with the config manager
	int soundVolumeMusic = ConfMan.getInt("music_volume");
	int soundVolumeSfx = ConfMan.getInt("sfx_volume");
	int soundVolumeSpeech = ConfMan.getInt("speech_volume");

	if (_musicEngine) {
		_musicEngine->setMusicVolume(soundVolumeMusic);
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundVolumeSfx);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, soundVolumeSpeech);
}



#pragma mark -
#pragma mark --- Main loop ---
#pragma mark -

int ScummEngine::go() {
	_engineStartTime = _system->getMillis() / 1000;

	// If requested, load a save game instead of running the boot script
	if (_saveLoadFlag != 2 || !loadState(_saveLoadSlot, _saveTemporaryState)) {
		int args[16];
		memset(args, 0, sizeof(args));
		args[0] = _bootParam;

		_saveLoadFlag = 0;
#ifndef DISABLE_HE
		if (_game.heversion >= 98) {
			((ScummEngine_v90he *)this)->_logicHE->initOnce();
			((ScummEngine_v90he *)this)->_logicHE->beforeBootScript();
		}
#endif
		if (_game.id == GID_MANIAC && _demoMode)
			runScript(9, 0, 0, args);
		else
			runScript(1, 0, 0, args);
	} else {
		_saveLoadFlag = 0;
	}

	int delta = 0;
	int diff = _system->getMillis();

	while (!_quit) {

		updatePalette();
		_system->updateScreen();

		diff -= _system->getMillis();
		waitForTimer(delta * 15 + diff);
		diff = _system->getMillis();
		delta = scummLoop(delta);

		if (delta < 1)	// Ensure we don't get into a loop
			delta = 1;  // by not decreasing sleepers.

		if (_quit) {
			// TODO: Maybe perform an autosave on exit?
		}
	}

	return 0;
}

void ScummEngine::waitForTimer(int msec_delay) {
	uint32 start_time;

	if (_fastMode & 2)
		msec_delay = 0;
	else if (_fastMode & 1)
		msec_delay = 10;

	start_time = _system->getMillis();

	while (!_quit) {
		_sound->updateCD(); // Loop CD Audio if needed
		parseEvents();
		if (_system->getMillis() >= start_time + msec_delay)
			break;
		_system->delayMillis(10);
	}
}

int ScummEngine::scummLoop(int delta) {
	if (_debugger->isAttached())
		_debugger->onFrame();

	// Randomize the PRNG by calling it at regular intervals. This ensures
	// that it will be in a different state each time you run the program.
	_rnd.getRandomNumber(2);

#ifndef DISABLE_HE
	if (_game.heversion >= 98) {
		((ScummEngine_v90he *)this)->_logicHE->startOfFrame();
	}
#endif
	if (_game.version > 2) {
		VAR(VAR_TMR_1) += delta;
		VAR(VAR_TMR_2) += delta;
		VAR(VAR_TMR_3) += delta;
		if (_game.id == GID_ZAK || _game.id == GID_INDY3) {
			// All versions of Indy3 set three extra timers
			// FM-TOWNS version of Zak sets three extra timers
			VAR(39) += delta;
			VAR(40) += delta;
			VAR(41) += delta;
		}
	}
	if (VAR_TMR_4 != 0xFF)
		VAR(VAR_TMR_4) += delta;

	if (delta > 15)
		delta = 15;

	decreaseScriptDelay(delta);

	_talkDelay -= delta;
	if (_talkDelay < 0)
		_talkDelay = 0;

	// Record the current ego actor before any scripts (including input scripts)
	// get a chance to run.
	int oldEgo = 0;
	if (VAR_EGO != 0xFF)
		oldEgo = VAR(VAR_EGO);

	// In V1-V3 games, CHARSET_1 is called much earlier than in newer games.
	// See also bug #770042 for a case were this makes a difference.
	if (_game.version <= 3)
		CHARSET_1();

	processKbd(false);

	if (_game.features & GF_NEW_CAMERA) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
		VAR(VAR_CAMERA_POS_Y) = camera._cur.y;
	} else if (_game.version <= 2) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x / 8;
	} else {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
	}
	if (_game.version <= 7)
		VAR(VAR_HAVE_MSG) = _haveMsg;

	if (_game.platform == Common::kPlatformC64 && _game.id == GID_MANIAC) {
		// TODO
	} else if (_game.version <= 2) {
		VAR(VAR_VIRT_MOUSE_X) = _virtualMouse.x / 8;
		VAR(VAR_VIRT_MOUSE_Y) = _virtualMouse.y / 2;

		// Adjust mouse coordinates as narrow rooms in NES are centered
		if (_game.platform == Common::kPlatformNES && _NESStartStrip > 0) {
			VAR(VAR_VIRT_MOUSE_X) -= 2;
			if (VAR(VAR_VIRT_MOUSE_X) < 0)
				VAR(VAR_VIRT_MOUSE_X) = 0;
		}
	} else {
		VAR(VAR_VIRT_MOUSE_X) = _virtualMouse.x;
		VAR(VAR_VIRT_MOUSE_Y) = _virtualMouse.y;
		VAR(VAR_MOUSE_X) = _mouse.x;
		VAR(VAR_MOUSE_Y) = _mouse.y;
		if (VAR_DEBUGMODE != 0xFF) {
			// This is NOT for the Mac version of Indy3/Loom
			VAR(VAR_DEBUGMODE) = _debugMode;
		}
	}

	if (_game.features & GF_AUDIOTRACKS) {
		// Covered automatically by the Sound class
	} else if (VAR_MUSIC_TIMER != 0xFF) {
		if (_musicEngine) {
			// The music engine generates the timer data for us.
			VAR(VAR_MUSIC_TIMER) = _musicEngine->getMusicTimer();
		} else {
			// Used for Money Island 1 (Amiga)
			// TODO: The music delay (given in milliseconds) might have to be tuned a little
			// to get it correct for all games. Without the ability to watch/listen to the
			// original games, I can't do that myself.
			const int MUSIC_DELAY = 350;
			_tempMusic += delta * 15;	// Convert delta to milliseconds
			if (_tempMusic >= MUSIC_DELAY) {
				_tempMusic -= MUSIC_DELAY;
				VAR(VAR_MUSIC_TIMER) += 1;
			}
		}
	}

	// Trigger autosave if necessary.
	if (!_saveLoadFlag && shouldPerformAutoSave(_lastSaveTime)) {
		_saveLoadSlot = 0;
		sprintf(_saveLoadName, "Autosave %d", _saveLoadSlot);
		_saveLoadFlag = 1;
		_saveTemporaryState = false;
	}

	if (VAR_GAME_LOADED != 0xFF)
		VAR(VAR_GAME_LOADED) = 0;
	if (_saveLoadFlag) {
load_game:
		bool success;
		const char *errMsg = 0;
		char filename[256];

		if (_saveLoadFlag == 1) {
			success = saveState(_saveLoadSlot, _saveTemporaryState);
			if (!success)
				errMsg = "Failed to save game state to file:\n\n%s";

			// Ender: Disabled for small_header games, as can overwrite game
			//  variables (eg, Zak256 cashcard values). Temp disabled for V8
			// because of odd timing issue with scripts and the variable reset
			if (success && _saveTemporaryState && !(_game.features & GF_SMALL_HEADER) && _game.version < 8)
				VAR(VAR_GAME_LOADED) = 201;
		} else {
			success = loadState(_saveLoadSlot, _saveTemporaryState);
			if (!success)
				errMsg = "Failed to load game state from file:\n\n%s";

			// Ender: Disabled for small_header games, as can overwrite game
			//  variables (eg, Zak256 cashcard values).
			if (success && _saveTemporaryState && !(_game.features & GF_SMALL_HEADER))
				VAR(VAR_GAME_LOADED) = 203;
		}

		makeSavegameName(filename, _saveLoadSlot, _saveTemporaryState);
		if (!success) {
			displayMessage(0, errMsg, filename);
		} else if (_saveLoadFlag == 1 && _saveLoadSlot != 0 && !_saveTemporaryState) {
			// Display "Save successful" message, except for auto saves
			char buf[256];
			snprintf(buf, sizeof(buf), "Successfully saved game state in file:\n\n%s", filename);

			GUI::TimedMessageDialog dialog(buf, 1500);
			runDialog(dialog);
		}
		if (success && _saveLoadFlag != 1)
			clearClickedStatus();

		_saveLoadFlag = 0;
		_lastSaveTime = _system->getMillis();
	}

	if (_completeScreenRedraw) {
		_charset->clearCharsetMask();
		_charset->_hasMask = false;

		// HACK as in game save stuff isn't supported currently
		if (_game.id == GID_LOOM) {
			int args[16];
			uint value;
			memset(args, 0, sizeof(args));
			args[0] = 2;

			if (_game.platform == Common::kPlatformMacintosh)
				value = 105;
			else if (_game.version == 4)	// 256 color CD version
				value = 150;
			else
 				value = 100;
			byte restoreScript = (_game.platform == Common::kPlatformFMTowns) ? 17 : 18;
			// if verbs should be shown restore them
			if (VAR(value) == 2)
				runScript(restoreScript, 0, 0, args);
		} else if (_game.version > 3) {
			for (int i = 0; i < _numVerbs; i++)
				drawVerb(i, 0);
		} else {
			redrawVerbs();
		}

		handleMouseOver(false);

		_completeScreenRedraw = false;
		_fullRedraw = true;
	}

	if (_game.heversion >= 80) {
		_sound->processSoundCode();
	}
	runAllScripts();
	checkExecVerbs();
	checkAndRunSentenceScript();

	if (_quit)
		return 0;

	// HACK: If a load was requested, immediately perform it. This avoids
	// drawing the current room right after the load is request but before
	// it is performed. That was annoying esp. if you loaded while a SMUSH
	// cutscene was playing.
	if (_saveLoadFlag && _saveLoadFlag != 1) {
		goto load_game;
	}

	if (_currentRoom == 0) {
		if (_game.version > 3)
			CHARSET_1();
		drawDirtyScreenParts();
	} else {
		walkActors();
		moveCamera();
		updateObjectStates();
		if (_game.version > 3)
			CHARSET_1();

		if (camera._cur.x != camera._last.x || _bgNeedsRedraw || _fullRedraw
				|| ((_game.features & GF_NEW_CAMERA) && camera._cur.y != camera._last.y)) {
			redrawBGAreas();
		}

		processDrawQue();

		if (_game.heversion >= 99)
			_fullRedraw = false;

		// Full Throttle always redraws verbs and draws verbs before actors
		if (_game.version >= 7)
			redrawVerbs();

#ifndef DISABLE_HE
		if (_game.heversion >= 90) {
			((ScummEngine_v90he *)this)->_sprite->resetBackground();
			((ScummEngine_v90he *)this)->_sprite->sortActiveSprites();
		}
#endif

		setActorRedrawFlags();
		resetActorBgs();

		if (!(getCurrentLights() & LIGHTMODE_room_lights_on) &&
		      getCurrentLights() & LIGHTMODE_flashlight_on) {
			drawFlashlight();
			setActorRedrawFlags();
		}

		processActors();

		_fullRedraw = false;

		if (_game.version >= 4 && _game.heversion <= 61)
			cyclePalette();
		palManipulate();
		if (_doEffect) {
			_doEffect = false;
			fadeIn(_newEffect);
			clearClickedStatus();
		}

		if (VAR_MAIN_SCRIPT != 0xFF && VAR(VAR_MAIN_SCRIPT) != 0) {
			runScript(VAR(VAR_MAIN_SCRIPT), 0, 0, 0);
		}

		// Handle mouse over effects (for verbs).
		handleMouseOver(oldEgo != VAR(VAR_EGO));

		// Render everything to the screen.
		drawDirtyScreenParts();

		if (_game.version <= 5)
			playActorSounds();
	}

	_sound->processSound();

#ifndef DISABLE_SCUMM_7_8
	if (_imuseDigital) {
		_imuseDigital->flushTracks();
		if ( ((_game.id == GID_DIG) && (!(_game.features & GF_DEMO))) || (_game.id == GID_CMI) )
			_imuseDigital->refreshScripts();
	}
	if (_smixer) {
		_smixer->flush();
	}
#endif

	camera._last = camera._cur;

	if (!(++res._expireCounter)) {
		res.increaseResourceCounter();
	}

	animateCursor();

	/* show or hide mouse */
	_system->showMouse(_cursor.state > 0);

#ifndef DISABLE_HE
	if (_game.heversion >= 90) {
		((ScummEngine_v90he *)this)->_sprite->updateImages();
	}
	if (_game.heversion >= 98) {
		((ScummEngine_v90he *)this)->_logicHE->endOfFrame();
	}
#endif

	if (VAR_TIMER != 0xFF)
		VAR(VAR_TIMER) = 0;
	return (VAR_TIMER_NEXT != 0xFF) ? VAR(VAR_TIMER_NEXT) : 4;

}

#pragma mark -
#pragma mark --- SCUMM ---
#pragma mark -

int ScummEngine::getHETimer(int timer) {
	checkRange(15, 1, timer, "getHETimer: Timer out of range(%d)");
	int time = _system->getMillis() - _heTimers[timer];
	return time;
}

void ScummEngine::setHETimer(int timer) {
	checkRange(15, 1, timer, "setHETimer: Timer out of range(%d)");
	_heTimers[timer] = _system->getMillis();
}

void ScummEngine::pauseGame() {
	pauseDialog();
}

void ScummEngine::shutDown() {
	_quit = true;
}

void ScummEngine::restart() {
// TODO: Check this function - we should probably be reinitting a lot more stuff, and I suspect
//	 this leaks memory like a sieve

// Fingolfing seez: An alternate way to implement restarting would be to create
// a save state right after startup ... to this end we could introduce a SaveFile
// subclass which is implemented using a memory buffer (i.e. no actual file is
// created). Then to restart we just have to load that pseudo save state.


	int i;

	// Reset some stuff
	_currentRoom = 0;
	_currentScript = 0xFF;
	killAllScriptsExceptCurrent();
	setShake(0);
	_sound->stopAllSounds();

	// Clear the script variables
	for (i = 0; i < _numVariables; i++)
		_scummVars[i] = 0;

	// Empty inventory
	for (i = 0; i < _numGlobalObjects; i++)
		clearOwnerOf(i);

	// Reallocate arrays
	allocateArrays();

	// Reread index (reset objectstate etc)
	readIndexFile();

	// Reinit scumm variables
	scummInit();
	initScummVars();

	if (_imuse) {
		_imuse->setBase(res.address[rtSound]);
	}

	// Reinit sound engine
	if (_game.version >= 5)
		_sound->setupSound();

	// Re-run bootscript
	int args[16];
	memset(args, 0, sizeof(args));
	args[0] = _bootParam;
	if (_game.id == GID_MANIAC && _demoMode)
		runScript(9, 0, 0, args);
	else
		runScript(1, 0, 0, args);
}

void ScummEngine::startManiac() {
	debug(0, "stub startManiac()");
	displayMessage(0, "Usually, Maniac Mansion would start now. But ScummVM doesn't do that yet. To play it, go to 'Add Game' in the ScummVM start menu and select the 'Maniac' directory inside the Tentacle game directory.");
}

#pragma mark -
#pragma mark --- GUI ---
#pragma mark -

int ScummEngine::runDialog(Dialog &dialog) {
	_dialogStartTime = _system->getMillis() / 1000;

	// Pause sound & video
	bool old_soundsPaused = _sound->_soundsPaused;
	_sound->pauseSounds(true);
	bool oldSmushPaused = _smushPaused;
	_smushPaused = true;

	// Open & run the dialog
	int result = dialog.runModal();

	// Restore old cursor
	updateCursor();

	// Resume sound & video
	_sound->pauseSounds(old_soundsPaused);
	_smushPaused = oldSmushPaused;

	_engineStartTime += (_system->getMillis() / 1000) - _dialogStartTime;
	_dialogStartTime = 0;

	// Return the result
	return result;
}

void ScummEngine::pauseDialog() {
	if (!_pauseDialog)
		_pauseDialog = new PauseDialog(this, 4);
	runDialog(*_pauseDialog);
}

void ScummEngine::versionDialog() {
	if (!_versionDialog)
		_versionDialog = new PauseDialog(this, 1);
	runDialog(*_versionDialog);
}

void ScummEngine::mainMenuDialog() {
	if (!_mainMenuDialog)
		_mainMenuDialog = new MainMenuDialog(this);
	runDialog(*_mainMenuDialog);
}

void ScummEngine::confirmExitDialog() {
	ConfirmDialog d(this, 6);

	if (runDialog(d)) {
		_quit = true;
	}
}

void ScummEngine::confirmRestartDialog() {
	ConfirmDialog d(this, 5);

	if (runDialog(d)) {
		restart();
	}
}

char ScummEngine::displayMessage(const char *altButton, const char *message, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, message);
	vsnprintf(buf, STRINGBUFLEN, message, va);
	va_end(va);

	GUI::MessageDialog dialog(buf, "OK", altButton);
	return runDialog(dialog);
}


#pragma mark -
#pragma mark --- Miscellaneous ---
#pragma mark -


void ScummEngine::errorString(const char *buf1, char *buf2) {
	if (_currentScript != 0xFF) {
		ScriptSlot *ss = &vm.slot[_currentScript];
		sprintf(buf2, "(%d:%d:0x%lX): %s", _roomResource,
			ss->number, (long)(_scriptPointer - _scriptOrgPointer), buf1);
	} else {
		strcpy(buf2, buf1);
	}

#ifdef _WIN32_WCE
	if (isSmartphone())
		return;
#endif

	// Unless an error -originated- within the debugger, spawn the debugger. Otherwise
	// exit out normally.
	if (_debugger && !_debugger->isAttached()) {
		printf("%s\n", buf2);	// (Print it again in case debugger segfaults)
		_debugger->attach(buf2);
		_debugger->onFrame();
	}
}

void ScummEngine::generateSubstResFileName(const char *filename, char *buf, int bufsize) {
	applySubstResFileName(_substResFileName, filename, buf, bufsize);
}


} // End of namespace Scumm
