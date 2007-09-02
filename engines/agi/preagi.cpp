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

#include "common/events.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/cursorman.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "agi/agi.h"
#include "agi/font.h"
#include "agi/graphics.h"
#include "agi/sprite.h"
#include "agi/opcodes.h"
#include "agi/keyboard.h"
#include "agi/menu.h"
#include "agi/sound.h"

// preagi engines
#include "agi/preagi_mickey.h"

// default attributes
#define IDA_DEFAULT		0x0F
#define IDA_DEFAULT_REV	0xF0

#define IDI_MAX_ROW_PIC	20

namespace Agi {

PreAgiEngine::PreAgiEngine(OSystem *syst) : AgiBase(syst) {

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

/*
	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = agiSettings; g->gameid; ++g)
		if (!scumm_stricmp(g->gameid, gameid))
			_gameId = g->id;
*/

	_rnd = new Common::RandomSource();

	Common::addSpecialDebugLevel(kDebugLevelMain, "Main", "Generic debug level");
	Common::addSpecialDebugLevel(kDebugLevelResources, "Resources", "Resources debugging");
	Common::addSpecialDebugLevel(kDebugLevelSprites, "Sprites", "Sprites debugging");
	Common::addSpecialDebugLevel(kDebugLevelInventory, "Inventory", "Inventory debugging");
	Common::addSpecialDebugLevel(kDebugLevelInput, "Input", "Input events debugging");
	Common::addSpecialDebugLevel(kDebugLevelMenu, "Menu", "Menu debugging");
	Common::addSpecialDebugLevel(kDebugLevelScripts, "Scripts", "Scripts debugging");
	Common::addSpecialDebugLevel(kDebugLevelSound, "Sound", "Sound debugging");
	Common::addSpecialDebugLevel(kDebugLevelText, "Text", "Text output debugging");
	Common::addSpecialDebugLevel(kDebugLevelSavegame, "Savegame", "Saving & restoring game debugging");

	memset(&_game, 0, sizeof(struct AgiGame));
	memset(&_debug, 0, sizeof(struct AgiDebug));
	memset(&g_mouse, 0, sizeof(struct Mouse));

/*
	_game.clockEnabled = false;
	_game.state = STATE_INIT;

	_keyQueueStart = 0;
	_keyQueueEnd = 0;

	_keyControl = 0;
	_keyAlt = 0;

	_allowSynthetic = false;

	g_tickTimer = 0;

	_intobj = NULL;

	_stackSize = 0;
	_imageStack = NULL;
	_imageStackPointer = 0;

	_menu = NULL;

	_lastSentence[0] = 0;
	memset(&_stringdata, 0, sizeof(struct StringData));

	_objects = NULL;

	_oldMode = -1;
	
	_predictiveDialogRunning = false;
	_predictiveDictText = NULL;
	_predictiveDictLine = NULL;
	_predictiveDictLineCount = 0;
	_firstSlot = 0;
*/
}

void PreAgiEngine::initialize() {
	// TODO: Some sound emulation modes do not fit our current music
	//       drivers, and I'm not sure what they are. For now, they might
	//       as well be called "PC Speaker" and "Not PC Speaker".

	// If used platform is Apple IIGS then we must use Apple IIGS sound emulation
	// because Apple IIGS AGI games use only Apple IIGS specific sound resources.
	/*
	if (ConfMan.hasKey("platform") &&
		Common::parsePlatform(ConfMan.get("platform")) == Common::kPlatformApple2GS) {
		_soundemu = SOUND_EMU_APPLE2GS;
	} else {
		switch (MidiDriver::detectMusicDriver(MDT_PCSPK)) {
		case MD_PCSPK:
			_soundemu = SOUND_EMU_PC;
			break;
		default:
			_soundemu = SOUND_EMU_NONE;
			break;
		}
	}
	*/

	if (ConfMan.hasKey("render_mode")) {
		_renderMode = Common::parseRenderMode(ConfMan.get("render_mode").c_str());
	} else if (ConfMan.hasKey("platform")) {
		switch (Common::parsePlatform(ConfMan.get("platform"))) {
		case Common::kPlatformAmiga:
			_renderMode = Common::kRenderAmiga;
			break;
		case Common::kPlatformPC:
			_renderMode = Common::kRenderEGA;
			break;
		default:
			_renderMode = Common::kRenderEGA;
			break;
		}
	}

	//_buttonStyle = AgiButtonStyle(_renderMode);
	//_defaultButtonStyle = AgiButtonStyle();
	//_console = new Console(this);
	_gfx = new GfxMgr(this);
	//_sound = new SoundMgr(this, _mixer);
	_picture = new PictureMgr(this, _gfx);
	//_sprites = new SpritesMgr(this, _gfx);

	_gfx->initMachine();

	_game.gameFlags = 0;

	_game.colorFg = 15;
	_game.colorBg = 0;

	_game.name[0] = '\0';

	_game.sbufOrig = (uint8 *)calloc(_WIDTH, _HEIGHT * 2); // Allocate space for two AGI screens vertically
	_game.sbuf16c  = _game.sbufOrig + SBUF16_OFFSET; // Make sbuf16c point to the 16 color (+control line & priority info) AGI screen
	_game.sbuf     = _game.sbuf16c; // Make sbuf point to the 16 color (+control line & priority info) AGI screen by default

	_game.lineMinPrint = 1; // hardcoded

	_gfx->initVideo();
	//_sound->initSound();

	//_timer->installTimerProc(agiTimerFunctionLow, 10 * 1000, NULL);

	_game.ver = -1;		// Don't display the conf file warning

	debugC(2, kDebugLevelMain, "Detect game");

	_loader = new AgiLoader_preagi(this);

	if (_loader->detectGame() == errOK) {
		_game.state = STATE_LOADED;
		debugC(2, kDebugLevelMain, "game loaded");
	} else {
		report("Could not open PreAGI game");
	}

	/* clear all resources and events */
	for (int i = 0; i < MAX_DIRS; i++) {
		//memset(&_game.views[i], 0, sizeof(struct AgiView));
		memset(&_game.pictures[i], 0, sizeof(struct AgiPicture));
		//memset(&_game.logics[i], 0, sizeof(struct AgiLogic));
		//memset(&_game.sounds[i], 0, sizeof(class AgiSound *)); // _game.sounds contains pointers now
		//memset(&_game.dirView[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirPic[i], 0, sizeof(struct AgiDir));
		//memset(&_game.dirLogic[i], 0, sizeof(struct AgiDir));
		//memset(&_game.dirSound[i], 0, sizeof(struct AgiDir));
	}

	// load resources
	_loader->init();

	debugC(2, kDebugLevelMain, "Init sound");
}

PreAgiEngine::~PreAgiEngine() {

}

int PreAgiEngine::init() {

	// Detect game
	if (!initGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}

	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(false);
	_system->initSize(320, 200);
	_system->endGFXTransaction();

	initialize();

	_gfx->gfxSetPalette();

	return 0;
}

int PreAgiEngine::go() {
	// run preagi engine main loop
	switch (getGameID()) {
		case GID_MICKEY:
			{
				Mickey *mickey = new Mickey(this);
				mickey->init();
				mickey->run();
			}
			break;
		default:
			error("Unknown preagi engine");
			break;
	}		
	return 0;
}

int PreAgiEngine::preAgiLoadResource(int r, int n) {
	return _loader->loadResource(r, n);
}

int PreAgiEngine::preAgiUnloadResource(int r, int n) {
	return _loader->unloadResource(r, n);
}

// String functions
// TODO: These need to be moved elsewhere

void PreAgiEngine::drawStr(int row, int col, int attr, char *buffer)
{
	int code;

	for (int iChar = 0; iChar < (int)strlen(buffer); iChar++)
	{
		code = buffer[iChar];

		switch (code)
		{
		case '\n':
		case 0x8D:
			if (++row == 200 / 8) return;
			col = 0;
			break;

		case '|':
			// swap attribute nibbles
			break;

		default:
			drawChar(col * 8, row * 8, attr, code, (char*)mickey_fontdata);

			if (++col == 320 / 8)
			{
				col = 0;
				if (++row == 200 / 8) return;
			}
		}
	}
}

void PreAgiEngine::drawStrMiddle(int row, int attr, char *buffer) {
	int col = (25 / 2) - (strlen(buffer) / 2);	// 25 = 320 / 8 (maximum column)
	drawStr(row, col, attr, buffer);
}

void PreAgiEngine::clearTextArea() {
	// FIXME: this causes crashes, I imagine it's because we're not currently locking the screen in drawStr
	for (int row = IDI_MAX_ROW_PIC; row < 200 / 8; row++) {
		//drawStr(row, 0, IDA_DEFAULT, "                                        ");	// 40 spaces
	}
}

void PreAgiEngine::drawChar(int x, int y, int attr, int code, char *fontdata)
{
	int cx, cy;
	uint8 color;

	for (cy = 0; cy < 8; cy++)
	{
		for (cx = 0; cx < 8; cx++)
		{
			if (fontdata[(code * 8) + cy] & (1 << (7 - cx)))
				color = attr & 0x0f;			// foreground color
			else
				color = (attr & 0xf0) / 0x10;	// background color

			_gfx->putPixelsA(x + cx, y + cy, 1, &color);
		}
	}
}

} // End of namespace Agi
