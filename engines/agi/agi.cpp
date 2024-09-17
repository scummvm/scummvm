/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#include "agi/agi.h"
#include "agi/detection.h"
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

void AgiEngine::wait(uint32 msec, bool busy) {
	uint32 endTime = _system->getMillis() + msec;

	if (busy) {
		_gfx->setMouseCursor(true); // Busy mouse cursor
	}

	do {
		processScummVMEvents();
		_system->updateScreen();
		_system->delayMillis(10);
	} while (_system->getMillis() < endTime);

	if (busy) {
		_gfx->setMouseCursor(); // regular mouse cursor
	}
}

int AgiEngine::agiInit() {
	debug(2, "initializing");
	debug(2, "game version = 0x%x", getVersion());

	// initialize with adj.ego.move.to.x.y(0, 0) so to speak
	_game.adjMouseX = _game.adjMouseY = 0;

	// reset all flags to false and all variables to 0
	memset(_game.flags, 0, sizeof(_game.flags));
	memset(_game.vars, 0, sizeof(_game.vars));

	// clear all resources and events
	for (int i = 0; i < MAX_DIRECTORY_ENTRIES; i++) {
		_game.views[i].reset();
		_game.pictures[i].reset();
		_game.logics[i].reset();
		_game.sounds[i] = nullptr; // _game.sounds contains pointers now
		_game.dirView[i].reset();
		_game.dirPic[i].reset();
		_game.dirLogic[i].reset();
		_game.dirSound[i].reset();
	}

	// clear view table
	for (int i = 0; i < SCREENOBJECTS_MAX; i++) {
		_game.screenObjTable[i].reset();
	}

	_game.addToPicView.reset();

	_words->clearEgoWords();

	if (!_menu)
		_menu = new GfxMenu(this, _gfx, _picture, _text);

	_gfx->initPriorityTable();

	// Clear the string buffer on startup, but not when the game restarts, as
	// some scripts expect that the game strings remain unaffected after a
	// restart. An example is script 98 in SQ2, which is not invoked on restart
	// to ask Ego's name again. The name is supposed to be maintained in string 1.
	// Fixes bug #5673.
	if (!_restartGame) {
		memset(_game.strings, 0, sizeof(_game.strings));
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
	default:
		warning("Unknown AGI Emulation Version %x", (int)(getVersion() >> 12));
		break;
	}

	if (getPlatform() == Common::kPlatformAmiga)
		debug(1, "Amiga padded game detected.");

	if (getFeatures() & GF_AGDS)
		debug(1, "AGDS mode enabled.");

	int ec = _loader->loadDirs();

	if (ec == errOK)
		ec = _loader->loadObjects();

	// note: demos has no words.tok
	if (ec == errOK)
		ec = _loader->loadWords();

	// Load logic 0 into memory
	if (ec == errOK)
		ec = loadResource(RESOURCETYPE_LOGIC, 0);

	_keyHoldMode = false;
	_keyHoldModeLastKey = Common::KEYCODE_INVALID;

	_game.mouseFence.setWidth(0); // Reset

	// Reset in-game timer
	inGameTimerReset();

	applyVolumeToMixer();

	return ec;
}

void AgiEngine::unloadResources() {
	// Make sure logic 0 is always loaded
	for (int i = 1; i < MAX_DIRECTORY_ENTRIES; i++) {
		unloadResource(RESOURCETYPE_LOGIC, i);
	}
	for (int i = 0; i < MAX_DIRECTORY_ENTRIES; i++) {
		unloadResource(RESOURCETYPE_VIEW, i);
		unloadResource(RESOURCETYPE_PICTURE, i);
		unloadResource(RESOURCETYPE_SOUND, i);
	}
}

void AgiEngine::agiDeinit() {
	if (!_loader)
		return;

	_words->clearEgoWords(); // remove all words from memory
	unloadResources();    // unload resources in memory
	unloadResource(RESOURCETYPE_LOGIC, 0);
	_objects.clear();
	_words->unloadDictionary();

	clearImageStack();
}

