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

#include "common/config-manager.h"
#include "common/file.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

DrillerEngine::DrillerEngine(OSystem *syst) : FreescapeEngine(syst) {}

void DrillerEngine::loadAssets() {
	Common::SeekableReadStream *file = nullptr;
	Common::String path = ConfMan.get("path");
	Common::FSDirectory gameDir(path);

	Common::File exe;
	if (_renderMode == "ega") {
		file = gameDir.createReadStreamForMember("DRILLE.EXE");

		if (file == nullptr)
			error("Failed to open DRILLE.EXE");

		load8bitBinary(file, 0x9b40, 16);
	} else if (_renderMode == "cga") {
		file = gameDir.createReadStreamForMember("DRILLC.EXE");

		if (file == nullptr)
			error("Failed to open DRILLC.EXE");
		load8bitBinary(file, 0x7bb0, 4);
	} else
		error("Invalid render mode %s for Driller", _renderMode.c_str());
}

void DrillerEngine::drawUI() {
	_gfx->renderCrossair(0);
	_gfx->setViewport(_fullscreenViewArea);

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];
	if (_renderMode == "ega" && _border) {
		//Common::Rect black(20, 177, 87, 191);
		//_gfx->drawRect2D(black, 255, 0, 0, 0);

		if (energy >= 0) {
			Common::Rect black(20, 186, 87 - energy, 191);
			_gfx->drawRect2D(black, 255, 0, 0, 0);
			Common::Rect energyBar(87 - energy, 186, 87, 191);
			_gfx->drawRect2D(energyBar, 255, 0xfc, 0xfc, 0x54);
		}

		if (shield >= 0) {
			Common::Rect black(20, 178, 87 - shield, 183);
			_gfx->drawRect2D(black, 255, 0, 0, 0);

			Common::Rect shieldBar(87 - shield, 178, 87, 183);
			_gfx->drawRect2D(shieldBar, 255, 0xfc, 0xfc, 0x54);
		}
	}
	_gfx->setViewport(_viewArea);
}


} // End of namespace Freescape