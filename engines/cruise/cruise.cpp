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

#include "cruise/cruise.h"

namespace Cruise {

//SoundDriver *g_soundDriver;
//SfxPlayer *g_sfxPlayer;
Common::SaveFileManager *g_saveFileMan;

CruiseEngine *g_cruise;

CruiseEngine::CruiseEngine(OSystem *syst) : Engine(syst) {
	Common::addSpecialDebugLevel(kCruiseDebugScript, "Script", "Script debug level");

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	g_cruise = this;
}

CruiseEngine::~CruiseEngine() {
}

int CruiseEngine::init() {
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

	return 0;
}

int CruiseEngine::go() {
	CursorMan.showMouse(true);

	Cruise::mainLoop();

	return 0;
}


void CruiseEngine::initialize() {

	fadeVar = 0;
	ptr_something = (ctpVar19Struct*)mallocAndZero(sizeof(ctpVar19Struct)*0x200);

	/*volVar1 = 0;
	fileData1 = 0;*/

	/*PAL_fileHandle = -1;*/

	// video init stuff

	loadSystemFont();

	// another bit of video init

	readVolCnf();

}

} // End of namespace Cruise