int AgiEngine::loadResource(int16 resourceType, int16 resourceNr) {
	if (resourceNr >= MAX_DIRECTORY_ENTRIES)
		return errBadResource;

	int ec = errOK;
	uint8 *data = nullptr;
	switch (resourceType) {
	case RESOURCETYPE_LOGIC:
		if (~_game.dirLogic[resourceNr].flags & RES_LOADED) {
			unloadResource(RESOURCETYPE_LOGIC, resourceNr);

			// load raw resource into data
			data = _loader->loadVolumeResource(&_game.dirLogic[resourceNr]);
			_game.logics[resourceNr].data = data;

			// uncompressed logic files need to be decrypted
			if (data != nullptr) {
				// RES_LOADED flag gets set by decode logic
				ec = decodeLogic(resourceNr);
				_game.logics[resourceNr].sIP = 2;
			} else {
				ec = errBadResource;
			}
		}

		// reset code pointer in case logic was cached
		_game.logics[resourceNr].cIP = _game.logics[resourceNr].sIP;
		break;

	case RESOURCETYPE_PICTURE:
		if (~_game.dirPic[resourceNr].flags & RES_LOADED) {
			// if loaded but not cached, unload it
			// if cached but not loaded, etc
			unloadResource(RESOURCETYPE_PICTURE, resourceNr);
			data = _loader->loadVolumeResource(&_game.dirPic[resourceNr]);

			if (data != nullptr) {
				_game.pictures[resourceNr].rdata = data;
				_game.dirPic[resourceNr].flags |= RES_LOADED;
			} else {
				ec = errBadResource;
			}
		}
		break;

	case RESOURCETYPE_SOUND:
		if (~_game.dirSound[resourceNr].flags & RES_LOADED) {
			data = _loader->loadVolumeResource(&_game.dirSound[resourceNr]);

			// "data" is freed by objects created by createFromRawResource on success
			_game.sounds[resourceNr] = AgiSound::createFromRawResource(data, _game.dirSound[resourceNr].len, resourceNr, _soundemu);
			if (_game.sounds[resourceNr] != nullptr) {
				_game.dirSound[resourceNr].flags |= RES_LOADED;
			} else {
				free(data);
				ec = errBadResource;
			}
		}
		break;

	case RESOURCETYPE_VIEW:
		// Load a VIEW resource into memory...
		// Since VIEWS alter the view table ALL the time
		// can we cache the view? or must we reload it all
		// the time?
		if (~_game.dirView[resourceNr].flags & RES_LOADED) {
			unloadResource(RESOURCETYPE_VIEW, resourceNr);
			data = _loader->loadVolumeResource(&_game.dirView[resourceNr]);
			if (data) {
				_game.dirView[resourceNr].flags |= RES_LOADED;
				ec = decodeView(data, _game.dirView[resourceNr].len, resourceNr);
				free(data);
			} else {
				ec = errBadResource;
			}
		}
		break;

	default:
		ec = errBadResource;
		break;
	}

	// WORKAROUND: Patches broken picture 147 in a corrupted Amiga version of Gold Rush! (v2.05 1989-03-09).
	// The picture can be seen in room 147 after dropping through the outhouse's hole in room 146.
	if (ec == errOK && getGameID() == GID_GOLDRUSH && resourceType == RESOURCETYPE_PICTURE && resourceNr == 147 && _game.dirPic[resourceNr].len == 1982) {
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

	return ec;
}

void AgiEngine::unloadResource(int16 resourceType, int16 resourceNr) {
	switch (resourceType) {
	case RESOURCETYPE_LOGIC:
		unloadLogic(resourceNr);
		break;
	case RESOURCETYPE_PICTURE:
		_picture->unloadPicture(resourceNr);
		break;
	case RESOURCETYPE_VIEW:
		unloadView(resourceNr);
		break;
	case RESOURCETYPE_SOUND:
		_sound->unloadSound(resourceNr);
		break;
	default:
		break;
	}
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
	_sound = nullptr;

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
		// Keep EGA
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
	case Common::kPlatformMacintosh:
		_renderMode = Common::kRenderMacintosh;
		break;
	default:
		break;
	}

	// If render mode is explicitly set, force rendermode
	switch (configRenderMode) {
	case Common::kRenderCGA:
		_renderMode = Common::kRenderCGA;
		break;
	case Common::kRenderEGA:
		_renderMode = Common::kRenderEGA;
		break;
	case Common::kRenderVGA:
		_renderMode = Common::kRenderVGA;
		break;
	case Common::kRenderHercG:
		_renderMode = Common::kRenderHercG;
		break;
	case Common::kRenderHercA:
		_renderMode = Common::kRenderHercA;
		break;
	case Common::kRenderAmiga:
		_renderMode = Common::kRenderAmiga;
		break;
	case Common::kRenderApple2GS:
		_renderMode = Common::kRenderApple2GS;
		break;
	case Common::kRenderAtariST:
		_renderMode = Common::kRenderAtariST;
		break;
	case Common::kRenderMacintosh:
		_renderMode = Common::kRenderMacintosh;
		break;
	default:
		break;
	}

	if (getFeatures() & GF_AGI256) {
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

	memset(&_debug, 0, sizeof(struct AgiDebug));

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

	_keyQueueStart = 0;
	_keyQueueEnd = 0;

	_allowSynthetic = false;

	_intobj = nullptr;

	_restartGame = false;

	_firstSlot = 0;

	resetControllers();

	_game._curLogic = nullptr;
	_veryFirstInitialCycle = true;
	_instructionCounter = 0;
	resetGetVarSecondsHeuristic();

	_setVolumeBrokenFangame = false; // for further study see AgiEngine::applyVolumeToMixer()

	_playTimeInSecondsAdjust = 0;
	_lastUsedPlayTimeInCycles = 0;
	_lastUsedPlayTimeInSeconds = 0;
	_passedPlayTimeCycles = 0;

	memset(_keyQueue, 0, sizeof(_keyQueue));

	_font = nullptr;
	_gfx = nullptr;
	_sound = nullptr;
	_picture = nullptr;
	_sprites = nullptr;
	_text = nullptr;
	_loader = nullptr;
	_menu = nullptr;
	_systemUI = nullptr;
	_inventory = nullptr;
	_logFile = nullptr;

	_keyHoldMode = false;
	_keyHoldModeLastKey = Common::KEYCODE_INVALID;

	_artificialDelayCurrentRoom = 0;
	_artificialDelayCurrentPicture = 0;
}

void AgiEngine::initialize() {
	// TODO: Some sound emulation modes do not fit our current music
	//       drivers, and I'm not sure what they are. For now, they might
	//       as well be called "PC Speaker" and "Not PC Speaker".

	// If platform is Apple or CoCo3 then their sound emulation must be used.
	// The sound resources in these games have platform-specific formats.
	if (getPlatform() == Common::kPlatformApple2) {
		_soundemu = SOUND_EMU_APPLE2;
	} else if (getPlatform() == Common::kPlatformApple2GS) {
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
			_soundemu = SOUND_EMU_MIDI;
			break;
		}
	}

	initRenderMode();

	setDebugger(new Console(this));
	_words = new Words(this);
	_font = new GfxFont(this);
	_gfx = new GfxMgr(this, _font);
	_sound = new SoundMgr(this, _mixer);
	_picture = new PictureMgr(this, _gfx);
	_sprites = new SpritesMgr(this, _gfx);
	_text = new TextMgr(this, _words, _gfx);
	_systemUI = new SystemUI(this, _gfx, _text);
	_inventory = new InventoryMgr(this, _gfx, _text, _systemUI);

	_font->init();
	_gfx->initVideo();

	_text->init(_systemUI);

	_text->charAttrib_Set(15, 0);

	if (getPlatform() == Common::kPlatformApple2) {
		_loader = new AgiLoader_A2(this);
	} else if (getVersion() <= 0x2001) {
		_loader = new AgiLoader_v1(this);
	} else if (getVersion() <= 0x2999) {
		_loader = new AgiLoader_v2(this);
	} else {
		_loader = new AgiLoader_v3(this);
	}
	_loader->init();
	
	// finally set up actual VM opcodes, because we should now have figured out the right AGI version
	setupOpCodes(getVersion());
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

AgiEngine::~AgiEngine() {
	agiDeinit();
	delete _loader;
	if (_gfx) {
		_gfx->deinitVideo();
	}
	if (_logFile) {
		_logFile->finalize();
		_logFile->close();
	}
	delete _logFile;
	delete _inventory;
	delete _systemUI;
	delete _menu;
	delete _text;
	delete _sprites;
	delete _picture;
	delete _gfx;
	delete _font;
	delete _words;
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

	int ec = runGame();

	switch (ec) {
	case errOK:            return Common::kNoError;
	case errFilesNotFound: return Common::kNoGameDataFoundError;
	case errBadFileOpen:   return Common::kReadingFailed;
	default:               return Common::kUnknownError;
	}
}

void AgiEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	applyVolumeToMixer();
}

