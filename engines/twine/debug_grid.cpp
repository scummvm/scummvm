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

#include "twine/debug_grid.h"
#include "common/debug.h"
#include "twine/grid.h"
#include "twine/keyboard.h"
#include "twine/redraw.h"
#include "twine/scene.h"
#include "twine/twine.h"

namespace TwinE {

DebugGrid::DebugGrid(TwinEEngine *engine) : _engine(engine) {
	canChangeScenes = _engine->cfgfile.Debug ? 1 : 0;
}

void DebugGrid::changeGridCamera(int16 pKey) {
	if (useFreeCamera) {
		// Press up - more X positions
		if (pKey == Keys::DebugGridCameraPressUp) {
			_engine->_grid->newCameraZ--;
			_engine->_redraw->reqBgRedraw = 1;
		}

		// Press down - less X positions
		else if (pKey == Keys::DebugGridCameraPressDown) {
			_engine->_grid->newCameraZ++;
			_engine->_redraw->reqBgRedraw = 1;
		}

		// Press left - less Z positions
		else if (pKey == Keys::DebugGridCameraPressLeft) {
			_engine->_grid->newCameraX--;
			_engine->_redraw->reqBgRedraw = 1;
		}

		// Press right - more Z positions
		else if (pKey == Keys::DebugGridCameraPressRight) {
			_engine->_grid->newCameraX++;
			_engine->_redraw->reqBgRedraw = 1;
		}
	}
}

void DebugGrid::changeGrid(int16 pKey) {
	if (!canChangeScenes) {
		return;
	}
	// Press up - more X positions
	if (pKey == Keys::NextRoom) {
		_engine->_scene->currentSceneIdx++;
		if (_engine->_scene->currentSceneIdx > NUM_SCENES)
			_engine->_scene->currentSceneIdx = 0;
		_engine->_scene->needChangeScene = _engine->_scene->currentSceneIdx;
		_engine->_redraw->reqBgRedraw = 1;
	}

	// Press down - less X positions
	if (pKey == Keys::PreviousRoom) {
		_engine->_scene->currentSceneIdx--;
		if (_engine->_scene->currentSceneIdx < 0)
			_engine->_scene->currentSceneIdx = NUM_SCENES;
		_engine->_scene->needChangeScene = _engine->_scene->currentSceneIdx;
		_engine->_redraw->reqBgRedraw = 1;
	}

	if (_engine->cfgfile.Debug && (pKey == 'f' || pKey == 'r')) {
		debug("Grid index changed: %d", _engine->_scene->needChangeScene);
	}
}

void DebugGrid::applyCellingGrid(int16 pKey) {
	// Increase celling grid index
	if (pKey == Keys::IncreaseCellingGridIndex) {
		_engine->_grid->cellingGridIdx++;
		if (_engine->_grid->cellingGridIdx > 133)
			_engine->_grid->cellingGridIdx = 133;
	}
	// Decrease celling grid index
	else if (pKey == Keys::DecreaseCellingGridIndex) {
		_engine->_grid->cellingGridIdx--;
		if (_engine->_grid->cellingGridIdx < 0)
			_engine->_grid->cellingGridIdx = 0;
	}
	// Enable/disable celling grid
	else if (pKey == Keys::ApplyCellingGrid) {
		if (_engine->_grid->useCellingGrid == -1) {
			_engine->_grid->useCellingGrid = 1;
			//createGridMap();
			_engine->_grid->initCellingGrid(_engine->_grid->cellingGridIdx);
			debug("Enable Celling Grid index: %d", _engine->_grid->cellingGridIdx);
			_engine->_scene->needChangeScene = -2; // tricky to make the fade
		} else if (_engine->_grid->useCellingGrid == 1) {
			_engine->_grid->useCellingGrid = -1;
			_engine->_grid->createGridMap();
			_engine->_redraw->reqBgRedraw = 1;
			debug("Disable Celling Grid index: %d", _engine->_grid->cellingGridIdx);
			_engine->_scene->needChangeScene = -2; // tricky to make the fade
		}
	}
}

} // namespace TwinE
