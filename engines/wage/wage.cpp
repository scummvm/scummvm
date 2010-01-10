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

#include "common/scummsys.h"
 
#include "common/config-manager.h"
#include "common/EventRecorder.h"
#include "common/file.h"
#include "common/fs.h"
 
#include "wage/wage.h"
#include "wage/macresman.h"
#include "wage/entities.h"
#include "wage/world.h"
 
namespace Wage {
 
WageEngine::WageEngine(OSystem *syst, const ADGameDescription *desc) : Engine(syst), _gameDescription(desc) {
	// Don't forget to register your random source
	g_eventRec.registerRandomSource(_rnd, "wage");
 
	printf("WageEngine::WageEngine\n");
}
 
WageEngine::~WageEngine() {
	// Dispose your resources here
	printf("WageEngine::~WageEngine\n");
 
	// Remove all of our debug levels here
	Common::clearAllDebugChannels();
}
 
Common::Error WageEngine::run() {
	// Initialize graphics using following:
	initGraphics(320, 200, false);
 
	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);
 
	// Additional setup.
	printf("WageEngine::init\n");
 
	// Your main even loop should be (invoked from) here.
	_resManager = new MacResManager(getGameFile());

	_world = new World();

	if (!_world->loadWorld(_resManager))
		return Common::kNoGameDataFoundError;
 
	return Common::kNoError;
}

} // End of namespace Wage
