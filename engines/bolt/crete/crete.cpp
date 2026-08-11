/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bolt/crete/crete.h"

namespace Bolt {

namespace Crete {
	
CreteEngine::CreteEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: BoltEngine(syst, gameDesc) {
	initCallbacks();
}

void CreteEngine::boltMain() {
	// TODO
	errorUnsupportedGame("Not implemented yet");
}

void CreteEngine::initCallbacks() {
	// TODO
}

} // End of namespace Crete

} // End of namespace Bolt
