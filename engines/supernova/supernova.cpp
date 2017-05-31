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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"

#include "engines/util.h"

#include "supernova/supernova.h"

namespace Supernova {

SupernovaEngine::SupernovaEngine(OSystem *syst)
	: Engine(syst)
	, _console(NULL)
{
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "sound");

	// setup engine specific debug channels
	DebugMan.addDebugChannel(kDebugGeneral, "general", "Supernova general debug channel");

	_rnd = new Common::RandomSource("supernova");
	
	debug("SupernovaEngine::SupernovaEngine");
}

SupernovaEngine::~SupernovaEngine() {
	debug("SupernovaEngine::~SupernovaEngine");

	delete _rnd;
	DebugMan.clearAllDebugChannels();
}

Common::Error SupernovaEngine::run() {
	initGraphics(320, 200);
	_console = new Console(this);

	debug("SupernovaEngine::init");

	return Common::kNoError;
}

}
