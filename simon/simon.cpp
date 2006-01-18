/* ScummVM - Scumm Interpreter
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
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/system.h"

#include "gui/about.h"
#include "gui/message.h"

#include "simon/simon.h"
#include "simon/intern.h"
#include "simon/vga.h"
#include "simon/debugger.h"
#include "simon/simon-md5.h"

#include "sound/mididrv.h"
#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

#ifdef __PALM_OS__
#include "globals.h"
#endif

using Common::File;

struct SimonGameSettings {
	const char *name;
	const char *description;
	uint32 features;
	const char *detectname;
	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
};

static const SimonGameSettings simon_settings[] = {
	// Simon the Sorcerer 1 & 2 (not SCUMM games)
	{"simon1acorn", "Simon the Sorcerer 1 (Acorn)", GAME_SIMON1ACORN, "DATA"},
	{"simon1dos", "Simon the Sorcerer 1 (DOS)", GAME_SIMON1DOS, "GAMEPC"},
	{"simon1amiga", "Simon the Sorcerer 1 (Amiga)", GAME_SIMON1AMIGA, "gameamiga"},
	{"simon2dos", "Simon the Sorcerer 2 (DOS)", GAME_SIMON2DOS, "GAME32"},
	{"simon1talkie", "Simon the Sorcerer 1 Talkie", GAME_SIMON1TALKIE, "GAMEPC"},
	{"simon1win", "Simon the Sorcerer 1 Talkie (Windows)", GAME_SIMON1TALKIE, 0},
	{"simon2talkie", "Simon the Sorcerer 2 Talkie", GAME_SIMON2TALKIE, "GSPTR30"},
	{"simon2win", "Simon the Sorcerer 2 Talkie (Windows)", GAME_SIMON2WIN, 0},
	{"simon2mac", "Simon the Sorcerer 2 Talkie (Amiga or Mac)", GAME_SIMON2WIN, 0},
	{"simon1cd32", "Simon the Sorcerer 1 Talkie (Amiga CD32)", GAME_SIMON1CD32, "gameamiga"},
	{"simon1demo", "Simon the Sorcerer 1 (DOS Demo)", GAME_SIMON1DEMO, "GDEMO"},
//	{"feeble", "The Feeble Files", GAME_FEEBLEFILES, "GAME22"},

	{NULL, NULL, 0, NULL}
};

static int compareMD5Table(const void *a, const void *b) {
	const char *key = (const char *)a;
	const MD5Table *elem = (const MD5Table *)b;
	return strcmp(key, elem->md5);
}

GameList Engine_SIMON_gameList() {
	const SimonGameSettings *g = simon_settings;
	GameList games;
	while (g->name) {
		games.push_back(g->toGameSettings());
		g++;
	}
	return games;
}

DetectedGameList Engine_SIMON_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const SimonGameSettings *g;
	char detectName[128];
	char detectName2[128];

	typedef Common::Map<Common::String, bool> StringSet;
	StringSet fileSet;

	for (g = simon_settings; g->name; ++g) {
		if (g->detectname == NULL)
			continue;

		strcpy(detectName, g->detectname);
		strcpy(detectName2, g->detectname);
		strcat(detectName2, ".");

		// Iterate over all files in the given directory
		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (!file->isDirectory()) {
				const char *name = file->displayName().c_str();

				if ((!scumm_stricmp(detectName, name))  || (!scumm_stricmp(detectName2, name))) {
					// Match found, add to list of candidates, then abort inner loop.
					detectedGames.push_back(g->toGameSettings());
					fileSet.addKey(file->path());
					break;
				}
			}
		}
	}

	// Now, we check the MD5 sums of the 'candidate' files. If we have an exact match,
	// only return that.
	bool exactMatch = false;
	for (StringSet::const_iterator iter = fileSet.begin(); iter != fileSet.end(); ++iter) {
		uint8 md5sum[16];
		const char *name = iter->_key.c_str();
		if (Common::md5_file(name, md5sum)) {
			char md5str[32+1];
			for (int j = 0; j < 16; j++) {
				sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
			}

			const MD5Table *elem;
			elem = (const MD5Table *)bsearch(md5str, md5table, ARRAYSIZE(md5table)-1, sizeof(MD5Table), compareMD5Table);
			if (elem) {
				if (!exactMatch)
					detectedGames.clear();	// Clear all the non-exact candidates
				// Find the GameSettings for that target
				for (g = simon_settings; g->name; ++g) {
					if (0 == scumm_stricmp(g->name, elem->target))
						break;
				}
				assert(g->name);
				// Insert the 'enhanced' game data into the candidate list
				detectedGames.push_back(DetectedGame(g->toGameSettings(), elem->language, elem->platform));
				exactMatch = true;
			}
		}
	}

	return detectedGames;
}

Engine *Engine_SIMON_create(GameDetector *detector, OSystem *syst) {
	return new Simon::SimonEngine(detector, syst);
}

REGISTER_PLUGIN(SIMON, "Simon the Sorcerer")

namespace Simon {

#ifdef __PALM_OS__
#define PTR(a) a
static const GameSpecificSettings *simon1_settings;
static const GameSpecificSettings *simon1acorn_settings;
static const GameSpecificSettings *simon1amiga_settings;
static const GameSpecificSettings *simon1demo_settings;
static const GameSpecificSettings *simon2win_settings;
static const GameSpecificSettings *simon2dos_settings;
#else
#define PTR(a) &a
static const GameSpecificSettings simon1_settings = {
	"SIMON.GME",                            // gme_filename
	"SIMON.WAV",                            // wav_filename
	"SIMON.VOC",                            // voc_filename
	"SIMON.MP3",                            // mp3_filename
	"SIMON.OGG",                            // vorbis_filename
	"SIMON.FLA",                            // flac_filename
	"EFFECTS.VOC",                          // voc_effects_filename
	"EFFECTS.MP3",                          // mp3_effects_filename
	"EFFECTS.OGG",                          // vorbis_effects_filename
	"EFFECTS.FLA",                          // flac_effects_filename
	"GAMEPC",                               // gamepc_filename
};

static const GameSpecificSettings simon1acorn_settings = {
	"DATA",                                 // gme_filename
	"",                                     // wav_filename
	"SIMON",                                // voc_filename
	"SIMON.MP3",                            // mp3_filename
	"SIMON.OGG",                            // vorbis_filename
	"SIMON.FLA",                            // flac_filename
	"EFFECTS",                              // voc_effects_filename
	"EFFECTS.MP3",                          // mp3_effects_filename
	"EFFECTS.OGG",                          // vorbis_effects_filename
	"EFFECTS.FLA",                          // flac_effects_filename
	"GAMEBASE",                             // gamepc_filename
};

static const GameSpecificSettings simon1amiga_settings = {
	"",                                     // gme_filename
	"",                                     // wav_filename
	"",                                     // voc_filename
	"SIMON.MP3",                            // mp3_filename
	"SIMON.OGG",                            // vorbis_filename
	"SIMON.FLA",                            // flac_filename
	"",                                     // voc_effects_filename
	"",                                     // mp3_effects_filename
	"",                                     // vorbis_effects_filename
	"",                                     // flac_effects_filename
	"gameamiga",                            // gamepc_filename
};

static const GameSpecificSettings simon1demo_settings = {
	"",                                     // gme_filename
	"",                                     // wav_filename
	"",                                     // voc_filename
	"",                                     // mp3_filename
	"",                                     // vorbis_filename
	"",					// flac_filename
	"",                                     // voc_effects_filename
	"",                                     // mp3_effects_filename
	"",                                     // vorbis_effects_filename
	"",                                     // flac_effects_filename
	"GDEMO",                                // gamepc_filename
};

static const GameSpecificSettings simon2win_settings = {
	"SIMON2.GME",                           // gme_filename
	"SIMON2.WAV",                           // wav_filename
	"SIMON2.VOC",                           // voc_filename
	"SIMON2.MP3",                           // mp3_filename
	"SIMON2.OGG",                           // vorbis_filename
	"SIMON2.FLA",                           // flac_filename
	"",                                     // voc_effects_filename
	"",                                     // mp3_effects_filename
	"",                                     // vorbis_effects_filename
	"",                                     // flac_effects_filename
	"GSPTR30",                              // gamepc_filename
};

static const GameSpecificSettings simon2dos_settings = {
	"SIMON2.GME",                           // gme_filename
	"",                                     // wav_filename
	"",                                     // voc_filename
	"",                                     // mp3_filename
	"",                                     // vorbis_filename
	"",					// flac_filename
	"",                                     // voc_effects_filename
	"",                                     // mp3_effects_filename
	"",                                     // vorbis_effects_filename
	"",                                     // flac_effects_filename
	"GAME32",                               // gamepc_filename
};

static const GameSpecificSettings feeblefiles_settings = {
	"",		                        // gme_filename
	"VOICES.WAV",                           // wav_filename
	"VOICES.VOC",                           // voc_filename
	"VOICES.MP3",                           // mp3_filename
	"VOICES.OGG",                           // vorbis_filename
	"VOICES.FLA",                           // flac_filename
	"",                                     // voc_effects_filename
	"",                                     // mp3_effects_filename
	"",                                     // vorbis_effects_filename
	"",                                     // flac_effects_filename
	"GAME22",                               // gamepc_filename
};
#endif

SimonEngine::SimonEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst), midi(syst) {
	int j =0;
	_vcPtr = 0;
	_vc_get_out_of_code = 0;
	_gameOffsetsPtr = 0;

	_debugger = 0;

	const SimonGameSettings *g = simon_settings;
	while (g->name) {
		if (!scumm_stricmp(detector->_game.name, g->name))
			break;
		g++;
	}
	if (!g->name)
		error("Invalid game '%s'\n", detector->_game.name);

	SimonGameSettings game = *g;

	switch (Common::parsePlatform(ConfMan.get("platform"))) {
	case Common::kPlatformAmiga:
	case Common::kPlatformMacintosh:
		if (game.features & GF_SIMON2)
			game.features |= GF_WIN;
		break;
	case Common::kPlatformWindows:
		game.features |= GF_WIN;
		break;
	default:
		break;
	}

	_game = game.features;

	// Convert older targets
	if (g->detectname == NULL) {
		if (!strcmp("simon1win", g->name)) {
			ConfMan.set("gameid", "simon1talkie");
			ConfMan.set("platform", "Windows");
		} else if (!strcmp("simon2win", g->name) || !strcmp("simon2mac", g->name)) {
			ConfMan.set("gameid", "simon2talkie");
			ConfMan.set("platform", "Windows");
		}
		ConfMan.flushToDisk();
	} else {
		char buf[100];
		uint8 md5sum[16];
		File f;

		sprintf(buf, g->detectname);
		f.open(buf);
		if (f.isOpen() == false)
			strcat(buf, ".");

		if (Common::md5_file(buf, md5sum)) {
			char md5str[32+1];
			for (j = 0; j < 16; j++) {
				sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
			}

			printf("%s  %s\n", md5str, buf);
			const MD5Table *elem;
			elem = (const MD5Table *)bsearch(md5str, md5table, ARRAYSIZE(md5table)-1, sizeof(MD5Table), compareMD5Table);
			if (elem)
				printf("Match found in database: target %s, language %s, platform %s\n",
					elem->target, Common::getLanguageDescription(elem->language), Common::getPlatformDescription(elem->platform));
			else
				printf("Unknown MD5! Please report the details (language, platform, etc.) of this game to the ScummVM team\n");
		}
	}

	VGA_DELAY_BASE = 1;
	if (_game == GAME_FEEBLEFILES) {
		NUM_VIDEO_OP_CODES = 85;
#ifndef __PALM_OS__
		VGA_MEM_SIZE = 7500000;
#else
		VGA_MEM_SIZE = gVars->memory[kMemSimon2Games];
#endif
		TABLES_MEM_SIZE = 200000;
	} else if (_game & GF_SIMON2) {
		TABLE_INDEX_BASE = 1580 / 4;
		TEXT_INDEX_BASE = 1500 / 4;
		NUM_VIDEO_OP_CODES = 75;
#ifndef __PALM_OS__
		VGA_MEM_SIZE = 2000000;
#else
		VGA_MEM_SIZE = gVars->memory[kMemSimon2Games];
#endif
		TABLES_MEM_SIZE = 100000;
		// Check whether to use MT-32 MIDI tracks in Simon the Sorcerer 2
		if ((_game & GF_SIMON2) && (ConfMan.getBool("native_mt32") || (_midiDriver == MD_MT32)))
			MUSIC_INDEX_BASE = (1128 + 612) / 4;
		else
			MUSIC_INDEX_BASE = 1128 / 4;
		SOUND_INDEX_BASE = 1660 / 4;
	} else {
		TABLE_INDEX_BASE = 1576 / 4;
		TEXT_INDEX_BASE = 1460 / 4;
		NUM_VIDEO_OP_CODES = 64;
#ifndef __PALM_OS__
		VGA_MEM_SIZE = 1000000;
#else
		VGA_MEM_SIZE = gVars->memory[kMemSimon1Games];
#endif
		TABLES_MEM_SIZE = 50000;
		MUSIC_INDEX_BASE = 1316 / 4;
		SOUND_INDEX_BASE = 0;
	}

	_language = Common::parseLanguage(ConfMan.get("language"));
	if (_game == GAME_FEEBLEFILES) {
		gss = PTR(feeblefiles_settings);
	} else if (_game & GF_SIMON2) {
		if (_game & GF_TALKIE) {
			gss = PTR(simon2win_settings);

			// Add default file directories
			File::addDefaultDirectory(_gameDataPath + "voices/");
			File::addDefaultDirectory(_gameDataPath + "VOICES/");
		} else {
			gss = PTR(simon2dos_settings);
		}
	} else if (_game & GF_SIMON1) {
		if (_game & GF_ACORN) {
			gss = PTR(simon1acorn_settings);

			// Add default file directories
			File::addDefaultDirectory(_gameDataPath + "execute/");
			File::addDefaultDirectory(_gameDataPath + "EXECUTE/");
		} else if (_game & GF_AMIGA) {
			gss = PTR(simon1amiga_settings);
		} else if (_game & GF_DEMO) {
			gss = PTR(simon1demo_settings);
		} else {
			gss = PTR(simon1_settings);
		}
	}

	if ((_game & GF_SIMON1) && (_game & GF_TALKIE)) {
		// Add default file directories
		switch (_language) {
		case 20:
			File::addDefaultDirectory(_gameDataPath + "hebrew/");
			File::addDefaultDirectory(_gameDataPath + "HEBREW/");
			break;
		case  5:
			File::addDefaultDirectory(_gameDataPath + "spanish/");
			File::addDefaultDirectory(_gameDataPath + "SPANISH/");
			break;
		case  3:
			File::addDefaultDirectory(_gameDataPath + "italian/");
			File::addDefaultDirectory(_gameDataPath + "ITALIAN/");
			break;
		case  2:
			File::addDefaultDirectory(_gameDataPath + "french/");
			File::addDefaultDirectory(_gameDataPath + "FRENCH/");
			break;
		}
	}

	_keyPressed = 0;

	_gameFile = 0;

	_strippedTxtMem = 0;
	_textSize = 0;
	_stringTabNum = 0;
	_stringTabPos = 0;
	_stringtab_numalloc = 0;
	_stringTabPtr = 0;

	_itemArrayPtr = 0;
	_itemArraySize = 0;
	_itemArrayInited = 0;

	_itemHeapPtr = 0;
	_itemHeapCurPos = 0;
	_itemHeapSize = 0;

	_iconFilePtr = 0;

	_tblList = 0;

	_codePtr = 0;

	_localStringtable = 0;
	_stringIdLocalMin = 1;
	_stringIdLocalMax = 0;

	_tablesHeapPtr = 0;
	_tablesHeapPtrOrg = 0;
	_tablesheapPtrNew = 0;
	_tablesHeapSize = 0;
	_tablesHeapCurPos = 0;
	_tablesHeapCurPosOrg = 0;
	_tablesHeapCurPosNew = 0;

	_subroutineList = 0;
	_subroutineListOrg = 0;
	_subroutine = 0;

	_dxSurfacePitch = 0;

	_recursionDepth = 0;

	_lastVgaTick = 0;

	_marks = 0;

	_scriptVar2 = 0;
	_runScriptReturn1 = 0;
	_skipVgaWait = 0;
	_noParentNotify = 0;
	_vgaRes328Loaded = 0;
	_hitarea_unk_3 = 0;
	_mortalFlag = 0;
	_videoVar8 = 0;
	_usePaletteDelay = 0;
	_syncFlag2 = 0;
	_inCallBack = 0;
	_cepeFlag = 0;
	_copyPartialMode = 0;
	_speed = 1;
	_fastMode = 0;
	_dxUse3Or4ForLock = 0;

	_debugMode = 0;
	_pause = 0;
	_startMainScript = 0;
	_continousMainScript = 0;
	_startVgaScript = 0;
	_continousVgaScript = 0;
	_drawImagesDebug = 0;
	_dumpImages = 0;
	_speech = true;
	_subtitles = true;
	_fade = true;
	_mouseCursor = 0;
	_vgaVar9 = 0;
	_scriptUnk1 = 0;
	_vgaVar6 = 0;
	_scrollX = 0;
	_scrollY = 0;
	_scrollXMax = 0;
	_scrollYMax = 0;
	_scrollCount = 0;
	_scrollFlag = 0;
	_scrollHeight = 0;
	_scrollWidth = 0;
	_scrollImage = 0;
	_vgaVar8 = 0;

	_scriptCondA = 0;
	_scriptCondB = 0;
	_scriptCondC = 0;

	_fcsUnk1 = 0;
	_fcsPtr1 = 0;

	_subjectItem = 0;
	_objectItem = 0;
	_item1 = 0;

	_hitAreaObjectItem = 0;
	_lastHitArea = 0;
	_lastHitArea2Ptr = 0;
	_lastHitArea3 = 0;
	_leftButtonDown = 0;
	_hitAreaSubjectItem = 0;
	_hitAreaPtr5 = 0;
	_hitAreaPtr7 = 0;
	_needHitAreaRecalc = 0;
	_verbHitArea = 0;
	_hitAreaUnk4 = 0;
	_lockCounter = 0;

	_windowNum = 0;

	_printCharUnk1 = 0;
	_printCharUnk2 = 0;
	_numLettersToPrint = 0;

	_lastTime = 0;

	_firstTimeStruct = 0;
	_pendingDeleteTimeEvent = 0;

	_base_time = 0;

	_mouseX = 0;
	_mouseY = 0;
	_mouseXOld = 0;
	_mouseYOld = 0;

	_dummyItem1 = new Item();
	_dummyItem2 = new Item();
	_dummyItem3 = new Item();

	_lockWord = 0;
	_scrollUpHitArea = 0;
	_scrollDownHitArea = 0;

	_videoVar7 = 0xFFFF;
	_paletteColorCount = 0;

	_videoVar4 = 0;
	_videoVar5 = 0;
	_videoVar3 = 0;
	_unkPalFlag = 0;
	_exitCutscene = 0;
	_skipSpeech = 0;
	_videoVar9 = 0;

	_soundFileId = 0;
	_lastMusicPlayed = -1;
	_nextMusicToPlay = -1;

	_showPreposition = 0;
	_showMessageFlag = 0;

	_videoNumPalColors = 0;

	_vgaSpriteChanged = 0;

	_vgaBufFreeStart = 0;
	_vgaBufEnd = 0;
	_vgaBufStart = 0;
	_vgaFileBufOrg = 0;
	_vgaFileBufOrg2 = 0;

	_curVgaFile1 = 0;
	_curVgaFile2 = 0;

	_timer1 = 0;
	_timer5 = 0;
	_timer4 = 0;

	_frameRate = 1;

	_vgaCurFile2 = 0;
	_vgaWaitFor = 0;
	_vgaCurFileId = 0;
	_vgaCurSpriteId = 0;

	_nextVgaTimerToProcess = 0;

	memset(_vcItemArray, 0, sizeof(_vcItemArray));
	memset(_itemArray6, 0, sizeof(_itemArray6));

	memset(_stringIdArray2, 0, sizeof(_stringIdArray2));
	memset(_stringIdArray3, 0, sizeof(_stringIdArray3));
	memset(_speechIdArray4, 0, sizeof(_speechIdArray4));

	memset(_bitArray, 0, sizeof(_bitArray));
	memset(_variableArray, 0, sizeof(_variableArray));

	memset(_fcsPtrArray3, 0, sizeof(_fcsPtrArray3));

	memset(_fcsData1, 0, sizeof(_fcsData1));
	memset(_fcsData2, 0, sizeof(_fcsData2));

	_freeStringSlot = 0;

	memset(_stringReturnBuffer, 0, sizeof(_stringReturnBuffer));

	memset(_pathFindArray, 0, sizeof(_pathFindArray));

	memset(_paletteBackup, 0, sizeof(_paletteBackup));
	memset(_palette, 0, sizeof(_palette));

	memset(_videoBuf1, 0, sizeof(_videoBuf1));

	_fcs_list = new FillOrCopyStruct[16];

	memset(_lettersToPrintBuf, 0, sizeof(_lettersToPrintBuf));

	_numScreenUpdates = 0;
	_vgaTickCounter = 0;

	_sound = 0;

	_effectsPaused = false;
	_ambientPaused = false;
	_musicPaused = false;

	_dumpFile = 0;

	_saveLoadType = 0;
	_saveLoadSlot = 0;
	memset(_saveLoadName, 0, sizeof(_saveLoadName));

	_saveLoadRowCurPos = 0;
	_numSaveGameRows = 0;
	_saveDialogFlag = false;
	_saveOrLoad = false;
	_saveLoadFlag = false;

	_sdlMouseX = 0;
	_sdlMouseY = 0;

	_sdl_buf_3 = 0;
	_sdl_buf = 0;
	_sdl_buf_attached = 0;

	_vc10BasePtrOld = 0;
	memcpy (_hebrew_char_widths,
		"\x5\x5\x4\x6\x5\x3\x4\x5\x6\x3\x5\x5\x4\x6\x5\x3\x4\x6\x5\x6\x6\x6\x5\x5\x5\x6\x5\x6\x6\x6\x6\x6", 32);

	if (_game == GAME_FEEBLEFILES) {
		_screenWidth = 640;
		_screenHeight = 480;
	} else {
		_screenWidth = 320;
		_screenHeight = 200;
	}
}

int SimonEngine::init(GameDetector &detector) {
	// Setup mixer
	if (!_mixer->isReady())
		warning("Sound initialization failed. "
						"Features of the game that depend on sound synchronization will most likely break");
	set_volume(ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_system->beginGFXTransaction();
		initCommonGFX(detector);
		_system->initSize(_screenWidth, _screenHeight);
		if (_game == GAME_FEEBLEFILES)
			_system->setGraphicsMode("1x");
	_system->endGFXTransaction();

	// Setup midi driver
	MidiDriver *driver = 0;
	_midiDriver = MD_NULL;
	if (_game == GAME_SIMON1AMIGA || _game == GAME_SIMON1CD32)
		driver = MidiDriver::createMidi(MD_NULL);	// Create fake MIDI driver for Simon1Amiga and Simon2CD32 for now
	else {
		_midiDriver = MidiDriver::detectMusicDriver(MDT_ADLIB | MDT_NATIVE);
		driver = MidiDriver::createMidi(_midiDriver);
	}
	if (!driver)
		driver = MidiDriver_ADLIB_create(_mixer);
	else if (ConfMan.getBool("native_mt32") || (_midiDriver == MD_MT32))
		driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	midi.mapMT32toGM (!(_game & GF_SIMON2) && !(ConfMan.getBool("native_mt32") || (_midiDriver == MD_MT32)));

	midi.set_driver(driver);
	int ret = midi.open();
	if (ret)
		warning ("MIDI Player init failed: \"%s\"", midi.getErrorName (ret));
	midi.set_volume(ConfMan.getInt("music_volume"));

	_debugMode = (gDebugLevel >= 0);

	if (ConfMan.hasKey("music_mute") && ConfMan.getBool("music_mute") == 1)
		midi.pause(_musicPaused ^= 1);

	if ((_game & GF_SIMON2) && ConfMan.hasKey("speech_mute") && ConfMan.getBool("speech_mute") == 1)
		_speech = 0;

	if ((!(_game & GF_SIMON2) && _language > 1) || ((_game & GF_SIMON2) && _language == 20)) {
		if (ConfMan.hasKey("subtitles") && ConfMan.getBool("subtitles") == 0)
			_subtitles = 0;
	} else
		_subtitles = ConfMan.getBool("subtitles");

	// Make sure either speech or subtitles is enabled
	if ((_game & GF_TALKIE) && !_speech && !_subtitles)
		_subtitles = 1;

	if (ConfMan.hasKey("fade") && ConfMan.getBool("fade") == 0)
		_fade = 0;

	if (ConfMan.hasKey("slow_down") && ConfMan.getInt("slow_down") >= 1)
		_speed = ConfMan.getInt("slow_down");

	// FIXME Use auto dirty rects cleanup code to reduce CPU usage
	g_system->setFeatureState(OSystem::kFeatureAutoComputeDirtyRects, true);

	return 0;
}

SimonEngine::~SimonEngine() {
	delete _gameFile;

	midi.close();

	free(_stringTabPtr);
	free(_itemArrayPtr);
	free(_itemHeapPtr - _itemHeapCurPos);
	free(_tablesHeapPtr - _tablesHeapCurPos);
	free(_tblList);
	free(_iconFilePtr);
	free(_gameOffsetsPtr);

	delete _dummyItem1;
	delete _dummyItem2;
	delete _dummyItem3;

	delete [] _fcs_list;

	delete _sound;
	delete _debugger;
}

void SimonEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);

#ifdef _WIN32_WCE
	if (isSmartphone())
		return;
#endif

	// Unless an error -originated- within the debugger, spawn the
	// debugger. Otherwise exit out normally.
	if (_debugger && !_debugger->isAttached()) {
		// (Print it again in case debugger segfaults)
		printf("%s\n", buf2);
		_debugger->attach(buf2);
		_debugger->onFrame();
	}
}

void SimonEngine::palette_fadeout(uint32 *pal_values, uint num) {
	byte *p = (byte *)pal_values;

	do {
		if (p[0] >= 8)
			p[0] -= 8;
		else
			p[0] = 0;
		if (p[1] >= 8)
			p[1] -= 8;
		else
			p[1] = 0;
		if (p[2] >= 8)
			p[2] -= 8;
		else
			p[2] = 0;
		p += sizeof(uint32);
	} while (--num);
}

byte *SimonEngine::allocateItem(uint size) {
	byte *org = _itemHeapPtr;
	size = (size + 3) & ~3;

	_itemHeapPtr += size;
	_itemHeapCurPos += size;

	if (_itemHeapCurPos > _itemHeapSize)
		error("Itemheap overflow");

	return org;
}

void SimonEngine::alignTableMem() {
	if ((unsigned long)_tablesHeapPtr & 3) {
		_tablesHeapPtr += 2;
		_tablesHeapCurPos += 2;
	}
}

byte *SimonEngine::allocateTable(uint size) {
	byte *org = _tablesHeapPtr;

	size = (size + 1) & ~1;

	_tablesHeapPtr += size;
	_tablesHeapCurPos += size;

	if (_tablesHeapCurPos > _tablesHeapSize)
		error("Tablesheap overflow");

	return org;
}

int SimonEngine::allocGamePcVars(File *in) {
	uint item_array_size, item_array_inited, stringtable_num;
	uint32 version;
	uint i;

	item_array_size = in->readUint32BE();
	version = in->readUint32BE();
	item_array_inited = in->readUint32BE();
	stringtable_num = in->readUint32BE();

	item_array_inited += 2;				// first two items are predefined
	item_array_size += 2;

	if (version != 0x80)
		error("Not a runtime database");

	_itemArrayPtr = (Item **)calloc(item_array_size, sizeof(Item *));
	if (_itemArrayPtr == NULL)
		error("Out of memory for Item array");

	_itemArraySize = item_array_size;
	_itemArrayInited = item_array_inited;

	for (i = 1; i < item_array_inited; i++) {
		_itemArrayPtr[i] = (Item *)allocateItem(sizeof(Item));
	}

	// The rest is cleared automatically by calloc
	allocateStringTable(stringtable_num + 10);
	_stringTabNum = stringtable_num;

	return item_array_inited;
}

void SimonEngine::loginPlayerHelper(Item *item, int a, int b) {
	Child9 *child;

	child = (Child9 *) findChildOfType(item, 9);
	if (child == NULL) {
		child = (Child9 *) allocateChildBlock(item, 9, sizeof(Child9));
	}

	if (a >= 0 && a <= 3)
		child->array[a] = b;
}

void SimonEngine::loginPlayer() {
	Child *child;

	_item1 = _itemArrayPtr[1];
	_item1->adjective = -1;
	_item1->noun = 10000;

	child = (Child *)allocateChildBlock(_item1, 3, sizeof(Child));
	if (child == NULL)
		error("player create failure");

	loginPlayerHelper(_item1, 0, 0);
}

void SimonEngine::allocateStringTable(int num) {
	_stringTabPtr = (byte **)calloc(num, sizeof(byte *));
	_stringTabPos = 0;
	_stringtab_numalloc = num;
}

void SimonEngine::setupStringTable(byte *mem, int num) {
	int i = 0;
	for (;;) {
		_stringTabPtr[i++] = mem;
		if (--num == 0)
			break;
		for (; *mem; mem++);
		mem++;
	}

	_stringTabPos = i;
}

void SimonEngine::setupLocalStringTable(byte *mem, int num) {
	int i = 0;
	for (;;) {
		_localStringtable[i++] = mem;
		if (--num == 0)
			break;
		for (; *mem; mem++);
		mem++;
	}
}

void SimonEngine::readSubroutineLine(File *in, SubroutineLine *sl, Subroutine *sub) {
	byte line_buffer[1024], *q = line_buffer;
	int size;

	if (sub->id == 0) {
		sl->cond_a = in->readUint16BE();
		sl->cond_b = in->readUint16BE();
		sl->cond_c = in->readUint16BE();
	}

	while ((*q = in->readByte()) != 0xFF) {
		if (*q == 87) {
			in->readUint16BE();
		} else {
			q = readSingleOpcode(in, q);
		}
	}

	size = q - line_buffer + 1;

	memcpy(allocateTable(size), line_buffer, size);
}

SubroutineLine *SimonEngine::createSubroutineLine(Subroutine *sub, int where) {
	SubroutineLine *sl, *cur_sl = NULL, *last_sl = NULL;

	if (sub->id == 0)
		sl = (SubroutineLine *)allocateTable(SUBROUTINE_LINE_BIG_SIZE);
	else
		sl = (SubroutineLine *)allocateTable(SUBROUTINE_LINE_SMALL_SIZE);

	// where is what offset to insert the line at, locate the proper beginning line
	if (sub->first != 0) {
		cur_sl = (SubroutineLine *)((byte *)sub + sub->first);
		while (where) {
			last_sl = cur_sl;
			cur_sl = (SubroutineLine *)((byte *)sub + cur_sl->next);
			if ((byte *)cur_sl == (byte *)sub)
				break;
			where--;
		}
	}

	if (last_sl != NULL) {
		// Insert the subroutine line in the middle of the link
		last_sl->next = (byte *)sl - (byte *)sub;
		sl->next = (byte *)cur_sl - (byte *)sub;
	} else {
		// Insert the subroutine line at the head of the link
		sl->next = sub->first;
		sub->first = (byte *)sl - (byte *)sub;
	}

	return sl;
}

void SimonEngine::readSubroutine(File *in, Subroutine *sub) {
	while (in->readUint16BE() == 0) {
		readSubroutineLine(in, createSubroutineLine(sub, 0xFFFF), sub);
	}
}

Subroutine *SimonEngine::createSubroutine(uint id) {
	Subroutine *sub;

	alignTableMem();

	sub = (Subroutine *)allocateTable(sizeof(Subroutine));
	sub->id = id;
	sub->first = 0;
	sub->next = _subroutineList;
	_subroutineList = sub;
	return sub;
}

void SimonEngine::readSubroutineBlock(File *in) {
	while (in->readUint16BE() == 0) {
		readSubroutine(in, createSubroutine(in->readUint16BE()));
	}
}

Child *SimonEngine::findChildOfType(Item *i, uint type) {
	Child *child = i->children;
	for (; child; child = child->next)
		if (child->type == type)
			return child;
	return NULL;
}

bool SimonEngine::hasChildOfType1(Item *item) {
	return findChildOfType(item, 1) != NULL;
}

bool SimonEngine::hasChildOfType2(Item *item) {
	return findChildOfType(item, 2) != NULL;
}

uint SimonEngine::getOffsetOfChild2Param(Child2 *child, uint prop) {
	uint m = 1;
	uint offset = 0;
	while (m != prop) {
		if (child->avail_props & m)
			offset++;
		m <<= 1;
	}
	return offset;
}

Child *SimonEngine::allocateChildBlock(Item *i, uint type, uint size) {
	Child *child = (Child *)allocateItem(size);
	child->next = i->children;
	i->children = child;
	child->type = type;
	return child;
}

void SimonEngine::allocItemHeap() {
	_itemHeapSize = 10000;
	_itemHeapCurPos = 0;
	_itemHeapPtr = (byte *)calloc(10000, 1);
}

void SimonEngine::allocTablesHeap() {
	_tablesHeapSize = TABLES_MEM_SIZE;
	_tablesHeapCurPos = 0;
	_tablesHeapPtr = (byte *)calloc(TABLES_MEM_SIZE, 1);
}

void SimonEngine::setItemState(Item *item, int value) {
	item->state = value;
}

int SimonEngine::getNextWord() {
	int16 a = (int16)READ_BE_UINT16(_codePtr);
	_codePtr += 2;
	return a;
}

uint SimonEngine::getNextStringID() {
	return (uint16)getNextWord();
}

uint SimonEngine::getVarOrByte() {
	uint a = *_codePtr++;
	if (a != 255)
		return a;
	return readVariable(*_codePtr++);
}

uint SimonEngine::getVarOrWord() {
	uint a = READ_BE_UINT16(_codePtr);
	_codePtr += 2;
	if (a >= 30000 && a < 30512)
		return readVariable(a - 30000);
	return a;
}

Item *SimonEngine::getNextItemPtr() {
	int a = getNextWord();
	switch (a) {
	case -1:
		return _subjectItem;
	case -3:
		return _objectItem;
	case -5:
		return getItem1Ptr();
	case -7:
		return getItemPtrB();
	case -9:
		return derefItem(getItem1Ptr()->parent);
	default:
		return derefItem(a);
	}
}

Item *SimonEngine::getNextItemPtrStrange() {
	int a = getNextWord();
	switch (a) {
	case -1:
		return _subjectItem;
	case -3:
		return _objectItem;
	case -5:
		return _dummyItem2;
	case -7:
		return NULL;
	case -9:
		return _dummyItem3;
	default:
		return derefItem(a);
	}
}

uint SimonEngine::getNextItemID() {
	int a = getNextWord();
	switch (a) {
	case -1:
		return itemPtrToID(_subjectItem);
	case -3:
		return itemPtrToID(_objectItem);
	case -5:
		return getItem1ID();
	case -7:
		return 0;
	case -9:
		return getItem1Ptr()->parent;
	default:
		return a;
	}
}

Item *SimonEngine::getItem1Ptr() {
	if (_item1)
		return _item1;
	return _dummyItem1;
}

Item *SimonEngine::getItemPtrB() {
	error("getItemPtrB: is this code ever used?");
	return _dummyItem1;
}

uint SimonEngine::getNextVarContents() {
	return (uint16)readVariable(getVarOrByte());
}

uint SimonEngine::readVariable(uint variable) {
	if (variable >= 255)
		error("Variable %d out of range in read", variable);
	return _variableArray[variable];
}

void SimonEngine::writeNextVarContents(uint16 contents) {
	writeVariable(getVarOrByte(), contents);
}

void SimonEngine::writeVariable(uint variable, uint16 contents) {
	if (variable >= 256)
		error("Variable %d out of range in write", variable);
	_variableArray[variable] = contents;
}

void SimonEngine::setItemParent(Item *item, Item *parent) {
	Item *old_parent = derefItem(item->parent);

	if (item == parent)
		error("Trying to set item as its own parent");

	// unlink it if it has a parent
	if (old_parent)
		unlinkItem(item);
	itemChildrenChanged(old_parent);
	linkItem(item, parent);
	itemChildrenChanged(parent);
}

void SimonEngine::itemChildrenChanged(Item *item) {
	int i;
	FillOrCopyStruct *fcs;

	if (_noParentNotify)
		return;

	lock();

	for (i = 0; i != 8; i++) {
		fcs = _fcsPtrArray3[i];
		if (fcs && fcs->fcs_data && fcs->fcs_data->item_ptr == item) {
			if (_fcsData1[i]) {
				_fcsData2[i] = true;
			} else {
				_fcsData2[i] = false;
				fcs_unk_proc_1(i, item, fcs->fcs_data->unk1, fcs->fcs_data->unk2);
			}
		}
	}

	unlock();
}

void SimonEngine::unlinkItem(Item *item) {
	Item *first, *parent, *next;

	// can't unlink item without parent
	if (item->parent == 0)
		return;

	// get parent and first child of parent
	parent = derefItem(item->parent);
	first = derefItem(parent->child);

	// the node to remove is first in the parent's children?
	if (first == item) {
		parent->child = item->sibling;
		item->parent = 0;
		item->sibling = 0;
		return;
	}

	for (;;) {
		if (!first)
			error("unlinkItem: parent empty");
		if (first->sibling == 0)
			error("unlinkItem: parent does not contain child");

		next = derefItem(first->sibling);
		if (next == item) {
			first->sibling = next->sibling;
			item->parent = 0;
			item->sibling = 0;
			return;
		}
		first = next;
	}
}

void SimonEngine::linkItem(Item *item, Item *parent) {
	uint id;
	// Don't allow that an item that is already linked is relinked
	if (item->parent)
		return;

	id = itemPtrToID(parent);
	item->parent = id;

	if (parent != 0) {
		item->sibling = parent->child;
		parent->child = itemPtrToID(item);
	} else {
		item->sibling = 0;
	}
}

const byte *SimonEngine::getStringPtrByID(uint string_id) {
	const byte *string_ptr;
	byte *dst;

	_freeStringSlot ^= 1;

	if (string_id < 0x8000) {
		string_ptr = _stringTabPtr[string_id];
	} else {
		string_ptr = getLocalStringByID(string_id);
	}

	dst = _stringReturnBuffer[_freeStringSlot];
	strcpy((char *)dst, (const char *)string_ptr);
	return dst;
}

const byte *SimonEngine::getLocalStringByID(uint string_id) {
	if (string_id < _stringIdLocalMin || string_id >= _stringIdLocalMax) {
		loadTextIntoMem(string_id);
	}
	return _localStringtable[string_id - _stringIdLocalMin];
}

void SimonEngine::loadTextIntoMem(uint string_id) {
	byte *p;
	char filename[30];
	int i;
	uint base_min = 0x8000, base_max, size;

	_tablesHeapPtr = _tablesheapPtrNew;
	_tablesHeapCurPos = _tablesHeapCurPosNew;

	p = _strippedTxtMem;

	// get filename
	while (*p) {
		for (i = 0; *p; p++, i++)
			filename[i] = *p;
		filename[i] = 0;
		p++;

		base_max = (p[0] << 8) | p[1];
		p += 2;

		if (string_id < base_max) {
			_stringIdLocalMin = base_min;
			_stringIdLocalMax = base_max;

			_localStringtable = (byte **)_tablesHeapPtr;

			size = (base_max - base_min + 1) * sizeof(byte *);
			_tablesHeapPtr += size;
			_tablesHeapCurPos += size;

			size = loadTextFile(filename, _tablesHeapPtr);

			setupLocalStringTable(_tablesHeapPtr, base_max - base_min + 1);

			_tablesHeapPtr += size;
			_tablesHeapCurPos += size;

			if (_tablesHeapCurPos > _tablesHeapSize) {
				error("loadTextIntoMem: Out of table memory");
			}
			return;
		}

		base_min = base_max;
	}

	error("loadTextIntoMem: didn't find %d", string_id);
}

void SimonEngine::loadTablesIntoMem(uint subr_id) {
	byte *p;
	int i;
	uint min_num, max_num;
	char filename[30];
	File *in;

	p = _tblList;
	if (p == NULL)
		return;

	while (*p) {
		for (i = 0; *p; p++, i++)
			filename[i] = *p;
		filename[i] = 0;
		p++;

		for (;;) {
			min_num = (p[0] << 8) | p[1];
			p += 2;

			if (min_num == 0)
				break;

			max_num = (p[0] << 8) | p[1];
			p += 2;

			if (subr_id >= min_num && subr_id <= max_num) {
				_subroutineList = _subroutineListOrg;
				_tablesHeapPtr = _tablesHeapPtrOrg;
				_tablesHeapCurPos = _tablesHeapCurPosOrg;
				_stringIdLocalMin = 1;
				_stringIdLocalMax = 0;

				in = openTablesFile(filename);
				readSubroutineBlock(in);
				closeTablesFile(in);

				if (_game & GF_SIMON2) {
					_sound->loadSfxTable(_gameFile, _gameOffsetsPtr[atoi(filename + 6) - 1 + SOUND_INDEX_BASE]);
				} else if (_game & GF_TALKIE) {
					memcpy(filename, "SFXXXX", 6);
					_sound->readSfxFile(filename);
				}

				alignTableMem();

				_tablesheapPtrNew = _tablesHeapPtr;
				_tablesHeapCurPosNew = _tablesHeapCurPos;

				if (_tablesHeapCurPos > _tablesHeapSize)
					error("loadTablesIntoMem: Out of table memory");
				return;
			}
		}
	}

	debug(1,"loadTablesIntoMem: didn't find %d", subr_id);
}

void SimonEngine::playSting(uint a) {
	if (!midi._enable_sfx)
		return;

	char filename[15];

	File mus_file;
	uint16 mus_offset;

	sprintf(filename, "STINGS%i.MUS", _soundFileId);
	mus_file.open(filename);
	if (!mus_file.isOpen()) {
		warning("Can't load sound effect from '%s'", filename);
		return;
	}

	mus_file.seek(a * 2, SEEK_SET);
	mus_offset = mus_file.readUint16LE();
	if (mus_file.ioFailed())
		error("Can't read sting %d offset", a);

	mus_file.seek(mus_offset, SEEK_SET);
	midi.loadSMF(&mus_file, a, true);
	midi.startTrack(0);
}

Subroutine *SimonEngine::getSubroutineByID(uint subroutine_id) {
	Subroutine *cur;

	_subroutine = subroutine_id;

	for (cur = _subroutineList; cur; cur = cur->next) {
		if (cur->id == subroutine_id)
			return cur;
	}

	loadTablesIntoMem(subroutine_id);

	for (cur = _subroutineList; cur; cur = cur->next) {
		if (cur->id == subroutine_id)
			return cur;
	}

	debug(1,"getSubroutineByID: subroutine %d not found", subroutine_id);
	return NULL;
}

uint SimonEngine::loadTextFile_gme(const char *filename, byte *dst) {
	uint res;
	uint32 offs;
	uint32 size;

	res = atoi(filename + 4) + TEXT_INDEX_BASE - 1;
	offs = _gameOffsetsPtr[res];
	size = _gameOffsetsPtr[res + 1] - offs;

	resfile_read(dst, offs, size);

	return size;
}

File *SimonEngine::openTablesFile_gme(const char *filename) {
	uint res;
	uint32 offs;

	res = atoi(filename + 6) + TABLE_INDEX_BASE - 1;
	offs = _gameOffsetsPtr[res];

	_gameFile->seek(offs, SEEK_SET);
	return _gameFile;
}

uint SimonEngine::loadTextFile_simon1(const char *filename, byte *dst) {
	File fo;
	fo.open(filename);
	uint32 size;

	if (fo.isOpen() == false)
		error("loadTextFile: Can't open '%s'", filename);

	size = fo.size();

	if (fo.read(dst, size) != size)
		error("loadTextFile: fread failed");
	fo.close();

	return size;
}

File *SimonEngine::openTablesFile_simon1(const char *filename) {
	File *fo = new File();
	fo->open(filename);
	if (fo->isOpen() == false)
		error("openTablesFile: Can't open '%s'", filename);
	return fo;
}

uint SimonEngine::loadTextFile(const char *filename, byte *dst) {
	if (_game & GF_OLD_BUNDLE)
		return loadTextFile_simon1(filename, dst);
	else
		return loadTextFile_gme(filename, dst);
}

File *SimonEngine::openTablesFile(const char *filename) {
	if (_game & GF_OLD_BUNDLE)
		return openTablesFile_simon1(filename);
	else
		return openTablesFile_gme(filename);
}

void SimonEngine::closeTablesFile(File *in) {
	if (_game & GF_OLD_BUNDLE) {
		in->close();
		delete in;
	}
}

void SimonEngine::addTimeEvent(uint timeout, uint subroutine_id) {
	TimeEvent *te = (TimeEvent *)malloc(sizeof(TimeEvent)), *first, *last = NULL;
	time_t cur_time;

	time(&cur_time);

	te->time = cur_time + timeout - _base_time;
	te->subroutine_id = subroutine_id;

	first = _firstTimeStruct;
	while (first) {
		if (te->time <= first->time) {
			if (last) {
				last->next = te;
				te->next = first;
				return;
			}
			te->next = _firstTimeStruct;
			_firstTimeStruct = te;
			return;
		}

		last = first;
		first = first->next;
	}

	if (last) {
		last->next = te;
		te->next = NULL;
	} else {
		_firstTimeStruct = te;
		te->next = NULL;
	}
}

void SimonEngine::delTimeEvent(TimeEvent *te) {
	TimeEvent *cur;

	if (te == _pendingDeleteTimeEvent)
		_pendingDeleteTimeEvent = NULL;

	if (te == _firstTimeStruct) {
		_firstTimeStruct = te->next;
		free(te);
		return;
	}

	cur = _firstTimeStruct;
	if (cur == NULL)
		error("delTimeEvent: none available");

	for (;;) {
		if (cur->next == NULL)
			error("delTimeEvent: no such te");
		if (te == cur->next) {
			cur->next = te->next;
			free(te);
			return;
		}
		cur = cur->next;
	}
}

void SimonEngine::killAllTimers() {
	TimeEvent *cur, *next;

	for (cur = _firstTimeStruct; cur; cur = next) {
		next = cur->next;
		delTimeEvent(cur);
	}
}

bool SimonEngine::kickoffTimeEvents() {
	time_t cur_time;
	TimeEvent *te;
	bool result = false;

	time(&cur_time);
	cur_time -= _base_time;

	while ((te = _firstTimeStruct) != NULL && te->time <= (uint32)cur_time) {
		result = true;
		_pendingDeleteTimeEvent = te;
		invokeTimeEvent(te);
		if (_pendingDeleteTimeEvent) {
			_pendingDeleteTimeEvent = NULL;
			delTimeEvent(te);
		}
	}

	return result;
}

void SimonEngine::invokeTimeEvent(TimeEvent *te) {
	Subroutine *sub;

	_scriptCondA = 0;
	if (_runScriptReturn1)
		return;
	sub = getSubroutineByID(te->subroutine_id);
	if (sub != NULL)
		startSubroutineEx(sub);
	_runScriptReturn1 = false;
}

void SimonEngine::o_setup_cond_c() {

	setup_cond_c_helper();

	_objectItem = _hitAreaObjectItem;

	if (_objectItem == _dummyItem2)
		_objectItem = getItem1Ptr();

	if (_objectItem == _dummyItem3)
		_objectItem = derefItem(getItem1Ptr()->parent);

	if (_objectItem != NULL) {
		_scriptCondC = _objectItem->noun;
	} else {
		_scriptCondC = -1;
	}
}

void SimonEngine::setup_cond_c_helper() {
	HitArea *last;

	if (_game & GF_SIMON2) {
		_mouseCursor = 0;
		if (_hitAreaUnk4 != 999) {
			_mouseCursor = 9;
			_needHitAreaRecalc++;
			_hitAreaUnk4 = 0;
		}
	}

	_lastHitArea = 0;
	_hitAreaObjectItem = NULL;

	last = _lastHitArea2Ptr;
	defocusHitarea();
	_lastHitArea2Ptr = last;

	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = 0;
		_leftButtonDown = 0;

		do {
			if (_exitCutscene && (_bitArray[0] & 0x200)) {
				startSubroutine170();
				goto out_of_here;
			}

			delay(100);
		} while (_lastHitArea3 == (HitArea *) 0xFFFFFFFF || _lastHitArea3 == 0);

		if (_lastHitArea == NULL) {
		} else if (_lastHitArea->id == 0x7FFB) {
			handle_uparrow_hitarea(_lastHitArea->fcs);
		} else if (_lastHitArea->id == 0x7FFC) {
			handle_downarrow_hitarea(_lastHitArea->fcs);
		} else if (_lastHitArea->item_ptr != NULL) {
			_hitAreaObjectItem = _lastHitArea->item_ptr;
			_variableArray[60] = (_lastHitArea->flags & 1) ? (_lastHitArea->flags >> 8) : 0xFFFF;
			break;
		}
	}

out_of_here:
	_lastHitArea3 = 0;
	_lastHitArea = 0;
	_lastHitArea2Ptr = NULL;
}

void SimonEngine::startSubroutine170() {
	Subroutine *sub;

	_sound->stopVoice();

	sub = getSubroutineByID(170);
	if (sub != NULL)
		startSubroutineEx(sub);

	_runScriptReturn1 = true;
}

uint SimonEngine::get_fcs_ptr_3_index(FillOrCopyStruct *fcs) {
	uint i;

	for (i = 0; i != ARRAYSIZE(_fcsPtrArray3); i++)
		if (_fcsPtrArray3[i] == fcs)
			return i;

	error("get_fcs_ptr_3_index: not found");
	return 0;
}

void SimonEngine::lock() {
	_lockCounter++;
}

void SimonEngine::unlock() {
	_lockWord |= 1;

	if (_lockCounter != 0)
		_lockCounter--;

	_lockWord &= ~1;
}

void SimonEngine::handle_mouse_moved() {
	uint x;

	if (_lockCounter) {
		_system->showMouse(false);
		return;
	}

	_system->showMouse(true);
	pollMouseXY();

	if (_mouseX >= 32768)
		_mouseX = 0;
	if (_mouseX >= 638 / 2)
		_mouseX = 638 / 2;

	if (_mouseY >= 32768)
		_mouseY = 0;
	if (_mouseY >= 199)
		_mouseY = 199;

	if (_hitAreaUnk4) {
		uint id = 101;
		if (_mouseY >= 136)
			id = 102;
		if (_hitAreaUnk4 != id)
			hitarea_proc_1();
	}

	if (_game & GF_SIMON2) {
		if (_bitArray[4] & 0x8000) {
			if (!_vgaVar9) {
				if (_mouseX >= 630 / 2 || _mouseX < 9)
					goto get_out2;
				_vgaVar9 = 1;
			}
			if (_scrollCount == 0) {
				if (_mouseX >= 631 / 2) {
					if (_scrollX != _scrollXMax)
						_scrollFlag = 1;
				} else if (_mouseX < 8) {
					if (_scrollX != 0)
						_scrollFlag = -1;
				}
			}
		} else {
		get_out2:;
			_vgaVar9 = 0;
		}
	}

	if (_mouseX != _mouseXOld || _mouseY != _mouseYOld)
		_needHitAreaRecalc++;

	x = 0;
	if (_lastHitArea3 == 0 && _leftButtonDown != 0) {
		_leftButtonDown = 0;
		x = 1;
	} else {
		if (_hitarea_unk_3 == 0 && _needHitAreaRecalc == 0)
			goto get_out;
	}

	setup_hitarea_from_pos(_mouseX, _mouseY, x);
	_lastHitArea3 = _lastHitArea;
	if (x == 1 && _lastHitArea == NULL)
		_lastHitArea3 = (HitArea *) - 1;

get_out:
	draw_mouse_pointer();
	_needHitAreaRecalc = 0;
}

void SimonEngine::fcs_unk_proc_1(uint fcs_index, Item *item_ptr, int unk1, int unk2) {
	Item *item_ptr_org = item_ptr;
	FillOrCopyStruct *fcs_ptr;
	uint width_div_3, height_div_3;
	uint j, k, i, num_sibs_with_flag;
	bool item_again;
	uint x_pos, y_pos;

	fcs_ptr = _fcsPtrArray3[fcs_index & 7];

	if (!(_game & GF_SIMON2)) {
		width_div_3 = fcs_ptr->width / 3;
		height_div_3 = fcs_ptr->height / 3;
	} else {
		width_div_3 = 100;
		height_div_3 = 40;
	}

	i = 0;

	if (fcs_ptr == NULL)
		return;

	if (fcs_ptr->fcs_data)
		fcs_unk1(fcs_index);

	fcs_ptr->fcs_data = (FillOrCopyData *) malloc(sizeof(FillOrCopyData));
	fcs_ptr->fcs_data->item_ptr = item_ptr;
	fcs_ptr->fcs_data->unk3 = -1;
	fcs_ptr->fcs_data->unk4 = -1;
	fcs_ptr->fcs_data->unk1 = unk1;
	fcs_ptr->fcs_data->unk2 = unk2;

	item_ptr = derefItem(item_ptr->child);

	while (item_ptr && unk1-- != 0) {
		num_sibs_with_flag = 0;
		while (item_ptr && width_div_3 > num_sibs_with_flag) {
			if ((unk2 == 0 || item_ptr->classFlags & unk2) && has_item_childflag_0x10(item_ptr))
				if (!(_game & GF_SIMON2)) {
					num_sibs_with_flag++;
				} else {
					num_sibs_with_flag += 20;
				}
			item_ptr = derefItem(item_ptr->sibling);
		}
	}

	if (item_ptr == NULL) {
		fcs_ptr->fcs_data->unk1 = 0;
		item_ptr = derefItem(item_ptr_org->child);
	}

	x_pos = 0;
	y_pos = 0;
	item_again = false;
	k = 0;
	j = 0;

	while (item_ptr) {
		if ((unk2 == 0 || item_ptr->classFlags & unk2) && has_item_childflag_0x10(item_ptr)) {
			if (item_again == false) {
				fcs_ptr->fcs_data->e[k].item = item_ptr;
				if (!(_game & GF_SIMON2)) {
					draw_icon_c(fcs_ptr, item_get_icon_number(item_ptr), x_pos * 3, y_pos);
					fcs_ptr->fcs_data->e[k].hit_area =
						setup_icon_hit_area(fcs_ptr, x_pos * 3, y_pos,
																item_get_icon_number(item_ptr), item_ptr);
				} else {
					draw_icon_c(fcs_ptr, item_get_icon_number(item_ptr), x_pos, y_pos);
					fcs_ptr->fcs_data->e[k].hit_area =
						setup_icon_hit_area(fcs_ptr, x_pos, y_pos, item_get_icon_number(item_ptr), item_ptr);
				}
				k++;
			} else {
				fcs_ptr->fcs_data->e[k].item = NULL;
				j = 1;
			}
			x_pos += (_game & GF_SIMON2) ? 20 : 1;

			if (x_pos >= width_div_3) {
				x_pos = 0;

				y_pos += (_game & GF_SIMON2) ? 20 : 1;
				if (y_pos >= height_div_3)
					item_again = true;
			}
		}
		item_ptr = derefItem(item_ptr->sibling);
	}

	fcs_ptr->fcs_data->e[k].item = NULL;

	if (j != 0 || fcs_ptr->fcs_data->unk1 != 0) {
		fcs_unk_proc_2(fcs_ptr, fcs_index);
	}
}

void SimonEngine::fcs_unk_proc_2(FillOrCopyStruct *fcs, uint fcs_index) {
	setup_hit_areas(fcs, fcs_index);

	fcs->fcs_data->unk3 = _scrollUpHitArea;
	fcs->fcs_data->unk4 = _scrollDownHitArea;
}

void SimonEngine::setup_hit_areas(FillOrCopyStruct *fcs, uint fcs_index) {
	HitArea *ha;

	ha = findEmptyHitArea();
	_scrollUpHitArea = ha - _hitAreas;
	if (!(_game & GF_SIMON2)) {
		ha->x = 308;
		ha->y = 149;
		ha->width = 12;
		ha->height = 17;
		ha->flags = 0x24;
		ha->id = 0x7FFB;
		ha->layer = 100;
		ha->fcs = fcs;
		ha->unk3 = 1;
	} else {
		ha->x = 81;
		ha->y = 158;
		ha->width = 12;
		ha->height = 26;
		ha->flags = 36;
		ha->id = 0x7FFB;
		ha->layer = 100;
		ha->fcs = fcs;
		ha->unk3 = 1;
	}

	ha = findEmptyHitArea();
	_scrollDownHitArea = ha - _hitAreas;

	if (!(_game & GF_SIMON2)) {
		ha->x = 308;
		ha->y = 176;
		ha->width = 12;
		ha->height = 17;
		ha->flags = 0x24;
		ha->id = 0x7FFC;
		ha->layer = 100;
		ha->fcs = fcs;
		ha->unk3 = 1;

		// Simon1 specific
		o_kill_sprite_simon1(0x80);
		loadSprite(0, 1, 0x80, 0, 0, 0xE);
	} else {
		ha->x = 227;
		ha->y = 162;
		ha->width = 12;
		ha->height = 26;
		ha->flags = 36;
		ha->id = 0x7FFC;
		ha->layer = 100;
		ha->fcs = fcs;
		ha->unk3 = 1;
	}
}


bool SimonEngine::has_item_childflag_0x10(Item *item) {
	Child2 *child = (Child2 *)findChildOfType(item, 2);
	return child && (child->avail_props & 0x10) != 0;
}

uint SimonEngine::item_get_icon_number(Item *item) {
	Child2 *child = (Child2 *)findChildOfType(item, 2);
	uint offs;

	if (child == NULL || !(child->avail_props & 0x10))
		return 0;

	offs = getOffsetOfChild2Param(child, 0x10);
	return child->array[offs];
}

void SimonEngine::f10_key() {
	HitArea *ha, *dha;
	uint count;
	uint y_, x_;
	byte *dst;
	uint b, color;

	_lockWord |= 0x8000;

	if (_game & GF_SIMON2)
		color = 0xec;
	else
		color = 0xe1;

	uint limit = (_game & GF_SIMON2) ? 200 : 134;

	for (int i = 0; i < 5; i++) {
		ha = _hitAreas;
		count = ARRAYSIZE(_hitAreas);

		timer_vga_sprites();

		do {
			if (ha->id != 0 && ha->flags & 0x20 && !(ha->flags & 0x40)) {

				dha = _hitAreas;
				if (ha->flags & 1) {
					while (dha != ha && dha->flags != ha->flags)
						++dha;
					if (dha != ha && dha->flags == ha->flags)
						continue;
				} else {
					dha = _hitAreas;
					while (dha != ha && dha->item_ptr != ha->item_ptr)
						++dha;
					if (dha != ha && dha->item_ptr == ha->item_ptr)
						continue;
				}

				if (ha->y >= limit || ((_game & GF_SIMON2) && ha->y >= _vgaVar8))
					continue;

				y_ = (ha->height >> 1) - 4 + ha->y;

				x_ = (ha->width >> 1) - 4 + ha->x - (_scrollX << 3);

				if (x_ >= 0x137)
					continue;

				dst = dx_lock_attached();

				dst += (((_dxSurfacePitch >> 2) * y_) << 2) + x_;

				b = _dxSurfacePitch;
				dst[4] = color;
				dst[b+1] = color;
				dst[b+4] = color;
				dst[b+7] = color;
				b += _dxSurfacePitch;
				dst[b+2] = color;
				dst[b+4] = color;
				dst[b+6] = color;
				b += _dxSurfacePitch;
				dst[b+3] = color;
				dst[b+5] = color;
				b += _dxSurfacePitch;
				dst[b] = color;
				dst[b+1] = color;
				dst[b+2] = color;
				dst[b+6] = color;
				dst[b+7] = color;
				dst[b+8] = color;
				b += _dxSurfacePitch;
				dst[b+3] = color;
				dst[b+5] = color;
				b += _dxSurfacePitch;
				dst[b+2] = color;
				dst[b+4] = color;
				dst[b+6] = color;
				b += _dxSurfacePitch;
				dst[b+1] = color;
				dst[b+4] = color;
				dst[b+7] = color;
				b += _dxSurfacePitch;
				dst[b+4] = color;

				dx_unlock_attached();
			}
		} while (ha++, --count);

		dx_update_screen_and_palette();
		delay(100);
		timer_vga_sprites();
		dx_update_screen_and_palette();
		delay(100);
	}

	_lockWord &= ~0x8000;
}

void SimonEngine::hitarea_stuff() {
	HitArea *ha;
	uint id;

	_leftButtonDown = 0;
	_lastHitArea = 0;
	_verbHitArea = 0;
	_hitAreaSubjectItem = NULL;
	_hitAreaObjectItem = NULL;

	hitarea_proc_1();

startOver:
	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		for (;;) {
			if (_keyPressed == 35)
				f10_key();
			processSpecialKeys();
			if (_lastHitArea3 == (HitArea *) 0xFFFFFFFF)
				goto startOver;
			if (_lastHitArea3 != 0)
				break;
			hitarea_stuff_helper();
			delay(100);
		}

		ha = _lastHitArea;

		if (ha == NULL) {
		} else if (ha->id == 0x7FFB) {
			handle_uparrow_hitarea(ha->fcs);
		} else if (ha->id == 0x7FFC) {
			handle_downarrow_hitarea(ha->fcs);
		} else if (ha->id >= 101 && ha->id < 113) {
			_verbHitArea = ha->unk3;
			handle_verb_hitarea(ha);
			_hitAreaUnk4 = 0;
		} else {
			if ((_verbHitArea != 0 || _hitAreaSubjectItem != ha->item_ptr && ha->flags & 0x80) &&
					ha->item_ptr) {
			if_1:;
				_hitAreaSubjectItem = ha->item_ptr;
				id = 0xFFFF;
				if (ha->flags & 1)
					id = ha->flags >> 8;
				_variableArray[60] = id;
				new_current_hitarea(ha);
				if (_verbHitArea != 0)
					break;
			} else {
				// else 1
				if (ha->unk3 == 0) {
					if (ha->item_ptr)
						goto if_1;
				} else {
					_verbHitArea = ha->unk3 & 0xBFFF;
					if (ha->unk3 & 0x4000) {
						_hitAreaSubjectItem = ha->item_ptr;
						break;
					}
					if (_hitAreaSubjectItem != NULL)
						break;
				}
			}
		}
	}

	_needHitAreaRecalc++;
}

void SimonEngine::hitarea_stuff_helper() {
	time_t cur_time;

	if (!(_game & GF_SIMON2)) {
		uint subr_id = _variableArray[254];
		if (subr_id != 0) {
			Subroutine *sub = getSubroutineByID(subr_id);
			if (sub != NULL) {
				startSubroutineEx(sub);
				startUp_helper_2();
			}
			_variableArray[254] = 0;
			_runScriptReturn1 = false;
		}
	} else {
		if (_variableArray[254] || _variableArray[249]) {
			hitarea_stuff_helper_2();
		}
	}

	time(&cur_time);
	if ((uint) cur_time != _lastTime) {
		_lastTime = cur_time;
		if (kickoffTimeEvents())
			startUp_helper_2();
	}
}

// Simon 2 specific
void SimonEngine::hitarea_stuff_helper_2() {
	uint subr_id;
	Subroutine *sub;

	subr_id = _variableArray[249];
	if (subr_id != 0) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[249] = 0;
			startSubroutineEx(sub);
			startUp_helper_2();
		}
		_variableArray[249] = 0;
	}

	subr_id = _variableArray[254];
	if (subr_id != 0) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[254] = 0;
			startSubroutineEx(sub);
			startUp_helper_2();
		}
		_variableArray[254] = 0;
	}

	_runScriptReturn1 = false;
}

void SimonEngine::startUp_helper_2() {
	if (!_mortalFlag) {
		_mortalFlag = true;
		showmessage_print_char(0);
		_fcsUnk1 = 0;
		if (_fcsPtrArray3[0] != 0) {
			_fcsPtr1 = _fcsPtrArray3[0];
			showmessage_helper_3(_fcsPtr1->textLength, _fcsPtr1->textMaxLength);
		}
		_mortalFlag = false;
	}
}

void SimonEngine::pollMouseXY() {
	_mouseX = _sdlMouseX;
	_mouseY = _sdlMouseY;
}

void SimonEngine::handle_verb_clicked(uint verb) {
	Subroutine *sub;
	int result;

	_objectItem = _hitAreaObjectItem;
	if (_objectItem == _dummyItem2) {
		_objectItem = getItem1Ptr();
	}
	if (_objectItem == _dummyItem3) {
		_objectItem = derefItem(getItem1Ptr()->parent);
	}

	_subjectItem = _hitAreaSubjectItem;
	if (_subjectItem == _dummyItem2) {
		_subjectItem = getItem1Ptr();
	}
	if (_subjectItem == _dummyItem3) {
		_subjectItem = derefItem(getItem1Ptr()->parent);
	}

	if (_subjectItem) {
		_scriptCondB = _subjectItem->noun;
	} else {
		_scriptCondB = -1;
	}

	if (_objectItem) {
		_scriptCondC = _objectItem->noun;
	} else {
		_scriptCondC = -1;
	}

	_scriptCondA = _verbHitArea;

	sub = getSubroutineByID(0);
	if (sub == NULL)
		return;

	result = startSubroutine(sub);
	if (result == -1)
		showMessageFormat("I don't understand");

	_runScriptReturn1 = false;

	sub = getSubroutineByID(100);
	if (sub)
		startSubroutine(sub);

	if (_game & GF_SIMON2)
		_runScriptReturn1 = false;

	startUp_helper_2();
}

TextLocation *SimonEngine::getTextLocation(uint a) {
	switch (a) {
	case 1:
		return &_textLocation1;
	case 2:
		return &_textLocation2;
	case 101:
		return &_textLocation3;
	case 102:
		return &_textLocation4;
	default:
		error("text, invalid value %d", a);
	}
	return NULL;
}

void SimonEngine::o_print_str() {
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	uint string_id = getNextStringID();
	const byte *string_ptr = NULL;
	uint speech_id = 0;
	TextLocation *tl;

	if (string_id != 0xFFFF)
		string_ptr = getStringPtrByID(string_id);

	if (_game & GF_TALKIE)
		speech_id = (uint16)getNextWord();

	tl = getTextLocation(vgaSpriteId);

	if (_speech && speech_id != 0)
		talk_with_speech(speech_id, vgaSpriteId);
	if ((_game & GF_SIMON2) && (_game & GF_TALKIE) && speech_id == 0)
		o_kill_sprite_simon2(2, vgaSpriteId + 2);

	if (string_ptr != NULL && (speech_id == 0 || _subtitles))
		talk_with_text(vgaSpriteId, color, (const char *)string_ptr, tl->x, tl->y, tl->width);

}

void SimonEngine::ensureVgaResLoadedC(uint vga_res) {
	_lockWord |= 0x80;
	ensureVgaResLoaded(vga_res);
	_lockWord &= ~0x80;
}

void SimonEngine::ensureVgaResLoaded(uint vga_res) {
	VgaPointersEntry *vpe;

	CHECK_BOUNDS(vga_res, _vgaBufferPointers);

	vpe = _vgaBufferPointers + vga_res;
	if (vpe->vgaFile1 != NULL)
		return;

	vpe->vgaFile2 = read_vga_from_datfile_2(vga_res * 2 + 1);
	vpe->vgaFile1 = read_vga_from_datfile_2(vga_res * 2);

}

byte *SimonEngine::setup_vga_destination(uint32 size) {
	byte *dest, *end;

	_videoVar4 = 0;

	for (;;) {
		dest = _vgaBufFreeStart;

		end = dest + size;

		if (end >= _vgaBufEnd) {
			_vgaBufFreeStart = _vgaBufStart;
		} else {
			_videoVar5 = false;
			vga_buf_unk_proc3(end);
			if (_videoVar5)
				continue;
			vga_buf_unk_proc1(end);
			if (_videoVar5)
				continue;
			delete_memptr_range(end);
			_vgaBufFreeStart = end;
			return dest;
		}
	}
}

void SimonEngine::setup_vga_file_buf_pointers() {
	byte *alloced;

	alloced = (byte *)malloc(VGA_MEM_SIZE);

	_vgaBufFreeStart = alloced;
	_vgaBufStart = alloced;
	_vgaFileBufOrg = alloced;
	_vgaFileBufOrg2 = alloced;
	_vgaBufEnd = alloced + VGA_MEM_SIZE;
}

void SimonEngine::vga_buf_unk_proc3(byte *end) {
	VgaPointersEntry *vpe;

	if (_videoVar7 == 0xFFFF)
		return;

	if (_videoVar4 == 2)
		error("vga_buf_unk_proc3: _videoVar4 == 2");

	vpe = &_vgaBufferPointers[_videoVar7];

	if (_vgaBufFreeStart <= vpe->vgaFile1 && end >= vpe->vgaFile1 ||
			_vgaBufFreeStart <= vpe->vgaFile2 && end >= vpe->vgaFile2) {
		_videoVar5 = 1;
		_videoVar4++;
		_vgaBufFreeStart = vpe->vgaFile1 + 0x5000;
	} else {
		_videoVar5 = 0;
	}
}

void SimonEngine::vga_buf_unk_proc1(byte *end) {
	VgaSprite *vsp;
	if (_lockWord & 0x20)
		return;

	for (vsp = _vgaSprites; vsp->id; vsp++) {
		vga_buf_unk_proc2(vsp->fileId, end);
		if (_videoVar5 == true)
			return;
	}
}

void SimonEngine::delete_memptr_range(byte *end) {
	uint count = ARRAYSIZE(_vgaBufferPointers);
	VgaPointersEntry *vpe = _vgaBufferPointers;
	do {
		if (_vgaBufFreeStart <= vpe->vgaFile1 && end >= vpe->vgaFile1 ||
				_vgaBufFreeStart <= vpe->vgaFile2 && end >= vpe->vgaFile2) {
			vpe->dd = 0;
			vpe->vgaFile1 = NULL;
			vpe->vgaFile2 = NULL;
		}

	} while (++vpe, --count);
}

void SimonEngine::vga_buf_unk_proc2(uint a, byte *end) {
	VgaPointersEntry *vpe;

	vpe = &_vgaBufferPointers[a];

	if (_vgaBufFreeStart <= vpe->vgaFile1 && end >= vpe->vgaFile1 ||
			_vgaBufFreeStart <= vpe->vgaFile2 && end >= vpe->vgaFile2) {
		_videoVar5 = true;
		_videoVar4++;
		_vgaBufFreeStart = vpe->vgaFile1 + 0x5000;
	} else {
		_videoVar5 = false;
	}
}

void SimonEngine::o_clear_vgapointer_entry(uint a) {
	VgaPointersEntry *vpe;

	vpe = &_vgaBufferPointers[a];

	vpe->dd = 0;
	vpe->vgaFile1 = NULL;
	vpe->vgaFile2 = NULL;
}

void SimonEngine::o_set_video_mode(uint mode, uint vga_res) {
	if (mode == 4)
		vc29_stopAllSounds();

	if (_lockWord & 0x10)
		error("o_set_video_mode_ex: _lockWord & 0x10");

	set_video_mode_internal(mode, vga_res);
}

void SimonEngine::set_video_mode_internal(uint mode, uint vga_res_id) {
	uint num, num_lines;
	VgaPointersEntry *vpe;
	byte *bb, *b;
	// uint16 count;
	const byte *vc_ptr_org;

	_windowNum = mode;
	_lockWord |= 0x20;

	if (vga_res_id == 0) {

		if (!(_game & GF_SIMON2)) {
			_unkPalFlag = true;
		} else {
			_dxUse3Or4ForLock = true;
			_vgaVar6 = true;
		}
	}

	_vgaCurFile2 = num = vga_res_id / 100;

	for (;;) {
		vpe = &_vgaBufferPointers[num];

		_curVgaFile1 = vpe->vgaFile1;
		_curVgaFile2 = vpe->vgaFile2;

		if (vpe->vgaFile1 != NULL)
			break;

		ensureVgaResLoaded(num);
	}

	// ensure flipping complete

	bb = _curVgaFile1;

	if (_game == GAME_FEEBLEFILES) {
		b = bb + READ_LE_UINT16(&((VgaFileHeader_Feeble *) bb)->hdr2_start);
		//count = READ_LE_UINT16(&((VgaFileHeader2_Feeble *) b)->imageCount);
		b = bb + READ_LE_UINT16(&((VgaFileHeader2_Feeble *) b)->imageTable);

		while (READ_LE_UINT16(&((ImageHeader_Feeble *) b)->id) != vga_res_id)
			b += sizeof(ImageHeader_Feeble);
	} else {
		b = bb + READ_BE_UINT16(&((VgaFileHeader_Simon *) bb)->hdr2_start);
		//count = READ_BE_UINT16(&((VgaFileHeader2_Simon *) b)->imageCount);
		b = bb + READ_BE_UINT16(&((VgaFileHeader2_Simon *) b)->imageTable);

		while (READ_BE_UINT16(&((ImageHeader_Simon *) b)->id) != vga_res_id)
			b += sizeof(ImageHeader_Simon);
	}

	if ((_game & GF_SIMON1) && vga_res_id == 16300) {
		dx_clear_attached_from_top(134);
		_usePaletteDelay = true;
	} else {
		_scrollX = 0;
		_scrollXMax = 0;
		_scrollCount = 0;
		_scrollFlag = 0;
		_scrollHeight = 134;
		if (_variableArray[34] != -1)
			_variableArray[251] = 0;
	}

	vc_ptr_org = _vcPtr;

	if (_game == GAME_FEEBLEFILES) {
		_vcPtr = _curVgaFile1 + READ_LE_UINT16(&((ImageHeader_Feeble *) b)->scriptOffs);
	} else {
		_vcPtr = _curVgaFile1 + READ_BE_UINT16(&((ImageHeader_Simon *) b)->scriptOffs);
	}
	//dump_vga_script(_vcPtr, num, vga_res_id);
	run_vga_script();
	_vcPtr = vc_ptr_org;


	if (_game & GF_SIMON2) {
		if (!_dxUse3Or4ForLock) {
			num_lines = _windowNum == 4 ? 134 : 200;
			_vgaVar8 = num_lines;
			dx_copy_from_attached_to_2(0, 0, _screenWidth, num_lines);
			dx_copy_from_attached_to_3(num_lines);
			_syncFlag2 = 1;
		}
		_dxUse3Or4ForLock = false;
	} else {
		// Allow one section of Simon the Sorcerer 1 introduction to be displayed
		// in lower half of screen
		if (_subroutine == 2923 || _subroutine == 2926)
			num_lines = 200;
		else
			num_lines = _windowNum == 4 ? 134 : 200;
		dx_copy_from_attached_to_2(0, 0, _screenWidth, num_lines);
		dx_copy_from_attached_to_3(num_lines);
		_syncFlag2 = 1;
		_timer5 = 0;
	}

	_lockWord &= ~0x20;

	if (!(_game & GF_SIMON2)) {
		if (_unkPalFlag) {
			_unkPalFlag = false;
			while (_paletteColorCount != 0) {
				delay(10);
			}
		}
	}
}

void SimonEngine::o_fade_to_black() {
	uint i;

	memcpy(_videoBuf1, _paletteBackup, 256 * sizeof(uint32));

	i = NUM_PALETTE_FADEOUT;
	do {
		palette_fadeout((uint32 *)_videoBuf1, 32);
		palette_fadeout((uint32 *)_videoBuf1 + 32 + 16, 144);
		palette_fadeout((uint32 *)_videoBuf1 + 32 + 16 + 144 + 16, 48);

		_system->setPalette(_videoBuf1, 0, 256);
		if (_fade)
			_system->updateScreen();
		delay(5);
	} while (--i);

	memcpy(_paletteBackup, _videoBuf1, 256 * sizeof(uint32));
	memcpy(_palette, _videoBuf1, 256 * sizeof(uint32));
}

void SimonEngine::delete_vga_timer(VgaTimerEntry * vte) {
	_lockWord |= 1;

	if (vte + 1 <= _nextVgaTimerToProcess) {
		_nextVgaTimerToProcess--;
	}

	do {
		memcpy(vte, vte + 1, sizeof(VgaTimerEntry));
		vte++;
	} while (vte->delay);

	_lockWord &= ~1;
}

void SimonEngine::expire_vga_timers() {
	VgaTimerEntry *vte = _vgaTimerList;

	_vgaTickCounter++;

	while (vte->delay) {
		if (!--vte->delay) {
			uint16 cur_file = vte->cur_vga_file;
			uint16 cur_unk = vte->sprite_id;
			const byte *script_ptr = vte->script_pointer;

			_nextVgaTimerToProcess = vte + 1;
			delete_vga_timer(vte);

			if ((_game & GF_SIMON2) && script_ptr == NULL) {
				// special scroll timer
				scroll_timeout();
			} else {
				vc_resume_sprite(script_ptr, cur_file, cur_unk);
			}
			vte = _nextVgaTimerToProcess;
		} else {
			vte++;
		}
	}
}

// Simon2 specific
void SimonEngine::scroll_timeout() {
	if (_scrollCount == 0)
		return;

	if (_scrollCount < 0) {
		if (_scrollFlag != -1) {
			_scrollFlag = -1;
			if (++_scrollCount == 0)
				return;
		}
	} else {
		if (_scrollFlag != 1) {
			_scrollFlag = 1;
			if (--_scrollCount == 0)
				return;
		}
	}

	add_vga_timer(6, NULL, 0, 0);
}

void SimonEngine::vc_resume_sprite(const byte *code_ptr, uint16 cur_file, uint16 cur_sprite) {
	VgaPointersEntry *vpe;

	_vgaCurSpriteId = cur_sprite;

	_vgaCurFileId = cur_file;
	_vgaCurFile2 = cur_file;
	vpe = &_vgaBufferPointers[cur_file];

	_curVgaFile1 = vpe->vgaFile1;
	_curVgaFile2 = vpe->vgaFile2;

	_vcPtr = code_ptr;

	run_vga_script();
}

void SimonEngine::add_vga_timer(uint num, const byte *code_ptr, uint cur_sprite, uint cur_file) {
	VgaTimerEntry *vte;

	// When Simon talks to the Golum about stew in French version of
	// Simon the Sorcerer 1 the code_ptr is at wrong location for
	// sprite 200. This  was a bug in the original game, which
	// caused several glitches in this scene.
	// We work around the problem by correcting the code_ptr for sprite
	// 200 in this scene, if it is wrong.
	if (!(_game & GF_SIMON2) && (_language == 2) &&
		(code_ptr - _vgaBufferPointers[cur_file].vgaFile1 == 4) && (cur_sprite == 200) && (cur_file == 2))
		code_ptr += 0x66;

	_lockWord |= 1;

	for (vte = _vgaTimerList; vte->delay; vte++) {
	}

	vte->delay = num;
	vte->script_pointer = code_ptr;
	vte->sprite_id = cur_sprite;
	vte->cur_vga_file = cur_file;

	_lockWord &= ~1;
}

void SimonEngine::o_force_unlock() {
	if (_game & GF_SIMON2 && _bitArray[4] & 0x8000)
		_mouseCursor = 0;
	_lockCounter = 0;
}

void SimonEngine::o_force_lock() {
	if (_game & GF_SIMON2) {
		_lockWord |= 0x8000;
		vc34_setMouseOff();
		_lockWord &= ~0x8000;
	} else {
		_lockWord |= 0x4000;
		vc34_setMouseOff();
		_lockWord &= ~0x4000;
	}
}

void SimonEngine::o_wait_for_vga(uint a) {
	_vgaWaitFor = a;
	_timer1 = 0;
	_exitCutscene = false;
	_skipSpeech = false;
	while (_vgaWaitFor != 0) {
		if (_skipSpeech && _game & GF_SIMON2) {
			if (_vgaWaitFor == 200 && !vc_get_bit(14)) {
				skip_speech();
				break;
			}
		} else if (_exitCutscene) {
			if (vc_get_bit(9)) {
				startSubroutine170();
				break;
			}
		} else {
			processSpecialKeys();
		}

		delay(10);

		if (_game & GF_SIMON2) {
			if (_timer1 >= 1000) {
				warning("wait timed out");
				break;
			}
		} else if (_timer1 >= 500) {
			warning("wait timed out");
			break;
		}

	}
}

void SimonEngine::skip_speech() {
	_sound->stopVoice();
	if (!(_bitArray[1] & 0x1000)) {
		_bitArray[0] |= 0x4000;
		_variableArray[100] = 5;
		loadSprite(4, 1, 0x1e, 0, 0, 0);
		o_wait_for_vga(0x82);
		o_kill_sprite_simon2(2, 1);
	}
}

void SimonEngine::timer_vga_sprites() {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	const byte *vc_ptr_org = _vcPtr;
	uint16 params[5];							// parameters to vc10

	if (_videoVar9 == 2)
		_videoVar9 = 1;

	if (_game & GF_SIMON2 && _scrollFlag) {
		timer_vga_sprites_helper();
	}

	vsp = _vgaSprites;

	while (vsp->id != 0) {
		vsp->windowNum &= 0x7FFF;

		vpe = &_vgaBufferPointers[vsp->fileId];
		_curVgaFile1 = vpe->vgaFile1;
		_curVgaFile2 = vpe->vgaFile2;
		_windowNum = vsp->windowNum;
		_vgaCurSpriteId = vsp->id;

		params[0] = readUint16Wrapper(&vsp->image);
		params[1] = readUint16Wrapper(&vsp->palette);
		params[2] = readUint16Wrapper(&vsp->x);
		params[3] = readUint16Wrapper(&vsp->y);

		if (_game & GF_SIMON2) {
			*(byte *)(&params[4]) = (byte)vsp->flags;
		} else {
			params[4] = READ_BE_UINT16(&vsp->flags);
		}

		_vcPtr = (const byte *)params;
		vc10_draw();

		vsp++;
	}

	if (_drawImagesDebug)
		memset(_sdl_buf_attached, 0, _screenWidth * _screenHeight);

	_videoVar8++;
	_vcPtr = vc_ptr_org;
}

void SimonEngine::timer_vga_sprites_helper() {
	byte *dst = dx_lock_2();
	const byte *src;
	uint x;

	if (_scrollFlag < 0) {
		memmove(dst + 8, dst, _screenWidth * _scrollHeight - 8);
	} else {
		memmove(dst, dst + 8, _screenWidth * _scrollHeight - 8);
	}

	x = _scrollX - 1;

	if (_scrollFlag > 0) {
		dst += _screenWidth - 8;
		x += 41;
	}

	src = _scrollImage + x * 4;
	decodeStripA(dst, src + READ_BE_UINT32(src), _scrollHeight);

	dx_unlock_2();


	memcpy(_sdl_buf_attached, _sdl_buf, _screenWidth * _screenHeight);
	dx_copy_from_attached_to_3(_scrollHeight);


	_scrollX += _scrollFlag;

	vc_write_var(0xfB, _scrollX);

	_scrollFlag = 0;
}

void SimonEngine::timer_vga_sprites_2() {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	const byte *vc_ptr_org = _vcPtr;
	uint16 params[5];							// parameters to vc10_draw

	if (_videoVar9 == 2)
		_videoVar9 = 1;

	vsp = _vgaSprites;
	while (vsp->id != 0) {
		vsp->windowNum &= 0x7FFF;

		vpe = &_vgaBufferPointers[vsp->fileId];
		_curVgaFile1 = vpe->vgaFile1;
		_curVgaFile2 = vpe->vgaFile2;
		_windowNum = vsp->windowNum;
		_vgaCurSpriteId = vsp->id;

		if (vsp->image)
			fprintf(_dumpFile, "id:%5d image:%3d base-color:%3d x:%3d y:%3d flags:%x\n",
							vsp->id, vsp->image, vsp->palette, vsp->x, vsp->y, vsp->flags);
		params[0] = readUint16Wrapper(&vsp->image);
		params[1] = readUint16Wrapper(&vsp->palette);
		params[2] = readUint16Wrapper(&vsp->x);
		params[3] = readUint16Wrapper(&vsp->y);
		params[4] = readUint16Wrapper(&vsp->flags);
		_vcPtr = (const byte *)params;
		vc10_draw();

		vsp++;
	}

	_videoVar8++;
	_vcPtr = vc_ptr_org;
}

void SimonEngine::timer_proc1() {
	_timer4++;

	if (_game & GF_SIMON2) {
		if (_lockWord & 0x80E9 || _lockWord & 2)
			return;
	} else {
		if (_lockWord & 0xC0E9 || _lockWord & 2)
			return;
	}

	_timer1++;

	_lockWord |= 2;

	if (!(_lockWord & 0x10)) {
		expire_vga_timers();
		expire_vga_timers();
		_syncFlag2 ^= 1;
		_cepeFlag ^= 1;
		if (!_cepeFlag)
			expire_vga_timers();

		if (_lockCounter != 0 && !_syncFlag2) {
			_lockWord &= ~2;
			return;
		}
	}

	timer_vga_sprites();
	if (_drawImagesDebug)
		timer_vga_sprites_2();

	if (_copyPartialMode == 1) {
		dx_copy_from_2_to_attached(80, 46, 208 - 80, 94 - 46);
	}

	if (_copyPartialMode == 2) {
		// copy partial from attached to 2
		dx_copy_from_attached_to_2(176, 61, _screenWidth - 176, 134 - 61);
		_copyPartialMode = 0;
	}

	if (_videoVar8) {
		handle_mouse_moved();
		dx_update_screen_and_palette();
		_videoVar8 = false;
	}

	_lockWord &= ~2;
}

void SimonEngine::timer_callback() {
	if (_timer5 != 0) {
		_syncFlag2 = true;
		_timer5--;
	} else {
		timer_proc1();
	}
}

void SimonEngine::fcs_setTextColor(FillOrCopyStruct *fcs, uint value) {
	fcs->text_color = value;
}

void SimonEngine::o_vga_reset() {
	if (_game & GF_SIMON2) {
		_lockWord |= 0x8000;
		vc27_resetSprite();
		_lockWord &= ~0x8000;
	} else {
		_lockWord |= 0x4000;
		vc27_resetSprite();
		_lockWord &= ~0x4000;
	}
}

bool SimonEngine::itemIsSiblingOf(uint16 a) {
	Item *item;

	CHECK_BOUNDS(a, _vcItemArray);

	item = _vcItemArray[a];
	if (item == NULL)
		return true;

	return getItem1Ptr()->parent == item->parent;
}

bool SimonEngine::itemIsParentOf(uint16 a, uint16 b) {
	Item *item_a, *item_b;

	CHECK_BOUNDS(a, _vcItemArray);
	CHECK_BOUNDS(b, _vcItemArray);

	item_a = _vcItemArray[a];
	item_b = _vcItemArray[b];

	if (item_a == NULL || item_b == NULL)
		return true;

	return derefItem(item_a->parent) == item_b;
}

bool SimonEngine::vc_maybe_skip_proc_1(uint16 a, int16 b) {
	Item *item;

	CHECK_BOUNDS(a, _vcItemArray);

	item = _vcItemArray[a];
	if (item == NULL)
		return true;
	return item->state == b;
}

// OK
void SimonEngine::fcs_delete(uint a) {
	if (_fcsPtrArray3[a] == NULL)
		return;
	fcs_unk1(a);
	video_copy_if_flag_0x8_c(_fcsPtrArray3[a]);
	_fcsPtrArray3[a] = NULL;
	if (_fcsUnk1 == a) {
		_fcsPtr1 = NULL;
		fcs_unk_2(0);
	}
}

// OK
void SimonEngine::fcs_unk_2(uint a) {
	a &= 7;

	if (_fcsPtrArray3[a] == NULL || _fcsUnk1 == a)
		return;

	_fcsUnk1 = a;
	showmessage_print_char(0);
	_fcsPtr1 = _fcsPtrArray3[a];

	showmessage_helper_3(_fcsPtr1->textLength, _fcsPtr1->textMaxLength);
}

// OK
FillOrCopyStruct *SimonEngine::fcs_alloc(uint x, uint y, uint w, uint h, uint flags, uint fill_color,
																				uint unk4) {
	FillOrCopyStruct *fcs;

	fcs = _fcs_list;
	while (fcs->mode != 0)
		fcs++;

	fcs->mode = 2;
	fcs->x = x;
	fcs->y = y;
	fcs->width = w;
	fcs->height = h;
	fcs->flags = flags;
	fcs->fill_color = fill_color;
	fcs->text_color = unk4;
	fcs->textColumn = 0;
	fcs->textRow = 0;
	fcs->textColumnOffset = 0;
	fcs->textMaxLength = fcs->width * 8 / 6; // characters are 6 pixels
	return fcs;
}

Item *SimonEngine::derefItem(uint item) {
	if (item >= _itemArraySize)
		error("derefItem: invalid item %d", item);
	return _itemArrayPtr[item];
}

uint SimonEngine::itemPtrToID(Item *id) {
	uint i;
	for (i = 0; i != _itemArraySize; i++)
		if (_itemArrayPtr[i] == id)
			return i;
	error("itemPtrToID: not found");
	return 0;
}

void SimonEngine::o_pathfind(int x, int y, uint var_1, uint var_2) {
	const uint16 *p;
	uint i, j;
	uint prev_i;
	uint x_diff, y_diff;
	uint best_i = 0, best_j = 0, best_dist = 0xFFFFFFFF;

	if (_game & GF_SIMON2) {
		x += _scrollX * 8;
	}

	int end = (_game == GAME_FEEBLEFILES) ? 9999 : 999;
	prev_i = 21 - _variableArray[12];
	for (i = 20; i != 0; --i) {
		p = (const uint16 *)_pathFindArray[20 - i];
		if (!p)
			continue;
		for (j = 0; readUint16Wrapper(&p[0]) != end; j++, p += 2) {	// 0xE703 = byteswapped 999
			x_diff = abs((int)(readUint16Wrapper(&p[0]) - x));
			y_diff = abs((int)(readUint16Wrapper(&p[1]) - 12 - y));

			if (x_diff < y_diff) {
				x_diff >>= 2;
				y_diff <<= 2;
			}
			x_diff += y_diff >> 2;

			if (x_diff < best_dist || x_diff == best_dist && prev_i == i) {
				best_dist = x_diff;
				best_i = 21 - i;
				best_j = j;
			}
		}
	}

	_variableArray[var_1] = best_i;
	_variableArray[var_2] = best_j;
}

// ok
void SimonEngine::fcs_unk1(uint fcs_index) {
	FillOrCopyStruct *fcs;
	uint16 fcsunk1;
	uint16 i;

	fcs = _fcsPtrArray3[fcs_index & 7];
	fcsunk1 = _fcsUnk1;

	if (fcs == NULL || fcs->fcs_data == NULL)
		return;

	fcs_unk_2(fcs_index);
	fcs_putchar(12);
	fcs_unk_2(fcsunk1);

	for (i = 0; fcs->fcs_data->e[i].item != NULL; i++) {
		delete_hitarea_by_index(fcs->fcs_data->e[i].hit_area);
	}

	if (fcs->fcs_data->unk3 != -1) {
		delete_hitarea_by_index(fcs->fcs_data->unk3);
	}

	if (fcs->fcs_data->unk4 != -1) {
		delete_hitarea_by_index(fcs->fcs_data->unk4);
		if (!(_game & GF_SIMON2))
			fcs_unk_5(fcs, fcs_index);
	}

	free(fcs->fcs_data);
	fcs->fcs_data = NULL;

	_fcsData1[fcs_index] = 0;
	_fcsData2[fcs_index] = 0;
}

// ok
void SimonEngine::fcs_unk_5(FillOrCopyStruct *fcs, uint fcs_index) {
	o_kill_sprite_simon1(0x80);
}

void SimonEngine::delete_hitarea_by_index(uint index) {
	CHECK_BOUNDS(index, _hitAreas);
	_hitAreas[index].flags = 0;
}

// ok
void SimonEngine::fcs_putchar(uint a) {
	if (_fcsPtr1 != _fcsPtrArray3[0])
		video_putchar(_fcsPtr1, a);
}

// ok
void SimonEngine::video_fill_or_copy_from_3_to_2(FillOrCopyStruct *fcs) {
	if (fcs->flags & 0x10)
		copy_img_from_3_to_2(fcs);
	else
		video_erase(fcs);

	fcs->textColumn = 0;
	fcs->textRow = 0;
	fcs->textColumnOffset = 0;
	fcs->textLength = 0;
}

// ok
void SimonEngine::copy_img_from_3_to_2(FillOrCopyStruct *fcs) {
	_lockWord |= 0x8000;

	if (!(_game & GF_SIMON2)) {
		dx_copy_rgn_from_3_to_2(fcs->y + fcs->height * 8 + ((fcs == _fcsPtrArray3[2]) ? 1 : 0), (fcs->x + fcs->width) * 8, fcs->y, fcs->x * 8);
	} else {
		if (_vgaVar6 && _fcsPtrArray3[2] == fcs) {
			fcs = _fcsPtrArray3[0x18 / 4];
			_vgaVar6 = 0;
		}

		dx_copy_rgn_from_3_to_2(fcs->y + fcs->height * 8, (fcs->x + fcs->width) * 8, fcs->y, fcs->x * 8);
	}

	_lockWord &= ~0x8000;
}

void SimonEngine::video_erase(FillOrCopyStruct *fcs) {
	byte *dst;
	uint h;

	_lockWord |= 0x8000;

	dst = dx_lock_2();
	dst += _dxSurfacePitch * fcs->y + fcs->x * 8;

	h = fcs->height * 8;
	do {
		memset(dst, fcs->fill_color, fcs->width * 8);
		dst += _dxSurfacePitch;
	} while (--h);

	dx_unlock_2();
	_lockWord &= ~0x8000;
}

VgaSprite *SimonEngine::find_cur_sprite() {
	VgaSprite *vsp = _vgaSprites;
	while (vsp->id) {
		if (_game & GF_SIMON2) {
			if (vsp->id == _vgaCurSpriteId && vsp->fileId == _vgaCurFileId)
				break;
		} else {
			if (vsp->id == _vgaCurSpriteId)
				break;
		}
		vsp++;
	}
	return vsp;
}

bool SimonEngine::isSpriteLoaded(uint16 id, uint16 fileId) {
	VgaSprite *vsp = _vgaSprites;
	while (vsp->id) {
		if (_game & GF_SIMON2) {
			if (vsp->id == id && vsp->fileId == fileId)
				return true;
		} else {
			if (vsp->id == id)
				return true;
		}
		vsp++;
	}
	return false;
}

void SimonEngine::processSpecialKeys() {
	switch (_keyPressed) {
	case 27: // escape
		_exitCutscene = true;
		break;
	case 59: // F1
		if (_game & GF_SIMON2) {
			vc_write_var(5, 50);
		} else {
			vc_write_var(5, 40);
		}
			vc_write_var(86, 0);
		break;
	case 60: // F2
		if (_game & GF_SIMON2) {
			vc_write_var(5, 75);
		} else {
			vc_write_var(5, 60);
		}
			vc_write_var(86, 1);
		break;
	case 61: // F3
		if (_game & GF_SIMON2) {
			vc_write_var(5, 125);
		} else {
			vc_write_var(5, 100);
		}
			vc_write_var(86, 2);
		break;
	case 63: // F5
		if (_game & GF_SIMON2)
			_exitCutscene = true;
		break;
	case 'p':
		pause();
		break;
	case 't':
		if ((_game & GF_SIMON2 && _game & GF_TALKIE) || ( _game & GF_TALKIE && _language > 1))
			if (_speech)
				_subtitles ^= 1;
		break;
	case 'v':
		if ((_game & GF_SIMON2) && (_game & GF_TALKIE))
			if (_subtitles)
				_speech ^= 1;
	case '+':
		midi.set_volume(midi.get_volume() + 16);
		break;
	case '-':
		midi.set_volume(midi.get_volume() - 16);
		break;
	case 'm':
		midi.pause(_musicPaused ^= 1);
		break;
	case 's':
		if (_game == GAME_SIMON1DOS)
			midi._enable_sfx ^= 1;
		else
			_sound->effectsPause(_effectsPaused ^= 1);
		break;
	case 'b':
		_sound->ambientPause(_ambientPaused ^= 1);
		break;
	case 'r':
		if (_debugMode)
			_startMainScript ^= 1;
		break;
	case 'o':
		if (_debugMode)
			_continousMainScript ^= 1;
		break;
	case 'a':
		if (_debugMode)
			_startVgaScript ^= 1;
		break;
	case 'g':
		if (_debugMode)
			_continousVgaScript ^= 1;
		break;
	case 'i':
		if (_debugMode)
			_drawImagesDebug ^= 1;
		break;
	case 'd':
		if (_debugMode)
			_dumpImages ^=1;
		break;
	}

	_keyPressed = 0;
}

void SimonEngine::pause() {
	_keyPressed = 1;
	_pause = 1;
	bool ambient_status = _ambientPaused;
	bool music_status = _musicPaused;

	midi.pause(true);
	_sound->ambientPause(true);
	while (_pause) {
		delay(1);
		if (_keyPressed == 'p')
			_pause = 0;
	}
	midi.pause(music_status);
	_sound->ambientPause(ambient_status);

}

void SimonEngine::video_toggle_colors(HitArea * ha, byte a, byte b, byte c, byte d) {
	byte *src, color;
	uint w, h, i;

	_lockWord |= 0x8000;
	src = dx_lock_2() + ha->y * _dxSurfacePitch + ha->x;

	w = ha->width;
	h = ha->height;

	// Works around bug in original Simon the Sorcerer 2
	// Animations continue in background when load/save dialog is open
	// often causing the savegame name highlighter to be cut short
	if (!(h > 0 && w > 0 && ha->x + w <= _screenWidth && ha->y + h <= _screenHeight)) {
		debug(1,"Invalid coordinates in video_toggle_colors (%d,%d,%d,%d)", ha->x, ha->y, ha->width, ha->height);
		_lockWord &= ~0x8000;
		return;
	}

	do {
		for (i = 0; i != w; ++i) {
			color = src[i];
			if (a >= color && b < color) {
				if (c >= color)
					color += d;
				else
					color -= d;
				src[i] = color;
			}
		}
		src += _dxSurfacePitch;
	} while (--h);


	dx_unlock_2();
	_lockWord &= ~0x8000;
}

void SimonEngine::video_copy_if_flag_0x8_c(FillOrCopyStruct *fcs) {
	if (fcs->flags & 8)
		copy_img_from_3_to_2(fcs);
	fcs->mode = 0;
}

void SimonEngine::loadSprite(uint windowNum, uint fileId, uint vgaSpriteId, uint x, uint y, uint palette) {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *p, *pp;
	uint count;

	_lockWord |= 0x40;

	if (isSpriteLoaded(vgaSpriteId, fileId)) {
		_lockWord &= ~0x40;
		return;
	}

	vsp = _vgaSprites;
	while (vsp->id != 0)
		vsp++;

	vsp->windowNum = windowNum;
	vsp->priority = 0;
	vsp->flags = 0;

	vsp->y = y;
	vsp->x = x;
	vsp->image = 0;
	vsp->palette = palette;
	vsp->id = vgaSpriteId;
	if (_game & GF_SIMON1)
		vsp->fileId = fileId = vgaSpriteId / 100;
	else
		vsp->fileId = fileId;


	for (;;) {
		vpe = &_vgaBufferPointers[fileId];
		_vgaCurFile2 = fileId;
		_curVgaFile1 = vpe->vgaFile1;
		if (vpe->vgaFile1 != NULL)
			break;
		ensureVgaResLoaded(fileId);
	}

	pp = _curVgaFile1;
	if (_game == GAME_FEEBLEFILES) {
		p = pp + READ_LE_UINT16(&((VgaFileHeader_Feeble *) pp)->hdr2_start);
		count = READ_LE_UINT16(&((VgaFileHeader2_Feeble *) p)->animationCount);
		p = pp + READ_LE_UINT16(&((VgaFileHeader2_Feeble *) p)->animationTable);
	} else {
		p = pp + READ_BE_UINT16(&((VgaFileHeader_Simon *) pp)->hdr2_start);
		count = READ_BE_UINT16(&((VgaFileHeader2_Simon *) p)->animationCount);
		p = pp + READ_BE_UINT16(&((VgaFileHeader2_Simon *) p)->animationTable);
	}

	for (;;) {
		if (_game == GAME_FEEBLEFILES) {
			if (READ_LE_UINT16(&((AnimationHeader_Feeble *) p)->id) == vgaSpriteId) {
				if (_startVgaScript)
					dump_vga_script(pp + READ_LE_UINT16(&((AnimationHeader_Feeble*)p)->scriptOffs), fileId, vgaSpriteId);

				add_vga_timer(VGA_DELAY_BASE, pp + READ_LE_UINT16(&((AnimationHeader_Feeble *) p)->scriptOffs), vgaSpriteId, fileId);
				break;
			}
			p += sizeof(AnimationHeader_Feeble);
		} else {
			if (READ_BE_UINT16(&((AnimationHeader_Simon *) p)->id) == vgaSpriteId) {
				if (_startVgaScript)
					dump_vga_script(pp + READ_BE_UINT16(&((AnimationHeader_Simon*)p)->scriptOffs), fileId, vgaSpriteId);

				add_vga_timer(VGA_DELAY_BASE, pp + READ_BE_UINT16(&((AnimationHeader_Simon *) p)->scriptOffs), vgaSpriteId, fileId);
				break;
			}
			p += sizeof(AnimationHeader_Simon);
		}

		if (!--count) {
			vsp->id = 0;
			break;
		}
	}

	_lockWord &= ~0x40;
}

void SimonEngine::talk_with_speech(uint speech_id, uint vgaSpriteId) {
	if (!(_game & GF_SIMON2)) {
		if (speech_id == 9999) {
			if (_subtitles)
				return;
			if (!(_bitArray[0] & 0x4000) && !(_bitArray[1] & 0x1000)) {
				_bitArray[0] |= 0x4000;
				_variableArray[100] = 0xF;
				loadSprite(4, 1, 0x82, 0, 0, 0);
				o_wait_for_vga(0x82);
			}
			_skipVgaWait = true;
		} else {
			if (_subtitles && _scriptVar2) {
				loadSprite(4, 2, 204, 0, 0, 0);
				o_wait_for_vga(204);
				o_kill_sprite_simon1(204);
			}
			o_kill_sprite_simon1(vgaSpriteId + 201);
			_sound->playVoice(speech_id);
			loadSprite(4, 2, vgaSpriteId + 201, 0, 0, 0);
		}
	} else {
		if (speech_id == 0xFFFF) {
			if (_subtitles)
				return;
			if (!(_bitArray[0] & 0x4000) && !(_bitArray[1] & 0x1000)) {
				_bitArray[0] |= 0x4000;
				_variableArray[100] = 5;
				loadSprite(4, 1, 0x1e, 0, 0, 0);
				o_wait_for_vga(0x82);
			}
			_skipVgaWait = true;
		} else {
			if (_subtitles && _language != 20) {
				_sound->playVoice(speech_id);
				return;
			} else if (_subtitles && _scriptVar2) {
				loadSprite(4, 2, 5, 0, 0, 0);
				o_wait_for_vga(205);
				o_kill_sprite_simon2(2,5);
			}

			o_kill_sprite_simon2(2, vgaSpriteId + 2);
			_sound->playVoice(speech_id);
			loadSprite(4, 2, vgaSpriteId + 2, 0, 0, 0);
		}
	}
}

void SimonEngine::talk_with_text(uint vgaSpriteId, uint color, const char *string, int16 x, int16 y, int16 width) {
	char convertedString[320];
	char *convertedString2 = convertedString;
	int16 height, len_div_3;
	int stringLength = strlen(string);
	int padding, lettersPerRow, lettersPerRowJustified;
	const int textHeight = 10;

	height = textHeight;
	lettersPerRow = width / 6;
	lettersPerRowJustified = stringLength / (stringLength / lettersPerRow + 1) + 1;

	len_div_3 = (stringLength + 3) / 3;
	if (!(_game & GF_SIMON2) && (_game & GF_TALKIE)) {
		if (_variableArray[141] == 0)
			_variableArray[141] = 9;
		_variableArray[85] = _variableArray[141] * len_div_3;
	} else {
		if (_variableArray[86] == 0)
			len_div_3 >>= 1;
		if (_variableArray[86] == 2)
			len_div_3 <<= 1;
		_variableArray[85] = len_div_3 * 5;
	}

	assert(stringLength > 0);
	while (stringLength > 0) {
		int pos = 0;
		if (stringLength > lettersPerRow) {
			int removeLastWord = 0;
			if (lettersPerRow > lettersPerRowJustified) {
				pos = lettersPerRowJustified;
				while (string[pos] != ' ')
					pos++;
				if (pos > lettersPerRow)
					removeLastWord = 1;
			}
			if (lettersPerRow <= lettersPerRowJustified || removeLastWord) {
				pos = lettersPerRow;
				while (string[pos] != ' ' && pos > 0)
					pos--;
			}
			height += textHeight;
			y -= textHeight;
		} else
			pos = stringLength;
		padding = (lettersPerRow - pos) % 2 ?
			(lettersPerRow - pos) / 2 + 1 : (lettersPerRow - pos) / 2;
		while (padding--)
			*convertedString2++ = ' ';
		stringLength -= pos;
		while (pos--)
			*convertedString2++ = *string++;
		*convertedString2++ = '\n';
		string++; // skip space
		stringLength--; // skip space
	}
	*(convertedString2 - 1) = '\0';

	if (_game & GF_SIMON2)
		o_kill_sprite_simon2(2, vgaSpriteId);
	else
		o_kill_sprite_simon1(vgaSpriteId + 199);

	color = color * 3 + 192;
	if (_game & GF_AMIGA)
		render_string_amiga(vgaSpriteId, color, width, height, convertedString);
	else
		render_string(vgaSpriteId, color, width, height, convertedString);

	int b = 4;
	if (!(_bitArray[8] & 0x20))
		b = 3;

	x >>= 3;

	if (y < 2)
		y = 2;

	if (_game & GF_SIMON2)
		loadSprite(b, 2, vgaSpriteId, x, y, 12);
	else
		loadSprite(b, 2, vgaSpriteId + 199, x, y, 12);
}

// Thanks to Stuart Caie for providing the original
// C conversion upon which this decruncher is based.

#define SD_GETBIT(var) do {     \
	if (!bits--) {              \
		s -= 4;                 \
		if (s < src)            \
			return false;       \
		bb = READ_BE_UINT32(s); \
		bits = 31;              \
	}                           \
	(var) = bb & 1;             \
	bb >>= 1;                   \
}while (0)

#define SD_GETBITS(var, nbits) do { \
	bc = (nbits);                   \
	(var) = 0;                      \
	while (bc--) {                   \
		(var) <<= 1;                \
		SD_GETBIT(bit);             \
		(var) |= bit;               \
	}                               \
}while (0)

#define SD_TYPE_LITERAL (0)
#define SD_TYPE_MATCH   (1)

static bool decrunch_file_amiga (byte *src, byte *dst, uint32 size) {
	byte *s = src + size - 4;
	uint32 destlen = READ_BE_UINT32 (s);
	uint32 bb, x, y;
	byte *d = dst + destlen;
	byte bc, bit, bits, type;

	// Initialize bit buffer.
	s -= 4;
	bb = x = READ_BE_UINT32 (s);
	bits = 0;
	do {
		x >>= 1;
		bits++;
	} while (x);
	bits--;

	while (d > dst) {
		SD_GETBIT(x);
		if (x) {
			SD_GETBITS(x, 2);
			switch (x) {
			case 0:
				type = SD_TYPE_MATCH;
				x = 9;
				y = 2;
				break;

			case 1:
				type = SD_TYPE_MATCH;
				x = 10;
				y = 3;
				break;

			case 2:
				type = SD_TYPE_MATCH;
				x = 12;
				SD_GETBITS(y, 8);
				break;

			default:
				type = SD_TYPE_LITERAL;
				x = 8;
				y = 8;
			}
		} else {
			SD_GETBIT(x);
			if (x) {
				type = SD_TYPE_MATCH;
				x = 8;
				y = 1;
			} else {
				type = SD_TYPE_LITERAL;
				x = 3;
				y = 0;
			}
		}

		if (type == SD_TYPE_LITERAL) {
			SD_GETBITS(x, x);
			y += x;
			if ((int)(y + 1) > (d - dst))
				return false; // Overflow?
			do {
				SD_GETBITS(x, 8);
				*--d = x;
			} while (y-- > 0);
		} else {
			if ((int)(y + 1) > (d - dst))
				return false; // Overflow?
			SD_GETBITS(x, x);
			if ((d + x) > (dst + destlen))
				return false; // Offset overflow?
			do {
				d--;
				*d = d[x];
			} while (y-- > 0);
		}
	}

	// Successful decrunch.
	return true;
}

#undef SD_GETBIT
#undef SD_GETBITS
#undef SD_TYPE_LITERAL
#undef SD_TYPE_MATCH

void SimonEngine::read_vga_from_datfile_1(uint vga_id) {
	if (_game & GF_OLD_BUNDLE) {
		File in;
		char buf[15];
		uint32 size;
		if (vga_id == 23)
			vga_id = 112;
		if (vga_id == 328)
			vga_id = 119;

		if (_game == GAME_SIMON1CD32) {
			sprintf(buf, "0%d.out", vga_id);
		} else if (_game == GAME_SIMON1AMIGA) {
			sprintf(buf, "0%d.pkd", vga_id);
		} else {
			sprintf(buf, "0%d.VGA", vga_id);
		}

		in.open(buf);
		if (in.isOpen() == false)
			error("read_vga_from_datfile_1: can't open %s", buf);
		size = in.size();

		if (_game == GAME_SIMON1AMIGA) {
			byte *buffer = new byte[size];
			if (in.read(buffer, size) != size)
				error("read_vga_from_datfile_1: read failed");
			decrunch_file_amiga (buffer, _vgaBufferPointers[11].vgaFile2, size);
			delete [] buffer;
		} else {
			if (in.read(_vgaBufferPointers[11].vgaFile2, size) != size)
				error("read_vga_from_datfile_1: read failed");
		}
		in.close();
	} else {
		uint32 offs_a = _gameOffsetsPtr[vga_id];
		uint32 size = _gameOffsetsPtr[vga_id + 1] - offs_a;

		resfile_read(_vgaBufferPointers[11].vgaFile2, offs_a, size);
	}
}

byte *SimonEngine::read_vga_from_datfile_2(uint id) {
	// !!! HACK !!!
	// allocate more space for text to cope with foreign languages that use
	// up more space than english. I hope 6400 bytes are enough. This number
	// is base on: 2 (lines) * 320 (screen width) * 10 (textheight) -- olki
	int extraBuffer = (id == 5 ? 6400 : 0);

	if (_game & GF_OLD_BUNDLE) {
		File in;
		char buf[15];
		uint32 size;
		byte *dst;

		if (_game == GAME_SIMON1CD32) {
			sprintf(buf, "%.3d%d.out", id >> 1, (id & 1) + 1);
		} else if (_game == GAME_SIMON1AMIGA) {
			sprintf(buf, "%.3d%d.pkd", id >> 1, (id & 1) + 1);
		} else {
			sprintf(buf, "%.3d%d.VGA", id >> 1, (id & 1) + 1);
		}

		in.open(buf);
		if (in.isOpen() == false)
			error("read_vga_from_datfile_2: can't open %s", buf);
		size = in.size();

		if (_game == GAME_SIMON1AMIGA) {
			byte *buffer = new byte[size];
			if (in.read(buffer, size) != size)
				error("read_vga_from_datfile_2: read failed");
			dst = setup_vga_destination (READ_BE_UINT32(buffer + size - 4) + extraBuffer);
			decrunch_file_amiga (buffer, dst, size);
			delete[] buffer;
		} else {
			dst = setup_vga_destination(size + extraBuffer);
			if (in.read(dst, size) != size)
				error("read_vga_from_datfile_2: read failed");
		}
		in.close();

		return dst;
	} else {
		uint32 offs_a = _gameOffsetsPtr[id];
		uint32 size = _gameOffsetsPtr[id + 1] - offs_a;
		byte *dst;

		dst = setup_vga_destination(size + extraBuffer);
		resfile_read(dst, offs_a, size);

		return dst;
	}
}

void SimonEngine::resfile_read(void *dst, uint32 offs, uint32 size) {
	_gameFile->seek(offs, SEEK_SET);
	if (_gameFile->read(dst, size) != size)
		error("resfile_read(%d,%d) read failed", offs, size);
}

void SimonEngine::openGameFile() {
	if (!(_game & GF_OLD_BUNDLE)) {
		_gameFile = new File();
		_gameFile->open(gss->gme_filename);

		if (_gameFile->isOpen() == false)
			error("Can't open game file '%s'", gss->gme_filename);

		uint32 size = _gameFile->readUint32LE();

		_gameOffsetsPtr = (uint32 *)malloc(size);
		if (_gameOffsetsPtr == NULL)
			error("out of memory, game offsets");

		resfile_read(_gameOffsetsPtr, 0, size);
#if defined(SCUMM_BIG_ENDIAN)
		for (uint r = 0; r < size / sizeof(uint32); r++)
			_gameOffsetsPtr[r] = FROM_LE_32(_gameOffsetsPtr[r]);
#endif
	}

	if (_game != GAME_FEEBLEFILES)
		loadIconFile();

	vc34_setMouseOff();

	runSubroutine101();
	startUp_helper_2();
}

void SimonEngine::runSubroutine101() {
	Subroutine *sub;

	sub = getSubroutineByID(101);
	if (sub != NULL)
		startSubroutineEx(sub);

	startUp_helper_2();
}

void SimonEngine::dx_copy_rgn_from_3_to_2(uint b, uint r, uint y, uint x) {
	byte *dst, *src;
	uint i;

	dst = dx_lock_2();
	src = _sdl_buf_3;

	dst += y * _dxSurfacePitch;
	src += y * _dxSurfacePitch;

	while (y < b) {
		for (i = x; i < r; i++)
			dst[i] = src[i];
		y++;
		dst += _dxSurfacePitch;
		src += _dxSurfacePitch;
	}

	dx_unlock_2();
}

void SimonEngine::dx_clear_surfaces(uint num_lines) {
	memset(_sdl_buf_attached, 0, num_lines * _screenWidth);

	_system->copyRectToScreen(_sdl_buf_attached, _screenWidth, 0, 0, _screenWidth, _screenHeight);

	if (_dxUse3Or4ForLock) {
		memset(_sdl_buf, 0, num_lines * _screenWidth);
		memset(_sdl_buf_3, 0, num_lines * _screenWidth);
	}
}

void SimonEngine::dx_clear_attached_from_top(uint lines) {
	memset(_sdl_buf_attached, 0, lines * _screenWidth);
}

void SimonEngine::dx_copy_from_attached_to_2(uint x, uint y, uint w, uint h) {
	uint offs = x + y * _screenWidth;
	byte *s = _sdl_buf_attached + offs;
	byte *d = _sdl_buf + offs;

	do {
		memcpy(d, s, w);
		d += _screenWidth;
		s += _screenWidth;
	} while (--h);
}

void SimonEngine::dx_copy_from_2_to_attached(uint x, uint y, uint w, uint h) {
	uint offs = x + y * _screenWidth;
	byte *s = _sdl_buf + offs;
	byte *d = _sdl_buf_attached + offs;

	do {
		memcpy(d, s, w);
		d += _screenWidth;
		s += _screenWidth;
	} while (--h);
}

void SimonEngine::dx_copy_from_attached_to_3(uint lines) {
	memcpy(_sdl_buf_3, _sdl_buf_attached, lines * _screenWidth);
}

void SimonEngine::dx_update_screen_and_palette() {
	_numScreenUpdates++;

	if (_paletteColorCount == 0 && _videoVar9 == 1) {
		_videoVar9 = 0;
		if (memcmp(_palette, _paletteBackup, 256 * 4) != 0) {
			memcpy(_paletteBackup, _palette, 256 * 4);
			_system->setPalette(_palette, 0, 256);
		}
	}

	_system->copyRectToScreen(_sdl_buf_attached, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	_system->updateScreen();

	memcpy(_sdl_buf_attached, _sdl_buf, _screenWidth * _screenHeight);

	if (_paletteColorCount != 0) {
		if (!(_game & GF_SIMON2) && _usePaletteDelay) {
			delay(100);
			_usePaletteDelay = false;
		}
		realizePalette();
	}
}

void SimonEngine::realizePalette() {
	if (_paletteColorCount & 0x8000) {
		fadeUpPalette();
	} else {
		_videoVar9 = false;
		memcpy(_paletteBackup, _palette, 256 * 4);
		_system->setPalette(_palette, 0, _paletteColorCount);
		_paletteColorCount = 0;
	}
}

void SimonEngine::fadeUpPalette() {
	uint8 paletteTmp[768];
	uint8 *src, *dst;
	int c, p;

	_paletteColorCount &= 0x7fff;
	_videoVar9 = false;

	memcpy(_videoBuf1, _palette, 1024); // Difference
	memset(_videoBuf1, 0, 768);

	memcpy(_paletteBackup, _palette, 768);
	memcpy(paletteTmp, _palette, 768);

	for (c = 255; c > 0; c -= 4) {
	  	src = paletteTmp;
 		dst = _videoBuf1;

		for (p = _paletteColorCount; p !=0 ; p--) {
			if (*src >= c)
				*dst = *dst + 4;
			
			src++;
			dst++;
 		}
 		_system->setPalette(_videoBuf1, 0, _videoNumPalColors);
		if (_fade)
			_system->updateScreen();
 		delay(5);
 	}
	_paletteColorCount = 0;
}

int SimonEngine::go() {
	if (!_dumpFile)
		_dumpFile = stdout;

	// allocate buffers
	_sdl_buf_3 = (byte *)calloc(_screenWidth * _screenHeight, 1);
	_sdl_buf = (byte *)calloc(_screenWidth * _screenHeight, 1);
	_sdl_buf_attached = (byte *)calloc(_screenWidth * _screenHeight, 1);

	allocItemHeap();
	allocTablesHeap();

	setup_vga_file_buf_pointers();

	_sound = new Sound(_game, gss, _mixer);
	_debugger = new Debugger(this);

	if (ConfMan.hasKey("sfx_mute") && ConfMan.getBool("sfx_mute") == 1) {
		if (_game == GAME_SIMON1DOS)
			midi._enable_sfx ^= 1;
		else
			_sound->effectsPause(_effectsPaused ^= 1);
	}

	loadGamePcFile(gss->gamepc_filename);

	addTimeEvent(0, 1);
	openGameFile();

	_lastMusicPlayed = -1;
	_frameRate = 1;

	_startMainScript = false;
	_continousMainScript = false;
	_startVgaScript = false;
	_continousVgaScript = false;
	_drawImagesDebug = false;

	if (gDebugLevel == 2)
		_continousMainScript = true;
	if (gDebugLevel == 3)
		_continousVgaScript = true;
	if (gDebugLevel == 4)
		_startMainScript = true;
	if (gDebugLevel == 5)
		_startVgaScript = true;

	if (_game & GF_TALKIE) {
		// English and German versions of Simon the Sorcerer 1 don't have full subtitles
		if (!(_game & GF_SIMON2) && _language < 2)
			_subtitles = false;
	} else {
		_subtitles = true;
	}

	while (1) {
		hitarea_stuff();
		handle_verb_clicked(_verbHitArea);
		delay(100);
	}

	return 0;
}

void SimonEngine::shutdown() {
	delete _gameFile;

	midi.close();

	free(_stringTabPtr);
	free(_itemArrayPtr);
	free(_itemHeapPtr - _itemHeapCurPos);
	free(_tablesHeapPtr - _tablesHeapCurPos);
	free(_tblList);
	free(_iconFilePtr);
	free(_gameOffsetsPtr);

	_system->quit();
}

void SimonEngine::delay(uint amount) {
	OSystem::Event event;

	uint32 start = _system->getMillis();
	uint32 cur = start;
	uint this_delay, vga_period;

	if (_debugger->isAttached())
		_debugger->onFrame();

	if (_fastMode)
	 	vga_period = 10;
	else if (_game & GF_SIMON2)
		vga_period = 45 * _speed;
	else
		vga_period = 50 * _speed;

	_rnd.getRandomNumber(2);

	do {
		while (!_inCallBack && cur >= _lastVgaTick + vga_period && !_pause) {
			_lastVgaTick += vga_period;

			// don't get too many frames behind
			if (cur >= _lastVgaTick + vga_period * 2)
				_lastVgaTick = cur;

			_inCallBack = true;
			timer_callback();
			_inCallBack = false;
		}

		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.keycode >= '0' && event.kbd.keycode <='9'
					&& (event.kbd.flags == OSystem::KBD_ALT ||
						event.kbd.flags == OSystem::KBD_CTRL)) {
					_saveLoadSlot = event.kbd.keycode - '0';

					// There is no save slot 0
					if (_saveLoadSlot == 0)
						_saveLoadSlot = 10;

					sprintf(_saveLoadName, "Quicksave %d", _saveLoadSlot);
					_saveLoadType = (event.kbd.flags == OSystem::KBD_ALT) ? 1 : 2;

					// We should only allow a load or save when it was possible in original
					// This stops load/save during copy protection, conversations and cut scenes
					if (!_lockCounter && !_showPreposition)
						quick_load_or_save();
				} else if (event.kbd.flags == OSystem::KBD_CTRL) {
					if (event.kbd.keycode == 'a') {
						GUI::Dialog *_aboutDialog;
						_aboutDialog = new GUI::AboutDialog();
						_aboutDialog->runModal();
					} else if (event.kbd.keycode == 'f')
						_fastMode ^= 1;
					else if (event.kbd.keycode == 'd')
						_debugger->attach();
				}
				// Make sure backspace works right (this fixes a small issue on OS X)
				if (event.kbd.keycode == 8)
					_keyPressed = 8;
				else
					_keyPressed = (byte)event.kbd.ascii;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				_sdlMouseX = event.mouse.x;
				_sdlMouseY = event.mouse.y;
				break;
			case OSystem::EVENT_LBUTTONDOWN:
				_leftButtonDown++;
#if defined (_WIN32_WCE) || defined(__PALM_OS__)
				_sdlMouseX = event.mouse.x;
				_sdlMouseY = event.mouse.y;
#endif
				break;
			case OSystem::EVENT_RBUTTONDOWN:
				if (_game & GF_SIMON2)
					_skipSpeech = true;
				else
					_exitCutscene = true;
				break;
			case OSystem::EVENT_QUIT:
				shutdown();
				return;
				break;
			default:
				break;
			}
		}

		if (amount == 0)
			break;

		{
			this_delay = _fastMode ? 1 : 20 * _speed;
			if (this_delay > amount)
				this_delay = amount;
			_system->delayMillis(this_delay);
		}
		cur = _system->getMillis();
	} while (cur < start + amount);
}

void SimonEngine::loadMusic (uint music) {
	char buf[4];

	if (_game & GF_AMIGA) {
		if (_game != GAME_SIMON1CD32) {
			// TODO Add support for decruncher
			debug(5,"loadMusic - Decrunch %dtune attempt", music);
		}
		// TODO Add Protracker support for simon1amiga/cd32
		debug(5,"playMusic - Load %dtune attempt", music);
	} else if (_game & GF_SIMON2) {        // Simon 2 music
		midi.stop();
		_gameFile->seek(_gameOffsetsPtr[MUSIC_INDEX_BASE + music - 1], SEEK_SET);
		_gameFile->read(buf, 4);
		if (!memcmp(buf, "FORM", 4)) {
			_gameFile->seek(_gameOffsetsPtr[MUSIC_INDEX_BASE + music - 1], SEEK_SET);
			midi.loadXMIDI (_gameFile);
		} else {
			_gameFile->seek(_gameOffsetsPtr[MUSIC_INDEX_BASE + music - 1], SEEK_SET);
			midi.loadMultipleSMF (_gameFile);
		}

		_lastMusicPlayed = music;
		_nextMusicToPlay = -1;
	} else if (_game & GF_SIMON1) {        // Simon 1 music
		midi.stop();
		midi.setLoop (true); // Must do this BEFORE loading music. (GMF may have its own override.)

		if (_game & GF_TALKIE) {
			// FIXME: The very last music resource, a cymbal crash for when the
			// two demons crash into each other, should NOT be looped like the
			// other music tracks. In simon1dos/talkie the GMF resource includes
			// a loop override that acomplishes this, but there seems to be nothing
			// for this in the SMF resources.
			if (music == 35)
				midi.setLoop (false);

			_gameFile->seek(_gameOffsetsPtr[MUSIC_INDEX_BASE + music], SEEK_SET);
			_gameFile->read(buf, 4);
			if (!memcmp(buf, "GMF\x1", 4)) {
				_gameFile->seek(_gameOffsetsPtr[MUSIC_INDEX_BASE + music], SEEK_SET);
				midi.loadSMF (_gameFile, music);
			} else {
				_gameFile->seek(_gameOffsetsPtr[MUSIC_INDEX_BASE + music], SEEK_SET);
				midi.loadMultipleSMF (_gameFile);
			}

		} else {
			char filename[15];
			File f;
			sprintf(filename, "MOD%d.MUS", music);
			f.open(filename);
			if (f.isOpen() == false) {
				warning("Can't load music from '%s'", filename);
				return;
			}
			if (_game & GF_DEMO)
				midi.loadS1D (&f);
			else
				midi.loadSMF (&f, music);
		}

		midi.startTrack (0);
	}
}

byte *SimonEngine::dx_lock_2() {
	_dxSurfacePitch = _screenWidth;
	return _sdl_buf;
}

void SimonEngine::dx_unlock_2() {
}

byte *SimonEngine::dx_lock_attached() {
	_dxSurfacePitch = _screenWidth;
	return _dxUse3Or4ForLock ? _sdl_buf_3 : _sdl_buf_attached;
}

void SimonEngine::dx_unlock_attached() {
}

void SimonEngine::set_volume(int volume) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume);
}

byte SimonEngine::getByte() {
	return *_codePtr++;
}

} // End of namespace Simon

#ifdef __PALM_OS__
#include "scumm_globals.h"

_GINIT(Simon_Simon)
_GSETPTR(Simon::simon1_settings, GBVARS_SIMON1SETTINGS_INDEX, Simon::GameSpecificSettings, GBVARS_SIMON)
_GSETPTR(Simon::simon1acorn_settings, GBVARS_SIMON1ACORNSETTINGS_INDEX, Simon::GameSpecificSettings, GBVARS_SIMON)
_GSETPTR(Simon::simon1amiga_settings, GBVARS_SIMON1AMIGASETTINGS_INDEX, Simon::GameSpecificSettings, GBVARS_SIMON)
_GSETPTR(Simon::simon1demo_settings, GBVARS_SIMON1DEMOSETTINGS_INDEX, Simon::GameSpecificSettings, GBVARS_SIMON)
_GSETPTR(Simon::simon2win_settings, GBVARS_SIMON2WINSETTINGS_INDEX, Simon::GameSpecificSettings, GBVARS_SIMON)
_GSETPTR(Simon::simon2dos_settings, GBVARS_SIMON2DOSSETTINGS_INDEX, Simon::GameSpecificSettings, GBVARS_SIMON)
_GEND

_GRELEASE(Simon_Simon)
_GRELEASEPTR(GBVARS_SIMON1SETTINGS_INDEX, GBVARS_SIMON)
_GRELEASEPTR(GBVARS_SIMON1ACORNSETTINGS_INDEX, GBVARS_SIMON)
_GRELEASEPTR(GBVARS_SIMON1AMIGASETTINGS_INDEX, GBVARS_SIMON)
_GRELEASEPTR(GBVARS_SIMON1DEMOSETTINGS_INDEX, GBVARS_SIMON)
_GRELEASEPTR(GBVARS_SIMON2WINSETTINGS_INDEX, GBVARS_SIMON)
_GRELEASEPTR(GBVARS_SIMON2DOSSETTINGS_INDEX, GBVARS_SIMON)
_GEND

#endif
