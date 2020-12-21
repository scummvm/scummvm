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

#include "twine/debugger/debug_grid.h"
#include "common/debug.h"
#include "twine/scene/grid.h"
#include "twine/input.h"
#include "twine/renderer/redraw.h"
#include "twine/scene/scene.h"
#include "twine/twine.h"

namespace TwinE {

DebugGrid::DebugGrid(TwinEEngine *engine) : _engine(engine) {
	canChangeScenes = _engine->cfgfile.Debug;
}

void DebugGrid::changeGridCamera() {
	if (!useFreeCamera) {
		return;
	}

	if (_engine->_input->isActionActive(TwinEActionType::DebugGridCameraPressUp)) {
		_engine->_grid->newCameraZ--;
		_engine->_redraw->reqBgRedraw = true;
	} else if (_engine->_input->isActionActive(TwinEActionType::DebugGridCameraPressDown)) {
		_engine->_grid->newCameraZ++;
		_engine->_redraw->reqBgRedraw = true;
	}
	if (_engine->_input->isActionActive(TwinEActionType::DebugGridCameraPressLeft)) {
		_engine->_grid->newCameraX--;
		_engine->_redraw->reqBgRedraw = true;
	} else if (_engine->_input->isActionActive(TwinEActionType::DebugGridCameraPressRight)) {
		_engine->_grid->newCameraX++;
		_engine->_redraw->reqBgRedraw = true;
	}
}

void DebugGrid::changeGrid() {
	if (!canChangeScenes) {
		return;
	}
	if (_engine->_input->toggleActionIfActive(TwinEActionType::NextRoom)) {
		_engine->_scene->currentSceneIdx++;
		if (_engine->_scene->currentSceneIdx >= LBA1SceneId::SceneIdMax) {
			_engine->_scene->currentSceneIdx = LBA1SceneId::Citadel_Island_Prison;
		}
		_engine->_scene->needChangeScene = _engine->_scene->currentSceneIdx;
		_engine->_redraw->reqBgRedraw = true;
	}

	if (_engine->_input->toggleActionIfActive(TwinEActionType::PreviousRoom)) {
		_engine->_scene->currentSceneIdx--;
		if (_engine->_scene->currentSceneIdx < LBA1SceneId::Citadel_Island_Prison) {
			_engine->_scene->currentSceneIdx = LBA1SceneId::SceneIdMax - 1;
		}
		_engine->_scene->needChangeScene = _engine->_scene->currentSceneIdx;
		_engine->_redraw->reqBgRedraw = true;
	}
}

void DebugGrid::applyCellingGrid() {
	// Increase celling grid index
	if (_engine->_input->toggleActionIfActive(TwinEActionType::IncreaseCellingGridIndex)) {
		_engine->_grid->cellingGridIdx++;
		if (_engine->_grid->cellingGridIdx > 133) {
			_engine->_grid->cellingGridIdx = 133;
		}
	}
	// Decrease celling grid index
	else if (_engine->_input->toggleActionIfActive(TwinEActionType::DecreaseCellingGridIndex)) {
		_engine->_grid->cellingGridIdx--;
		if (_engine->_grid->cellingGridIdx < 0) {
			_engine->_grid->cellingGridIdx = 0;
		}
	}
	// Enable/disable celling grid
	else if (_engine->_input->toggleActionIfActive(TwinEActionType::ApplyCellingGrid)) {
		if (_engine->_grid->useCellingGrid == -1) {
			_engine->_grid->useCellingGrid = 1;
			//createGridMap();
			_engine->_grid->initCellingGrid(_engine->_grid->cellingGridIdx);
			debug("Enable Celling Grid index: %d", _engine->_grid->cellingGridIdx);
			_engine->_scene->needChangeScene = -2; // tricky to make the fade
		} else if (_engine->_grid->useCellingGrid == 1) {
			_engine->_grid->useCellingGrid = -1;
			_engine->_grid->createGridMap();
			_engine->_redraw->reqBgRedraw = true;
			debug("Disable Celling Grid index: %d", _engine->_grid->cellingGridIdx);
			_engine->_scene->needChangeScene = -2; // tricky to make the fade
		}
	}
}

} // namespace TwinE
