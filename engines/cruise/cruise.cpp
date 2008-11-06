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

#include "cruise/cruise.h"

namespace Cruise {

//SoundDriver *g_soundDriver;
//SfxPlayer *g_sfxPlayer;
Common::SaveFileManager * g_saveFileMan;

CruiseEngine *g_cruise;

CruiseEngine::CruiseEngine(OSystem * syst, const CRUISEGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {

#ifdef PALMOS_MODE
	_currentVolumeFile = new Common::File();
#endif

	Common::addSpecialDebugLevel(kCruiseDebugScript, "Script",
	    "Script debug level");

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType,
	    ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType,
	    ConfMan.getInt("music_volume"));

	g_cruise = this;

	syst->getEventManager()->registerRandomSource(_rnd, "cruise");
}

CruiseEngine::~CruiseEngine() {
#ifdef PALMOS_MODE
	delete _currentVolumeFile;
#endif
}

Common::Error CruiseEngine::init() {
	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(false);
	_system->initSize(320, 200);
	_system->endGFXTransaction();

	initialize();

	return Common::kNoError;
}

Common::Error CruiseEngine::go() {
	Cruise::changeCursor(Cruise::CURSOR_NORMAL);
	CursorMan.showMouse(true);

	Cruise::mainLoop();

	return Common::kNoError;
}

void CruiseEngine::initialize() {

	PCFadeFlag = 0;
	workBuffer = (uint8 *) mallocAndZero(8192);

	/*volVar1 = 0;
	 * fileData1 = 0; */

	/*PAL_fileHandle = -1; */

	// video init stuff

	initSystem();

	// another bit of video init

	readVolCnf();

}

} // End of namespace Cruise
