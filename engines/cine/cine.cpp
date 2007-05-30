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

CineEngine::CineEngine(OSystem *syst) : Engine(syst) {
	Common::addSpecialDebugLevel(kCineDebugScript, "Script", "Script debug level");

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	g_cine = this;
}

CineEngine::~CineEngine() {
	if (g_cine->getGameType() == Cine::GType_OS) {
		freePoldatDat();
		freeErrmessDat();
	}
}

int CineEngine::init() {
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

	delete g_sound;
	return 0;
}


void CineEngine::initialize() {
	uint16 i;

	setupOpcodes();

	initLanguage(g_cine->getLanguage());
	gfxInit();

	textDataPtr = (byte *)malloc(8000);

	partBuffer = (PartBuffer *)malloc(NUM_MAX_PARTDATA * sizeof(PartBuffer));

	animDataTable = (AnimData *)malloc(NUM_MAX_ANIMDATA * sizeof(AnimData));
	
	loadTextData("texte.dat", textDataPtr);

	if (g_cine->getGameType() == Cine::GType_OS && !(g_cine->getFeatures() & GF_DEMO)) {
		loadPoldatDat("poldat.dat");
		loadErrmessDat("errmess.dat");
	}

	for (i = 0; i < NUM_MAX_OBJECT; i++) {
		objectTable[i].part = 0;
		objectTable[i].name[0] = 0;
	}

	for (i = 0; i < NUM_MAX_OBJECTDATA; i++) {
		globalVars[i] = 0;
	}

	for (i = 0; i < NUM_MAX_SCRIPT; i++) {
		scriptTable[i].ptr = NULL;
		scriptTable[i].size = 0;
	}

	for (i = 0; i < NUM_MAX_MESSAGE; i++) {
		messageTable[i].ptr = NULL;
		messageTable[i].len = 0;
	}

	for (i = 0; i < NUM_MAX_REL; i++) {
		relTable[i].data = NULL;
		relTable[i].size = 0;
		relTable[i].obj1Param1 = 0;
		relTable[i].obj1Param2 = 0;
		relTable[i].obj2Param = 0;
	}

	for (i = 0; i < NUM_MAX_ANIMDATA; i++) {
		animDataTable[i].ptr1 = NULL;
		animDataTable[i].ptr2 = NULL;
	}

	overlayHead.next = NULL;
	overlayHead.previous = NULL;

	var8 = 0;
	bgIncrustList = NULL;

	objScriptList.next = NULL;
	globalScriptsHead.next = NULL;

	objScriptList.scriptPtr = NULL;
	globalScriptsHead.scriptPtr = NULL;

	var2 = 0;
	var3 = 0;
	var4 = 0;
	var5 = 0;

	freePrcLinkedList();

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
