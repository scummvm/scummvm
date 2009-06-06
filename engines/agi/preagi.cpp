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

#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "agi/preagi.h"
#include "agi/graphics.h"
#include "agi/sprite.h"
#include "agi/opcodes.h"
#include "agi/keyboard.h"
#include "agi/menu.h"
#include "agi/sound.h"

// preagi engines
#include "agi/preagi_mickey.h"
#include "agi/preagi_troll.h"
#include "agi/preagi_winnie.h"

namespace Agi {

PreAgiEngine::PreAgiEngine(OSystem *syst, const AGIGameDescription *gameDesc) : AgiBase(syst, gameDesc) {

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_rnd = new Common::RandomSource();

	Common::addDebugChannel(kDebugLevelMain, "Main", "Generic debug level");
	Common::addDebugChannel(kDebugLevelResources, "Resources", "Resources debugging");
	Common::addDebugChannel(kDebugLevelSprites, "Sprites", "Sprites debugging");
	Common::addDebugChannel(kDebugLevelInventory, "Inventory", "Inventory debugging");
	Common::addDebugChannel(kDebugLevelInput, "Input", "Input events debugging");
	Common::addDebugChannel(kDebugLevelMenu, "Menu", "Menu debugging");
	Common::addDebugChannel(kDebugLevelScripts, "Scripts", "Scripts debugging");
	Common::addDebugChannel(kDebugLevelSound, "Sound", "Sound debugging");
	Common::addDebugChannel(kDebugLevelText, "Text", "Text output debugging");
	Common::addDebugChannel(kDebugLevelSavegame, "Savegame", "Saving & restoring game debugging");

	memset(&_game, 0, sizeof(struct AgiGame));
	memset(&_debug, 0, sizeof(struct AgiDebug));
	memset(&g_mouse, 0, sizeof(struct Mouse));
}

void PreAgiEngine::initialize() {
	// TODO: Some sound emulation modes do not fit our current music
	//       drivers, and I'm not sure what they are. For now, they might
	//       as well be called "PC Speaker" and "Not PC Speaker".

	switch (MidiDriver::detectMusicDriver(MDT_PCSPK)) {
	case MD_PCSPK:
		_soundemu = SOUND_EMU_PC;
		break;
	default:
		_soundemu = SOUND_EMU_NONE;
		break;
	}

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

	_gfx = new GfxMgr(this);
	//_sound = new SoundMgr(this, _mixer);
	_picture = new PictureMgr(this, _gfx);
	//_sprites = new SpritesMgr(this, _gfx);

	_gfx->initMachine();

	_game.gameFlags = 0;

	_game.colorFg = 15;
	_game.colorBg = 0;

	_defaultColor = 0xF;

	_game.name[0] = '\0';

	_game.sbufOrig = (uint8 *)calloc(_WIDTH, _HEIGHT * 2); // Allocate space for two AGI screens vertically
	_game.sbuf16c  = _game.sbufOrig + SBUF16_OFFSET; // Make sbuf16c point to the 16 color (+control line & priority info) AGI screen
	_game.sbuf     = _game.sbuf16c; // Make sbuf point to the 16 color (+control line & priority info) AGI screen by default

	_game.lineMinPrint = 0; // hardcoded

	_gfx->initVideo();
	//_sound->initSound();

	_speakerStream = new Audio::PCSpeaker(_mixer->getOutputRate());
	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
							_speakerStream, -1, 255, 0, false, true);


	//_timer->installTimerProc(agiTimerFunctionLow, 10 * 1000, NULL);

	_game.ver = -1;		// Don't display the conf file warning

	debugC(2, kDebugLevelMain, "Detect game");

	// clear all resources and events
	for (int i = 0; i < MAX_DIRS; i++) {
		memset(&_game.pictures[i], 0, sizeof(struct AgiPicture));
		memset(&_game.sounds[i], 0, sizeof(class AgiSound *)); // _game.sounds contains pointers now
		memset(&_game.dirPic[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirSound[i], 0, sizeof(struct AgiDir));
	}

	debugC(2, kDebugLevelMain, "Init sound");
}

PreAgiEngine::~PreAgiEngine() {
	_mixer->stopHandle(_speakerHandle);
	delete _speakerStream;
}


Common::Error PreAgiEngine::go() {
	setflag(fSoundOn, true);	// enable sound

//
// FIXME (Fingolfin asks): Why are Mickey, Winnie and Troll standalone classes
// instead of being subclasses of PreAgiEngine ?
//

	// run preagi engine main loop
	switch (getGameID()) {
	case GID_MICKEY:
		{
			Mickey *mickey = new Mickey(this);
			mickey->init();
			mickey->run();
			delete mickey;
		}
		break;
	case GID_WINNIE:
		{
			Winnie *winnie = new Winnie(this);
			winnie->init();
			winnie->run();
			delete winnie;
		}
		break;
	case GID_TROLL:
		{
			Troll *troll = new Troll(this);
			troll->init();
			troll->run();
			delete troll;
		}
		break;
	default:
		error("Unknown preagi engine");
		break;
	}
	return Common::kNoError;
}

} // End of namespace Agi
