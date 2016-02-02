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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/md5.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/random.h"
#include "common/textconsole.h"

#include "engines/util.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/cursorman.h"

#include "audio/mididrv.h"
#include "audio/mixer.h"

#include "agi/agi.h"
#include "agi/font.h"
#include "agi/graphics.h"
#include "agi/inv.h"
#include "agi/sprite.h"
#include "agi/text.h"
#include "agi/keyboard.h"
#include "agi/menu.h"
#include "agi/systemui.h"
#include "agi/words.h"

#include "gui/predictivedialog.h"

namespace Agi {

void AgiEngine::allowSynthetic(bool allow) {
	_allowSynthetic = allow;
}

void AgiEngine::pollTimer() {
	_lastTick += 50;

	while (_system->getMillis() < _lastTick) {
		processEvents();
		_console->onFrame();
		_system->delayMillis(10);
		_system->updateScreen();
	}

	_lastTick = _system->getMillis();
}

void AgiEngine::pause(uint32 msec) {
	uint32 endTime = _system->getMillis() + msec;

	_gfx->setMouseCursor(true); // Busy mouse cursor

	while (_system->getMillis() < endTime) {
		processEvents();
		_system->updateScreen();
		_system->delayMillis(10);
	}
	_gfx->setMouseCursor(); // regular mouse cursor
}

int AgiEngine::agiInit() {
	int ec, i;

	debug(2, "initializing");
	debug(2, "game version = 0x%x", getVersion());

	// initialize with adj.ego.move.to.x.y(0, 0) so to speak
	_game.adjMouseX = _game.adjMouseY = 0;

	// reset all flags to false and all variables to 0
	memset(_game.flags, 0, sizeof(_game.flags));
	memset(_game.vars, 0, sizeof(_game.vars));

	// clear all resources and events
	for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++) {
		memset(&_game.views[i], 0, sizeof(struct AgiView));
		memset(&_game.pictures[i], 0, sizeof(struct AgiPicture));
		memset(&_game.logics[i], 0, sizeof(struct AgiLogic));
		memset(&_game.sounds[i], 0, sizeof(class AgiSound *)); // _game.sounds contains pointers now
		memset(&_game.dirView[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirPic[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirLogic[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirSound[i], 0, sizeof(struct AgiDir));
	}

	// clear view table
	for (i = 0; i < SCREENOBJECTS_MAX; i++)
		memset(&_game.screenObjTable[i], 0, sizeof(struct ScreenObjEntry));

	memset(&_game.addToPicView, 0, sizeof(struct ScreenObjEntry));

	_words->clearEgoWords();

	if (!_menu)
		_menu = new GfxMenu(this, _gfx, _picture, _text);

	_gfx->initPriorityTable();

	// Clear the string buffer on startup, but not when the game restarts, as
	// some scripts expect that the game strings remain unaffected after a
	// restart. An example is script 98 in SQ2, which is not invoked on restart
	// to ask Ego's name again. The name is supposed to be maintained in string 1.
	// Fixes bug #3292784.
	if (!_restartGame) {
		for (i = 0; i < MAX_STRINGS; i++)
			_game.strings[i][0] = 0;
	}

	// setup emulation

	switch (getVersion() >> 12) {
	case 2:
		debug("Emulating Sierra AGI v%x.%03x",
		      (int)(getVersion() >> 12) & 0xF,
		      (int)(getVersion()) & 0xFFF);
		break;
	case 3:
		debug("Emulating Sierra AGI v%x.002.%03x",
		      (int)(getVersion() >> 12) & 0xF,
		      (int)(getVersion()) & 0xFFF);
		break;
	}

	if (getPlatform() == Common::kPlatformAmiga)
		_game.gameFlags |= ID_AMIGA;

	if (getFeatures() & GF_AGDS)
		_game.gameFlags |= ID_AGDS;

	if (_game.gameFlags & ID_AMIGA)
		debug(1, "Amiga padded game detected.");

	if (_game.gameFlags & ID_AGDS)
		debug(1, "AGDS mode enabled.");

	ec = _loader->init();   // load vol files, etc

	if (ec == errOK)
		ec = _loader->loadObjects(OBJECTS);

	// note: demogs has no words.tok
	if (ec == errOK)
		ec = _loader->loadWords(WORDS);

	// Load logic 0 into memory
	if (ec == errOK)
		ec = _loader->loadResource(RESOURCETYPE_LOGIC, 0);

#ifdef __DS__
	// Normally, the engine loads the predictive text dictionary when the predictive dialog
	// is shown.  On the DS version, the word completion feature needs the dictionary too.

	// FIXME - loadDict() no long exists in AGI as this has been moved to within the
	// GUI Predictive Dialog, but DS Word Completion is probably broken due to this...
#endif

	_egoHoldKey = false;

	_game.mouseFence.setWidth(0); // Reset

	// Reset in-game timer
	inGameTimerReset();

	// Sync volume settings from ScummVM system settings
	setVolumeViaSystemSetting();

	return ec;
}

/*
 * Public functions
 */

void AgiEngine::agiUnloadResources() {
	int i;

	// Make sure logic 0 is always loaded
	for (i = 1; i < MAX_DIRECTORY_ENTRIES; i++) {
		_loader->unloadResource(RESOURCETYPE_LOGIC, i);
	}
	for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++) {
		_loader->unloadResource(RESOURCETYPE_VIEW, i);
		_loader->unloadResource(RESOURCETYPE_PICTURE, i);
		_loader->unloadResource(RESOURCETYPE_SOUND, i);
	}
}

int AgiEngine::agiDeinit() {
	int ec;

	_words->clearEgoWords(); // remove all words from memory
	agiUnloadResources();    // unload resources in memory
	_loader->unloadResource(RESOURCETYPE_LOGIC, 0);
	ec = _loader->deinit();
	unloadObjects();
	_words->unloadDictionary();

	clearImageStack();

	return ec;
}

int AgiEngine::agiLoadResource(int16 resourceType, int16 resourceNr) {
	int i;

	i = _loader->loadResource(resourceType, resourceNr);

	// WORKAROUND: Patches broken picture 147 in a corrupted Amiga version of Gold Rush! (v2.05 1989-03-09).
	// The picture can be seen in room 147 after dropping through the outhouse's hole in room 146.
	if (i == errOK && getGameID() == GID_GOLDRUSH && resourceType == RESOURCETYPE_PICTURE && resourceNr == 147 && _game.dirPic[resourceNr].len == 1982) {
		uint8 *pic = _game.pictures[resourceNr].rdata;
		Common::MemoryReadStream picStream(pic, _game.dirPic[resourceNr].len);
		Common::String md5str = Common::computeStreamMD5AsString(picStream, _game.dirPic[resourceNr].len);
		if (md5str == "1c685eb048656cedcee4eb6eca2cecea") {
			pic[0x042] = 0x4B; // 0x49 -> 0x4B
			pic[0x043] = 0x66; // 0x26 -> 0x66
			pic[0x204] = 0x68; // 0x28 -> 0x68
			pic[0x6C0] = 0x2D; // 0x25 -> 0x2D
			pic[0x6F0] = 0xF0; // 0x70 -> 0xF0
			pic[0x734] = 0x6F; // 0x2F -> 0x6F
		}
	}

	return i;
}

int AgiEngine::agiUnloadResource(int16 resourceType, int16 resourceNr) {
	return _loader->unloadResource(resourceType, resourceNr);
}

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

AgiBase::AgiBase(OSystem *syst, const AGIGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_noSaveLoadAllowed = false;

	_rnd = new Common::RandomSource("agi");
	_sound = 0;

	initFeatures();
	initVersion();
}

AgiBase::~AgiBase() {
	delete _rnd;

	delete _sound;
}

void AgiBase::initRenderMode() {
	Common::Platform platform = Common::parsePlatform(ConfMan.get("platform"));
	Common::RenderMode configRenderMode = Common::parseRenderMode(ConfMan.get("render_mode").c_str());

	// Default to EGA PC rendering
	_renderMode = Common::kRenderEGA;

	switch (platform) {
	case Common::kPlatformDOS:
		switch (configRenderMode) {
		case Common::kRenderCGA:
			_renderMode = Common::kRenderCGA;
			break;
		// Hercules is not supported atm
		//case Common::kRenderHercA:
		//case Common::kRenderHercG:
		//	_renderMode = Common::kRenderHercG;
		//	break;
		default:
			break;
		}
		break;
	case Common::kPlatformAmiga:
		_renderMode = Common::kRenderAmiga;
		break;
	case Common::kPlatformApple2GS:
		_renderMode = Common::kRenderApple2GS;
		break;
	case Common::kPlatformAtariST:
		_renderMode = Common::kRenderAtariST;
		break;
	default:
		break;
	}

	// If render mode is explicitly set, force rendermode
	switch (configRenderMode) {
	case Common::kRenderAmiga:
		_renderMode = Common::kRenderAmiga;
		break;
	case Common::kRenderApple2GS:
		_renderMode = Common::kRenderApple2GS;
		break;
	case Common::kRenderAtariST:
		_renderMode = Common::kRenderAtariST;
		break;
	default:
		break;
	}

	if (getFeatures() & (GF_AGI256 | GF_AGI256_2)) {
		// If current game is AGI256, switch (force) to VGA render mode
		_renderMode = Common::kRenderVGA;
	}
}

const byte *AgiBase::getFontData() {
	return _font->getFontData();
}

AgiEngine::AgiEngine(OSystem *syst, const AGIGameDescription *gameDesc) : AgiBase(syst, gameDesc) {

	// Setup mixer
	syncSoundSettings();

	DebugMan.addDebugChannel(kDebugLevelMain, "Main", "Generic debug level");
	DebugMan.addDebugChannel(kDebugLevelResources, "Resources", "Resources debugging");
	DebugMan.addDebugChannel(kDebugLevelSprites, "Sprites", "Sprites debugging");
	DebugMan.addDebugChannel(kDebugLevelInventory, "Inventory", "Inventory debugging");
	DebugMan.addDebugChannel(kDebugLevelInput, "Input", "Input events debugging");
	DebugMan.addDebugChannel(kDebugLevelMenu, "Menu", "Menu debugging");
	DebugMan.addDebugChannel(kDebugLevelScripts, "Scripts", "Scripts debugging");
	DebugMan.addDebugChannel(kDebugLevelSound, "Sound", "Sound debugging");
	DebugMan.addDebugChannel(kDebugLevelText, "Text", "Text output debugging");
	DebugMan.addDebugChannel(kDebugLevelSavegame, "Savegame", "Saving & restoring game debugging");


	memset(&_game, 0, sizeof(struct AgiGame));
	memset(&_debug, 0, sizeof(struct AgiDebug));
	memset(&_mouse, 0, sizeof(struct Mouse));

	_game.mouseEnabled = true;
	_game.mouseHidden = false;
	// don't check for Amiga, Amiga doesn't allow disabling mouse support. It's mandatory.
	if (!ConfMan.getBool("mousesupport")) {
		// we effectively disable the mouse for games, that explicitly do not want mouse support to be enabled
		_game.mouseEnabled = false;
		_game.mouseHidden = true;
	}

	_game._vm = this;

	_game.gfxMode = true;

	_game.state = STATE_INIT;

	_keyQueueStart = 0;
	_keyQueueEnd = 0;

	_allowSynthetic = false;

	_intobj = NULL;

	memset(&_stringdata, 0, sizeof(struct StringData));

	_objects = NULL;

	_restartGame = false;

	_oldMode = INPUTMODE_NONE;

	_firstSlot = 0;

	resetControllers();

	setupOpcodes();
	_game._curLogic = NULL;

	_lastSaveTime = 0;
	_lastTick = 0;

	memset(_keyQueue, 0, sizeof(_keyQueue));

	_text = NULL;
	_sprites = NULL;
	_picture = NULL;
	_loader = NULL;
	_console = NULL;
	_menu = NULL;
	_gfx = NULL;
	_systemUI = NULL;

	_egoHoldKey = false;
}

void AgiEngine::initialize() {
	// TODO: Some sound emulation modes do not fit our current music
	//       drivers, and I'm not sure what they are. For now, they might
	//       as well be called "PC Speaker" and "Not PC Speaker".

	// If used platform is Apple IIGS then we must use Apple IIGS sound emulation
	// because Apple IIGS AGI games use only Apple IIGS specific sound resources.
	if (getPlatform() == Common::kPlatformApple2GS) {
		_soundemu = SOUND_EMU_APPLE2GS;
	} else if (getPlatform() == Common::kPlatformCoCo3) {
		_soundemu = SOUND_EMU_COCO3;
	} else if (ConfMan.get("music_driver") == "auto") {
		// Default sound is the proper PCJr emulation
		_soundemu = SOUND_EMU_PCJR;
	} else {
		switch (MidiDriver::getMusicType(MidiDriver::detectDevice(MDT_PCSPK | MDT_AMIGA | MDT_ADLIB | MDT_PCJR | MDT_MIDI))) {
		case MT_PCSPK:
			_soundemu = SOUND_EMU_PC;
			break;
		case MT_ADLIB:
			_soundemu = SOUND_EMU_NONE;
			break;
		case MT_PCJR:
			_soundemu = SOUND_EMU_PCJR;
			break;
		case MT_AMIGA:
			_soundemu = SOUND_EMU_AMIGA;
			break;
		default:
			debug(0, "DEF");
			_soundemu = SOUND_EMU_MIDI;
			break;
		}
	}

	initRenderMode();

	_console = new Console(this);
	_words = new Words(this);
	_font = new GfxFont(this);
	_gfx = new GfxMgr(this);
	_sound = new SoundMgr(this, _mixer);
	_picture = new PictureMgr(this, _gfx);
	_sprites = new SpritesMgr(this, _gfx);
	_text = new TextMgr(this, _words, _gfx);
	_systemUI = new SystemUI(this, _gfx, _text);
	_inventory = new InventoryMgr(this, _gfx, _text, _systemUI);

	_font->init();
	_text->init(_systemUI);

	_game.gameFlags = 0;

	_text->charAttrib_Set(15, 0);

	_game.name[0] = '\0';

	_gfx->initVideo();

	_lastSaveTime = 0;

	_lastTick = _system->getMillis();

	debugC(2, kDebugLevelMain, "Detect game");

	if (agiDetectGame() == errOK) {
		_game.state = STATE_LOADED;
		debugC(2, kDebugLevelMain, "game loaded");
	} else {
		warning("Could not open AGI game");
	}

	debugC(2, kDebugLevelMain, "Init sound");
}

bool AgiEngine::promptIsEnabled() {
	return _text->promptIsEnabled();
}

void AgiEngine::redrawScreen() {
	_game.gfxMode = true; // enable graphics mode
	_gfx->setPalette(true); // set graphics mode palette
	_text->charAttrib_Set(_text->_textAttrib.foreground, _text->_textAttrib.background);
	_gfx->clearDisplay(0);
	_picture->showPic();
	_text->statusDraw();
	_text->promptRedraw();
}

// Adjust a given coordinate to the local game screen
// Used on mouse cursor coordinates before passing them to scripts
void AgiEngine::adjustPosToGameScreen(int16 &x, int16 &y) {
	x = x / 2; // 320 -> 160
	y = y - _gfx->getRenderStartOffsetY(); // remove status bar line
	if (y < 0) {
		y = 0;
	}
	if (y >= SCRIPT_HEIGHT) {
		y = SCRIPT_HEIGHT + 1; // 1 beyond
	}
}

AgiEngine::~AgiEngine() {
	// If the engine hasn't been initialized yet via
	// AgiEngine::initialize(), don't attempt to free any resources, as
	// they haven't been allocated. Fixes bug #1742432 - AGI: Engine
	// crashes if no game is detected
	if (_game.state == STATE_INIT) {
		return;
	}

	agiDeinit();
	delete _loader;
	_gfx->deinitVideo();
	delete _inventory;
	delete _systemUI;
	delete _text;
	delete _sprites;
	delete _picture;
	delete _gfx;
	delete _font;
	delete _words;
	delete _console;
}

Common::Error AgiBase::init() {
	initialize();

	_gfx->setPalette(true);

	return Common::kNoError;
}

Common::Error AgiEngine::go() {
	if (_game.mouseEnabled) {
		CursorMan.showMouse(true);
	}
	inGameTimerReset();

	if (_game.state < STATE_LOADED) {
		do {
			mainCycle();
		} while (_game.state < STATE_RUNNING);
	}

	runGame();

	return Common::kNoError;
}

void AgiEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	setVolumeViaSystemSetting();
}

// Scenes that need this:
//
// Manhunter 1:
//  - intro text screen (DrawPic)
//  - MAD "zooming in..." during intro and other scenes, for example room 124 (NewRoom)
//     The NewRoom call is not done during the same cycle as the "zooming in..." print call.
// Space Quest 1:
//  - right at the start of the game (NewRoom)
// Space Quest 2
//  - right at the start of the game (NewRoom)
//  - after exiting the very first room, a message pops up, that isn't readable without it (NewRoom)


// Games, that must not be triggered:
//
// Fanmade Voodoo Girl:
//  - waterfall (DrawPic, room 17)
//  - inside shop (NewRoom, changes to same room every new button, room 4)

void AgiEngine::nonBlockingText_IsShown() {
	_game.nonBlockingTextShown = true;
	_game.nonBlockingTextCyclesLeft = 2; // 1 additional script cycle is counted too
}
void AgiEngine::nonBlockingText_Forget() {
	_game.nonBlockingTextShown = false;
	_game.nonBlockingTextCyclesLeft = 0;
}
void AgiEngine::nonBlockingText_CycleDone() {
	if (_game.nonBlockingTextCyclesLeft) {
		_game.nonBlockingTextCyclesLeft--;

		if (!_game.nonBlockingTextCyclesLeft) {
			// cycle count expired, we assume that non-blocking text won't be a problem for room / pic change
			_game.nonBlockingTextShown = false;
		}
	}
}

void AgiEngine::loadingTrigger_NewRoom(int16 newRoomNr) {
	if (_game.nonBlockingTextShown) {
		_game.nonBlockingTextShown = false;

		int16 curRoomNr = getVar(VM_VAR_CURRENT_ROOM);

		if (newRoomNr != curRoomNr) {
			if (!_game.automaticRestoreGame) {
				// wait a bit, we detected non-blocking text
				pause(2000); // 2 seconds
			}
		}
	}
}

void AgiEngine::loadingTrigger_DrawPicture() {
	if (_game.nonBlockingTextShown) {
		_game.nonBlockingTextShown = false;

		if (!_game.automaticRestoreGame) {
			// wait a bit, we detected non-blocking text
			pause(2000); // 2 seconds
		}
	}
}

} // End of namespace Agi
