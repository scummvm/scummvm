/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
#include "common/fs.h"
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
#include "cine/sfx_player.h"
#include "cine/sound_driver.h"
#include "cine/various.h"


namespace Cine {

SoundDriver *g_soundDriver;
SfxPlayer *g_sfxPlayer;
Common::SaveFileManager *g_saveFileMan;

CineEngine *g_cine;

static void initialize();

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

	if (g_cine->getGameType() == GType_FW) {
		g_soundDriver = new AdlibSoundDriverINS(_mixer);
	} else {
		g_soundDriver = new AdlibSoundDriverADL(_mixer);
	}
	g_sfxPlayer = new SfxPlayer(g_soundDriver);
	g_saveFileMan = _saveFileMan;

	initialize();

	return 0;
}

int CineEngine::go() {
	CursorMan.showMouse(true);

	mainLoop(1);

	if (g_cine->getGameType() == Cine::GType_FW)
		snd_clearBasesonEntries();

	delete g_sfxPlayer;
	delete g_soundDriver;
	return 0;
}


static void initialize() {
	uint16 i;

	setupOpcodes();

	initLanguage(g_cine->getLanguage());
	init_video();

	textDataPtr = (byte *)malloc(8000);

	partBuffer = (PartBuffer *)malloc(NUM_MAX_PARTDATA * sizeof(PartBuffer));

	animDataTable = (AnimData *)malloc(NUM_MAX_ANIMDATA * sizeof(AnimData));
	
	loadTextData("texte.dat", textDataPtr);

	switch (g_cine->getGameType()) {
	case Cine::GType_FW:
		snd_loadBasesonEntries("BASESON.SND");
		break;
	case Cine::GType_OS:
		// TODO
		// load POLDAT.DAT
		// load ERRMESS.DAT (default responses to actions)
		break;
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

	loadPrc(BOOT_PRC_NAME);
	strcpy(currentPrcName, BOOT_PRC_NAME);

	setMouseCursor(MOUSE_CURSOR_NORMAL);
}

} // End of namespace Cine