// WORKAROUND:
// Sometimes Sierra printed some text on the screen and did a room change immediately afterwards expecting the
// interpreter to load the data for a bit of time. This of course doesn't happen in our AGI, so we try to
// detect such situations via heuristic and then delay the game for a bit.
// In those cases a wait mouse cursor will be shown.
//
// Scenes that need this:
//
// Gold Rush:
//  - During Stagecoach path, after getting solving the steep hill "Congratulations!!!" (NewRoom)
//  - when following your mule "Yet right on his tail!!!" (NewRoom/NewPicture - but room 123 stays the same)
// Manhunter 1:
//  - intro text screen (DrawPic)
//  - MAD "zooming in..." during intro and other scenes, for example room 124 (NewRoom)
//     The NewRoom call is not done during the same cycle as the "zooming in..." print call.
// Space Quest 1:
//  - right at the start of the game (NewRoom)
//  - right at the end of the asteroids "That was mighty close!" (NewRoom)
// Space Quest 2
//  - right at the start of the game (NewRoom)
//  - after exiting the very first room, a message pops up, that isn't readable without it (NewRoom)
//  - Climbing into shuttle on planet Labion. "You open the hatch and head on in." (NewRoom)


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

void AgiEngine::artificialDelay_Reset() {
	nonBlockingText_Forget();
	_artificialDelayCurrentRoom = -1;
	_artificialDelayCurrentPicture = -1;
}

