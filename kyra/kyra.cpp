/* ScummVM - Kyrandia Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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

#include "base/gameDetector.h"
#include "base/plugins.h"
#include "backends/fs/fs.h"

#include "sound/mixer.h"
#include "common/file.h"
#include "common/config-manager.h"

#include "kyra.h"
#include "resource.h"
#include "script.h"

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
        {"kyra1cd", "Legend of Kyrandia (CD)",  GF_TALKIE & GF_KYRA1,  "CHAPTER1.VRM"},
        {"kyra1", "Legend of Kyrandia (Floppy)", GF_FLOPPY & GF_KYRA1, "INTRO.SND"},
	{ 0, 0, 0, 0}
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
			const char *name = file->displayName().c_str();
			if ((!scumm_stricmp(game->detectName, name))) {
				detectedGames.push_back(game->toGameSettings());
				break;
			}
		}
	}

        return detectedGames;
}

Engine *Engine_KYRA_create(GameDetector *detector, OSystem *syst) {
	return new Kyra::KyraEngine(detector, syst);
}

REGISTER_PLUGIN("Legend of Kyrandia Engine", Engine_KYRA_gameList, Engine_KYRA_create, Engine_KYRA_detectGames)

namespace Kyra {
KyraEngine::KyraEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolume(ConfMan.getInt("sfx_volume") * ConfMan.getInt("master_volume") / 255);

	//getGameDataPath();

	// Initialize backen
	syst->initSize(320, 200);
	_screen = new uint8[320*200];
	memset((void*)_screen, 0, sizeof(_screen));

	_resMgr = new Resourcemanager(this, getGameDataPath());
	assert(_resMgr);

	setCurrentPalette(_resMgr->loadPalette("PALETTE.COL"));

	// loads the 2 cursors
	_mouse = _resMgr->loadImage("MOUSE.CPS");	//startup.pak
	_items = _resMgr->loadImage("ITEMS.CPS");

	// loads the Font
	_font = _resMgr->loadFont("8FAT.FNT");

	// loads out scripts
	_npcScript = _resMgr->loadScript("_NPC.EMC");
	_currentScript = _resMgr->loadScript("_STARTUP.EMC");
}

KyraEngine::~KyraEngine() {
	delete _resMgr;
	delete _mouse;
	delete _items;
	delete _npcScript;
	delete _currentScript;
	delete _font;
}

void KyraEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void KyraEngine::go() {
	warning("Kyrandia Engine ::go()");
	// starts the init script
	if (!_currentScript->startScript(kSetupScene)) {
		error("couldn't init '_STARTUP.EMC' script");
	}

	if (_currentScript->contScript() != kScriptStopped) {
		if (_currentScript->state() == kScriptError) {
			error("couldn't run script");
		} else {
			warning("init script returned: %d", _currentScript->state());
		}
	}

	while(true) {
		OSystem::Event event;
		//if (_debugger->isAttached())
		//	_debugger->onFrame();

		updateScreen();
		while (g_system->pollEvent(event)) {
			switch (event.event_code) {
				case OSystem::EVENT_QUIT:
					g_system->quit();
					break;
				default:
					break;
			}
		}
		_system->delayMillis(10);
	}
}

void KyraEngine::shutdown() {
	_system->quit();
}

void KyraEngine::updateScreen(void) {
        _system->copyRectToScreen(_screen, 320, 0, 0, 320, 240);
        _system->updateScreen();
}

void KyraEngine::setCurrentPalette(Palette* pal, bool delNextTime) {
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

