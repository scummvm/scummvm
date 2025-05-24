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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/console.h"
#include "bagel/hodjnpodj/gfx/bold_font.h"
#include "bagel/music.h"
#include "bagel/hodjnpodj/metagame/frame/hodjpodj.h"

namespace Bagel {
namespace HodjNPodj {

#define LOGO_MIDI       "sound/maintitl.mid"

HodjNPodjEngine *g_engine;
GAMESTRUCT *pGameParams;
CBfcMgr *lpMetaGame;

HodjNPodjEngine::HodjNPodjEngine(OSystem *syst, const ADGameDescription *gameDesc) :
	BagelEngine(syst, gameDesc) {
	g_engine = this;
	pGameParams = &_gameInfo;
	lpMetaGame = &_metaGame;
}

HodjNPodjEngine::~HodjNPodjEngine() {
}

Common::Error HodjNPodjEngine::run() {
	initGraphics(GAME_WIDTH, GAME_HEIGHT);
	_screen = new Graphics::Screen();

	// Set the engine's debugger console
	setDebugger(new Console());

	_midi = new MusicPlayer();

	// Load the font
	for (int size = 8; size <= 14; size += 2) {
		Graphics::WinFont *font = new Graphics::WinFont();
		if (!font->loadFromFON("msserif.fon",
		                       Graphics::WinFontDirEntry("MS Sans Serif", size)))
			error("Could not load msserif.fon");
		_fonts[size] = new Gfx::BoldFont(font);
	}

	//_metaGame.initBFCInfo();
#if 0
	_settings.load();
	Settings::Domain &meta = _settings["Meta"];
	_bAnimationsEnabled = meta.getBool("Animations", true);
	_bScrollingEnabled = meta.getBool("MapScrolling", false);
	_gameId = getGameId();
#endif
	// Run the game
	auto &app = Metagame::Frame::theApp;
	app.addResources("meta/hnpmeta.dll");
	app.Run();

	for (int size = 8; size <= 14; size += 2)
		delete _fonts[size];

//	_settings.save();
	return Common::kNoError;
}

Common::Error HodjNPodjEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

void HodjNPodjEngine::startBackgroundMidi() {
	_backgroundMidi = new CBofSound(this, LOGO_MIDI, SOUND_MIDI | SOUND_LOOP /* | SOUND_DONT_LOOP_TO_END */);
	(*_backgroundMidi).play();
}

void HodjNPodjEngine::stopBackgroundMidi() {
	if (_backgroundMidi != nullptr) {
		(*_backgroundMidi).stop();
		delete _backgroundMidi;
		_backgroundMidi = nullptr;
	}
}

void HodjNPodjEngine::selectMinigame(int newArea) {
	#ifdef TODO
	int nWhichMinigame = newArea - MG_GAME_BASE;
	assert(nWhichMinigame >= 0 && nWhichMinigame < MG_GAME_COUNT);

	getScreen()->clear();

	//bLoadedDLL = FALSE;

	if (newArea == MG_GAME_CHALLENGE) {
		startBackgroundMidi();
		#ifdef TODO
		bSuccess = LoadZoomDLL();
		bReturnToZoom = FALSE;
		if (bSuccess == FALSE) {
			lpMetaGame->m_bRestart = TRUE;
			bSuccess = LoadMetaDLL();
		} else
			startBackgroundMidi();
		#else
		error("TODO: MG_GAME_CHALLENGE");
		#endif
		return;
	}

	stopBackgroundMidi();
	#ifdef TODO
	if (bReturnToZoom) {

		if (lpGameStruct != NULL) {
			delete lpGameStruct;
			lpGameStruct = NULL;
		}
		lpGameStruct = new GAMESTRUCT;
		lpGameStruct->lCrowns = 1000;
		lpGameStruct->lScore = 0;
		lpGameStruct->nSkillLevel = SKILLLEVEL_MEDIUM;
		bSoundEffectsEnabled = pMyApp->GetProfileInt("Meta", "SoundEffects", TRUE);
		bMusicEnabled = pMyApp->GetProfileInt("Meta", "Music", TRUE);
		lpGameStruct->bSoundEffectsEnabled = bSoundEffectsEnabled;
		lpGameStruct->bMusicEnabled = bMusicEnabled;
		lpGameStruct->bPlayingMetagame = FALSE;
		lpGameStruct->bPlayingHodj = TRUE;
	}
	#endif

	// Switch to the minigame
	addView(CMgStatic::cGameTable[nWhichMinigame]._viewName);
	#else
	error("TODO: select minigame");
	#endif
}

} // namespace HodjNPodj
} // namespace Bagel