void AgiEngine::artificialDelay_CycleDone() {
	if (_game.nonBlockingTextCyclesLeft) {
		_game.nonBlockingTextCyclesLeft--;

		if (!_game.nonBlockingTextCyclesLeft) {
			// cycle count expired, we assume that non-blocking text won't be a problem for room / pic change
			_game.nonBlockingTextShown = false;
		}
	}
}

// WORKAROUND:
// On Apple IIgs, there are situations like for example the Police Quest 1 intro, where music is playing
// and then the scripts switch to a new room, expecting it to load for a bit of time. In ScummVM this results
// in music getting cut off, because our loading is basically done in an instant. This also happens in the
// original interpreter, when you use a faster CPU in emulation.
//
// That's why there is an additional table, where one can add such situations to it.
// These issues are basically impossible to detect, because sometimes music is also supposed to play throughout
// multiple rooms.
//
// Normally all text-based issues should get detected by the current heuristic. Do not add those in here.

//         script, description,                                       signature                   patch
static const AgiArtificialDelayEntry artificialDelayTable[] = {
	{ GID_GOLDRUSH,   Common::kPlatformApple2GS, ARTIFICIALDELAYTYPE_NEWROOM,     14,  21, 2200 }, // Stagecoach path: right after getting on it in Brooklyn
	{ GID_PQ1,        Common::kPlatformApple2GS, ARTIFICIALDELAYTYPE_NEWPICTURE,   1,   2, 2200 }, // Intro: music track is supposed to finish before credits screen. Developers must have assumed that room loading would take that long.
	{ GID_MH1,        Common::kPlatformApple2GS, ARTIFICIALDELAYTYPE_NEWPICTURE, 155, 183, 2200 }, // Happens, when hitting fingers at bar
	{ GID_AGIDEMO,    Common::kPlatformUnknown,  ARTIFICIALDELAYTYPE_END,         -1,  -1,    0 }
};

