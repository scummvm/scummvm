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

 /*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/debug-channels.h"
#include "common/scummsys.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/rect.h"

#include "engines/util.h"

#include "lab/lab.h"
#include "lab/labfun.h"

#include "engines/advancedDetector.h"

namespace Lab {

bool LabEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsRTL) ? true : false;
}

LabEngine *g_lab;

LabEngine::LabEngine(OSystem *syst, const ADGameDescription *gameDesc)
 : Engine(syst), _gameDescription(gameDesc) {
	g_lab = this;
}

LabEngine::~LabEngine() {
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}

Common::Error LabEngine::run() {
	if (getFeatures() & GF_LOWRES)
		initGraphics(320, 200, false);
	else
		initGraphics(640, 480, true);

	g_music = new Music();

	go();

	return Common::kNoError;
}

Common::String LabEngine::generateSaveFileName(uint slot) {
	return Common::String::format("%s.%03u", _targetName.c_str(), slot);
}

} // End of namespace Lab
