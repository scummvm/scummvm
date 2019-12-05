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

#include "ultima8/ultima8.h"
#include "ultima8/detection.h"
#include "ultima8/kernel/gui_app.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"

namespace Ultima8 {

Ultima8Engine *g_vm;

Ultima8Engine::Ultima8Engine(OSystem *syst, const Ultima8GameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Ultima8") {
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugGraphics, "Graphics", "Graphics debug level");
}

Ultima8Engine::~Ultima8Engine() {
}

uint32 Ultima8Engine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Error Ultima8Engine::run() {
	GUIApp app(0, nullptr);
	app.startup();
	app.run();

	return Common::kNoError;
}

Common::FSNode Ultima8Engine::getGameDirectory() const {
	return Common::FSNode(ConfMan.get("path"));
}

} // End of namespace Ultima8
