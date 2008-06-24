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
	Common::addSpecialDebugLevel(kCineDebugScript, "Script", "Script debug level");
	Common::addSpecialDebugLevel(kCineDebugPart,   "Part",   "Part debug level");
	Common::addSpecialDebugLevel(kCineDebugSound,  "Sound",  "Sound debug level");

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	g_cine = this;

	syst->getEventManager()->registerRandomSource(_rnd, "cine");
}

CineEngine::~CineEngine() {
	if (g_cine->getGameType() == Cine::GType_OS) {
		freePoldatDat();
		freeErrmessDat();
	}
	Common::clearAllSpecialDebugLevels();

	free(palPtr);
	free(partBuffer);
	free(textDataPtr);
}

int CineEngine::init() {
	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(false);
	_system->initSize(320, 200);
	_system->endGFXTransaction();

	if (g_cine->getPlatform() == Common::kPlatformPC) {
		g_sound = new PCSound(_mixer, this);
	} else {
		// Paula chipset for Amiga and Atari versions
		g_sound = new PaulaSound(_mixer, this);
	}
	g_saveFileMan = _saveFileMan;

	initialize();

	return 0;
}

int CineEngine::go() {
	CursorMan.showMouse(true);
	mainLoop(1);

	delete renderer;
	delete[] page3Raw;
	delete g_sound;
	
	return _rtl;
}


void CineEngine::initialize() {
	setupOpcodes();

	initLanguage(g_cine->getLanguage());

	if (g_cine->getGameType() == Cine::GType_OS) {
		renderer = new OSRenderer;
	} else {
		renderer = new FWRenderer;
	}

	page3Raw = new byte[320 * 200];
	textDataPtr = (byte *)malloc(8000);

	partBuffer = (PartBuffer *)malloc(NUM_MAX_PARTDATA * sizeof(PartBuffer));

	if (g_cine->getGameType() == Cine::GType_OS) {
		readVolCnf();
	}

	loadTextData("texte.dat", textDataPtr);

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

	memset(objectTable, 0, sizeof(objectTable));

	var8 = 0;

	var2 = var3 = var4 = var5 = 0;

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
