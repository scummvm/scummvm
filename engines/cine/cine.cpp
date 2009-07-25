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

#include "common/events.h"
#include "common/EventRecorder.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/system.h"

#include "graphics/cursorman.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "cine/cine.h"
#include "cine/bg_list.h"
#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/texte.h"
#include "cine/sound.h"
#include "cine/various.h"

namespace Cine {

Sound *g_sound;
Common::SaveFileManager *g_saveFileMan;

CineEngine *g_cine;

CineEngine::CineEngine(OSystem *syst, const CINEGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	Common::addDebugChannel(kCineDebugScript, "Script", "Script debug level");
	Common::addDebugChannel(kCineDebugPart,   "Part",   "Part debug level");
	Common::addDebugChannel(kCineDebugSound,  "Sound",  "Sound debug level");

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	// Use music volume for plain sound types (At least the Adlib player uses a plain sound type
	// so previously the music and sfx volume controls didn't affect it at all).
	// FIXME: Make Adlib player differentiate between playing sound effects and music and remove this.
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, ConfMan.getInt("music_volume"));

	g_cine = this;

	g_eventRec.registerRandomSource(_rnd, "cine");
}

CineEngine::~CineEngine() {
	if (g_cine->getGameType() == Cine::GType_OS) {
		freeErrmessDat();
	}
	Common::clearAllDebugChannels();
}

Common::Error CineEngine::run() {
	// Initialize backend
	initGraphics(320, 200, false);

	if (g_cine->getPlatform() == Common::kPlatformPC) {
		g_sound = new PCSound(_mixer, this);
	} else {
		// Paula chipset for Amiga and Atari versions
		g_sound = new PaulaSound(_mixer, this);
	}
	g_saveFileMan = _saveFileMan;

	initialize();

	CursorMan.showMouse(true);
	mainLoop(1);

	delete renderer;
	delete[] collisionPage;
	delete g_sound;

	return Common::kNoError;
}

int CineEngine::getTimerDelay() const {
	return (10923000 * _timerDelayMultiplier) / 1193180;
}

/*! \brief Modify game speed
 * \param speedChange Negative values slow game down, positive values speed it up, zero does nothing
 * \return Timer delay multiplier's value after the game speed change
 */
int CineEngine::modifyGameSpeed(int speedChange) {
	// If we want more speed we decrement the timer delay multiplier and vice versa.
	_timerDelayMultiplier = CLIP(_timerDelayMultiplier - speedChange, 1, 50);
	return _timerDelayMultiplier;
}

void CineEngine::initialize() {
	// Resize object table to its correct size and reset all its elements
	objectTable.resize(NUM_MAX_OBJECT);
	resetObjectTable();

	// Resize animation data table to its correct size and reset all its elements
	animDataTable.resize(NUM_MAX_ANIMDATA);
	freeAnimDataTable();

	// Resize zone data table to its correct size and reset all its elements
	zoneData.resize(NUM_MAX_ZONE);
	Common::set_to(zoneData.begin(), zoneData.end(), 0);

	// Resize zone query table to its correct size and reset all its elements
	zoneQuery.resize(NUM_MAX_ZONE);
	Common::set_to(zoneQuery.begin(), zoneQuery.end(), 0);

	_timerDelayMultiplier = 12; // Set default speed
	setupOpcodes();

	initLanguage(g_cine->getLanguage());

	if (g_cine->getGameType() == Cine::GType_OS) {
		renderer = new OSRenderer;
	} else {
		renderer = new FWRenderer;
	}

	collisionPage = new byte[320 * 200];

	// Clear part buffer as there's nothing loaded into it yet.
	// Its size will change when loading data into it with the loadPart function.
	partBuffer.clear();

	if (g_cine->getGameType() == Cine::GType_OS) {
		readVolCnf();
	}

	loadTextData("texte.dat");

	if (g_cine->getGameType() == Cine::GType_OS && !(g_cine->getFeatures() & GF_DEMO)) {
		loadPoldatDat("poldat.dat");
		loadErrmessDat("errmess.dat");
	}

	// in case ScummVM engines can be restarted in the future
	scriptTable.clear();
	relTable.clear();
	objectScripts.clear();
	globalScripts.clear();
	bgIncrustList.clear();
	freeAnimDataTable();
	overlayList.clear();
	messageTable.clear();
	resetObjectTable();

	var8 = 0;

	var2 = var3 = var4 = var5 = 0;

	musicIsPlaying = 0;
	currentDatName[0] = 0;

	_preLoad = false;
	if (ConfMan.hasKey("save_slot")) {
		char saveNameBuffer[256];

		sprintf(saveNameBuffer, "%s.%1d", _targetName.c_str(), ConfMan.getInt("save_slot"));

		bool res = makeLoad(saveNameBuffer);

		if (res)
			_preLoad = true;
	}

	if (!_preLoad) {
		loadPrc(BOOT_PRC_NAME);
		strcpy(currentPrcName, BOOT_PRC_NAME);
		setMouseCursor(MOUSE_CURSOR_NORMAL);
	}
}

} // End of namespace Cine