uint16 AgiEngine::artificialDelay_SearchTable(AgiArtificialDelayTriggerType triggerType, int16 orgNr, int16 newNr) {
	if (getPlatform() != Common::kPlatformApple2GS) {
		return 0;
	}

	const AgiArtificialDelayEntry *delayEntry = artificialDelayTable;

	while (delayEntry->triggerType != ARTIFICIALDELAYTYPE_END) {
		if (triggerType == delayEntry->triggerType) {
			if ((orgNr == delayEntry->orgNr) && (newNr == delayEntry->newNr)) {
				if ((getGameID() == delayEntry->gameId) && (getPlatform() == delayEntry->platform)) {
					warning("artificial delay forced");
					return delayEntry->millisecondsDelay;
				}
			}
		}

		delayEntry++;
	}
	return 0;
}

void AgiEngine::artificialDelayTrigger_NewRoom(int16 newRoomNr) {
	//warning("artificial delay trigger: room %d -> new room %d", _artificialDelayCurrentRoom, newRoomNr);

	if (!_game.automaticRestoreGame) {
		uint16 millisecondsDelay = artificialDelay_SearchTable(ARTIFICIALDELAYTYPE_NEWROOM, _artificialDelayCurrentRoom, newRoomNr);

		if (_game.nonBlockingTextShown) {
			if (newRoomNr != _artificialDelayCurrentRoom) {
				if (millisecondsDelay < 2000) {
					// wait a bit, we detected non-blocking text
					millisecondsDelay = 2000; // 2 seconds
				}
			}
		}

		if (millisecondsDelay) {
			wait(millisecondsDelay, true); // set busy mouse cursor
			_game.nonBlockingTextShown = false;
		}
	}

	_artificialDelayCurrentRoom = newRoomNr;
}

void AgiEngine::artificialDelayTrigger_DrawPicture(int16 newPictureNr) {
	//warning("artificial delay trigger: picture %d -> new picture %d", _artificialDelayCurrentPicture, newPictureNr);

	if (!_game.automaticRestoreGame) {
		uint16 millisecondsDelay = artificialDelay_SearchTable(ARTIFICIALDELAYTYPE_NEWPICTURE, _artificialDelayCurrentPicture, newPictureNr);

		if (_game.nonBlockingTextShown) {
			if (newPictureNr != _artificialDelayCurrentPicture) {
				if (millisecondsDelay < 2000) {
					// wait a bit, we detected non-blocking text
					millisecondsDelay = 2000; // 2 seconds, set busy
				}
			}
		}

		if (millisecondsDelay) {
			wait(millisecondsDelay, true); // set busy mouse cursor
			_game.nonBlockingTextShown = false;
		}
	}
	_artificialDelayCurrentPicture = newPictureNr;
}

const char *AgiGame::getString(int number) {
	if (0 <= number && number <= MAX_STRINGS) {
		return strings[number];
	} else {
		// WORKAROUND: Flag Quest detects the interpreter version by comparing
		// out of bounds strings to values know to be in memory in Sierra's
		// interpreters. The game only starts if a known value matches an
		// allowed version. We return the value for version 2.917. Bug #15060
		if (number == 56) {
			return ".917";
		}
		warning("invalid string number: %d", number);
		return "";
	}
}

void AgiGame::setString(int number, const char *str) {
	if (0 <= number && number <= MAX_STRINGS) {
		Common::strlcpy(strings[number], str, MAX_STRINGLEN);
	} else {
		// Occurs in Groza, number = 150
		warning("invalid string number: %d, '%s'", number, str);
	}
}

void AgiGame::setSpeedLevel(byte s) {
	speedLevel = s;
	_vm->setVar(VM_VAR_WINDOW_AUTO_CLOSE_TIMER, 6);
	switch (speedLevel) {
	case 0:
		_vm->_text->messageBox("Fastest speed.");
		break;
	case 1:
		_vm->_text->messageBox("Fast speed.");
		break;
	case 2:
		_vm->_text->messageBox("Normal speed.");
		break;
	case 3:
	case 4:
		_vm->_text->messageBox("Slow speed.");
		break;
	}
}

} // End of namespace Agi
