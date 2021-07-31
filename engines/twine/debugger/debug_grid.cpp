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
	_canChangeScenes = _engine->_cfgfile.Debug;
}

void DebugGrid::changeGridCamera() {
	if (!_useFreeCamera) {
		return;
	}

	Grid *grid = _engine->_grid;
	Redraw *redraw = _engine->_redraw;
	Input *input = _engine->_input;
	if (input->isActionActive(TwinEActionType::DebugGridCameraPressUp)) {
		grid->newCamera.z--;
		redraw->_reqBgRedraw = true;
	} else if (input->isActionActive(TwinEActionType::DebugGridCameraPressDown)) {
		grid->newCamera.z++;
		redraw->_reqBgRedraw = true;
	}
	if (input->isActionActive(TwinEActionType::DebugGridCameraPressLeft)) {
		grid->newCamera.x--;
		redraw->_reqBgRedraw = true;
	} else if (input->isActionActive(TwinEActionType::DebugGridCameraPressRight)) {
		grid->newCamera.x++;
		redraw->_reqBgRedraw = true;
	}
}

void DebugGrid::changeGrid() {
	if (!_canChangeScenes) {
		return;
	}
	Scene *scene = _engine->_scene;
	Redraw *redraw = _engine->_redraw;
	Input *input = _engine->_input;
	if (input->toggleActionIfActive(TwinEActionType::NextRoom)) {
		scene->currentSceneIdx++;
		if (scene->currentSceneIdx >= LBA1SceneId::SceneIdMax) {
			scene->currentSceneIdx = LBA1SceneId::Citadel_Island_Prison;
		}
		scene->needChangeScene = scene->currentSceneIdx;
		redraw->_reqBgRedraw = true;
	}

	if (input->toggleActionIfActive(TwinEActionType::PreviousRoom)) {
		scene->currentSceneIdx--;
		if (scene->currentSceneIdx < LBA1SceneId::Citadel_Island_Prison) {
			scene->currentSceneIdx = LBA1SceneId::SceneIdMax - 1;
		}
		scene->needChangeScene = scene->currentSceneIdx;
		redraw->_reqBgRedraw = true;
	}
}

void DebugGrid::applyCellingGrid() {
	Grid *grid = _engine->_grid;
	Input *input = _engine->_input;
	// Increase celling grid index
	if (input->toggleActionIfActive(TwinEActionType::IncreaseCellingGridIndex)) {
		grid->cellingGridIdx++;
		if (grid->cellingGridIdx > 133) {
			grid->cellingGridIdx = 133;
		}
	}
	// Decrease celling grid index
	else if (input->toggleActionIfActive(TwinEActionType::DecreaseCellingGridIndex)) {
		grid->cellingGridIdx--;
		if (grid->cellingGridIdx < 0) {
			grid->cellingGridIdx = 0;
		}
	}
	// Enable/disable celling grid
	else if (input->toggleActionIfActive(TwinEActionType::ApplyCellingGrid)) {
		if (grid->useCellingGrid == -1) {
			grid->useCellingGrid = 1;
			//grid->createGridMap();
			grid->initCellingGrid(grid->cellingGridIdx);
			debug("Enable Celling Grid index: %d", grid->cellingGridIdx);
			_engine->_scene->needChangeScene = SCENE_CEILING_GRID_FADE_2; // tricky to make the fade
		} else if (grid->useCellingGrid == 1) {
			grid->useCellingGrid = -1;
			grid->createGridMap();
			_engine->_redraw->_reqBgRedraw = true;
			debug("Disable Celling Grid index: %d", grid->cellingGridIdx);
			_engine->_scene->needChangeScene = SCENE_CEILING_GRID_FADE_2; // tricky to make the fade
		}
	}
}

} // namespace TwinE
