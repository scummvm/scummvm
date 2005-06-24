/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

#include "common/stdafx.h"

#include "common/system.h"
#include "base/gameDetector.h"
#include "base/plugins.h"
#include "backends/fs/fs.h"

#include "sound/mixer.h"
#include "common/file.h"
#include "common/config-manager.h"
#include "sound/mididrv.h"

#include "kyra/kyra.h"
#include "kyra/resource.h"
#include "kyra/script.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"

struct KyraGameSettings {
	const char *name;
	const char *description;
	uint32 features;
	const char *detectName;
	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
};

static const KyraGameSettings kyra_settings[] = {
	{"kyra1", "Legend of Kyrandia (Floppy)", GF_FLOPPY | GF_KYRA1, "INTRO.SND"},
	{"kyra1cd", "Legend of Kyrandia (CD)",  GF_TALKIE | GF_KYRA1,  "CHAPTER1.VRM"},
	{"kyra2", "Hand of Fate (Floppy)", GF_FLOPPY | GF_KYRA2, 0 },
	{"kyra2cd", "Hand of Fate (CD)", GF_TALKIE | GF_KYRA2, "AUDIO.PAK"},
	{"kyra3", "Malcolm's Revenge", GF_TALKIE | GF_KYRA3, "K3INTRO0.VQA"},
	{0, 0, 0, 0}
};

GameList Engine_KYRA_gameList() {
	GameList games;
	const KyraGameSettings *g = kyra_settings;

	while (g->name) {
		games.push_back(g->toGameSettings());
		g++;
	}

	return games;
}

DetectedGameList Engine_KYRA_detectGames(const FSList &fslist) {
	const KyraGameSettings *game;
	DetectedGameList detectedGames;

	for (game = kyra_settings; game->name; ++game) {
		if (game->detectName == NULL)
			continue;

		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (!file->isDirectory()) {
				const char *name = file->displayName().c_str();
				if ((!scumm_stricmp(game->detectName, name))) {
					detectedGames.push_back(game->toGameSettings());
					break;
				}
			}
		}
	}

	return detectedGames;
}

Engine *Engine_KYRA_create(GameDetector *detector, OSystem *syst) {
	return new Kyra::KyraEngine(detector, syst);
}

REGISTER_PLUGIN(KYRA, "Legend of Kyrandia Engine")

namespace Kyra {
KyraEngine::KyraEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	// gets the game
	if (detector->_game.features & GF_KYRA1) {
		if (detector->_game.features & GF_FLOPPY)
			_game = KYRA1;
		else
			_game = KYRA1CD;
	} else if (detector->_game.features & GF_KYRA2) {
		if (detector->_game.features & GF_FLOPPY)
			_game = KYRA2;
		else
			_game = KYRA2CD;
	} else if (detector->_game.features & GF_KYRA3) {
		_game = KYRA3;
	} else {
		error("unknown game");
	}
}

int KyraEngine::init(GameDetector &detector) {

	// Initialize backen
	_system->beginGFXTransaction();
		initCommonGFX(detector);
		_system->initSize(320, 200);
	_system->endGFXTransaction();

	_screen = new uint8[320*200];
	memset(_screen, 0, sizeof(uint8) * 320 * 200);

	int midiDrv = MidiDriver::detectMusicDriver(MDT_NATIVE | MDT_ADLIB | MDT_PREFER_NATIVE);
	bool native_mt32 = (ConfMan.getBool("native_mt32") || (midiDrv == MD_MT32));

	MidiDriver *driver = MidiDriver::createMidi(midiDrv);
	if (!driver) {
		// In this case we should play the Adlib tracks, but for now
		// the automagic MIDI-to-Adlib conversion will do.
		driver = MidiDriver_ADLIB_create(_mixer);
	} else if (native_mt32)
		driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	_midiDriver = new MusicPlayer(driver, this);
	assert(_midiDriver);
	_midiDriver->hasNativeMT32(native_mt32);
	_midiDriver->setVolume(255);

	_resMgr = new Resourcemanager(this);
	assert(_resMgr);

	setCurrentPalette(_resMgr->loadPalette("PALETTE.COL"));

	// loads the 2 cursors
	_mouse = _resMgr->loadImage("MOUSE.CPS");
	_items = _resMgr->loadImage("ITEMS.CPS");

	// loads the Font
	_font = _resMgr->loadFont("8FAT.FNT");
	
	_npcScript = _resMgr->loadScript("_NPC.EMC");

	// loads the scripts (only Kyrandia 1)
	if (_game == KYRA1 || _game == KYRA1CD) {
		_currentScript = _resMgr->loadScript("_STARTUP.EMC");
	} else {
		error("game start files not known");
	}
	
	assert(_npcScript);
	assert(_currentScript);
	
	return 0;
}

