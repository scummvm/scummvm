/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/icb/icb.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/system.h"
#include "audio/mixer.h"

#include "graphics/pixelbuffer.h"

namespace ICB {

IcbEngine *g_icb;

IcbEngine::IcbEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, 127);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_randomSource = new Common::RandomSource("icb");
	g_icb = this;
	(void)_gameDescription; // silence warning
}

IcbEngine::~IcbEngine() {
	delete _randomSource;
	g_icb = NULL;
}

// TODO: Refactor, this is currently implemented in p4_pc.cpp
void InitEngine(const char *cmdLine);
bool mainLoopIteration();

Common::Error IcbEngine::run() {
	initGfx(false, false);
	warning("Gfx initialized");
	InitEngine("");
	warning("Init engine initialized");
	mainLoop();
	return Common::kNoError;
}

// TODO: This should be refactored
void IcbEngine::initGfx(bool hwAccel, bool fullscreen) {
	// g_system->setWindowCaption("In Cold Blood (C)2000 Revolution Software Ltd");
	// g_system->setupScreen(640, 480, false, false);
}

void IcbEngine::mainLoop() {
	warning("Starting main loop");
	while (mainLoopIteration()) {
	}
	warning("Ending main loop");
}

} // End of namespace ICB