KyraEngine::~KyraEngine() {
	delete _resMgr;
	delete[] _screen;
	delete _mouse;
	delete _items;
	delete _npcScript;
	delete _currentScript;
	delete _font;
}

void KyraEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

int KyraEngine::go() {
	warning("Kyrandia Engine ::go()");
	// starts the init script
/*	if (!_currentScript->startScript(kSetupScene)) {
		error("couldn't init '_STARTUP.EMC' script");
	}

	if (_currentScript->contScript() != kScriptStopped) {
		if (_currentScript->state() == kScriptError) {
			error("couldn't run script");
		} else {
			warning("init script returned: %d", _currentScript->state());
		}
	}*/
	
	Movie* movie = _resMgr->loadMovie("MAL-KAL.WSA");
	assert(movie);
	CPSImage* image = _resMgr->loadImage("GEMCUT.CPS");
	assert(image);

	int16 currentFrame = 0;
	uint32 lastFrameChange = 0;

	image->transparency(0);
	image->drawToPlane(_screen, 320, 200, 0, 0, 0, 0, 320, 136);
	movie->setImageBackground(_screen, 320, 200);
	movie->position(16, 58);

	setCurrentPalette(_resMgr->loadPalette("MAL-KAL.COL"));

	uint8* _buffer = new uint8[320 * 200];
	assert(_buffer);
	memcpy(_buffer, _screen, 320 * 200);
	movie->renderFrame(_buffer, 320, 200, movie->countFrames() - 1);

	if (_midiDriver) {
		_midiDriver->playMusic("KYRA2A.XMI");
		_midiDriver->playTrack(3);
	}
	
	while (true) {
		OSystem::Event event;
		//if (_debugger->isAttached())
		//	_debugger->onFrame();
		
		memcpy(_screen, _buffer, 320 * 200);
		if (lastFrameChange + movie->frameChange() < _system->getMillis()) {
			lastFrameChange = _system->getMillis();
			++currentFrame;

			if (currentFrame >= (int16)movie->countFrames()) {
				currentFrame = 0;
			}
		}
		
		movie->renderFrame(_screen, 320, 200, currentFrame);
		_font->drawStringToPlane("This is only a test!", _screen, 320, 200, 75, 179, 136);
		_font->drawStringToPlane("Nothing scripted!", _screen, 320, 200, 85, 189, 136);
		updateScreen();
		while (g_system->pollEvent(event)) {
			switch (event.type) {
				case OSystem::EVENT_QUIT:
					g_system->quit();
					break;
				default:
					break;
			}
		}
		_system->delayMillis(10);
	}
	
	delete movie;
	delete image;
	delete [] _buffer;
	
	return 0;
}

void KyraEngine::shutdown() {
	_system->quit();
}

void KyraEngine::updateScreen(void) {
	_system->copyRectToScreen(_screen, 320, 0, 0, 320, 200);
	_system->updateScreen();
}

void KyraEngine::setCurrentPalette(Palette* pal, bool delNextTime) {
	if (!pal)
		return;

//        if (_delPalNextTime)
//                delete _currentPal;

//        _delPalNextTime = delNextTime;

//        _currentPal = pal;

	if (pal->getData()) {
		_system->setPalette(pal->getData(), 0, 256);
	} else {
		warning("palette contains no data");
	}
}

} // End of namespace KYRA

