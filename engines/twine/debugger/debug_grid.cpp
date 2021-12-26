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
		grid->_newCamera.z--;
		redraw->_reqBgRedraw = true;
	} else if (input->isActionActive(TwinEActionType::DebugGridCameraPressDown)) {
		grid->_newCamera.z++;
		redraw->_reqBgRedraw = true;
	}
	if (input->isActionActive(TwinEActionType::DebugGridCameraPressLeft)) {
		grid->_newCamera.x--;
		redraw->_reqBgRedraw = true;
	} else if (input->isActionActive(TwinEActionType::DebugGridCameraPressRight)) {
		grid->_newCamera.x++;
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
		scene->_currentSceneIdx++;
		if (scene->_currentSceneIdx >= LBA1SceneId::SceneIdMax) {
			scene->_currentSceneIdx = LBA1SceneId::Citadel_Island_Prison;
		}
		scene->_needChangeScene = scene->_currentSceneIdx;
		redraw->_reqBgRedraw = true;
	}

	if (input->toggleActionIfActive(TwinEActionType::PreviousRoom)) {
		scene->_currentSceneIdx--;
		if (scene->_currentSceneIdx < LBA1SceneId::Citadel_Island_Prison) {
			scene->_currentSceneIdx = LBA1SceneId::SceneIdMax - 1;
		}
		scene->_needChangeScene = scene->_currentSceneIdx;
		redraw->_reqBgRedraw = true;
	}
}

void DebugGrid::applyCellingGrid() {
	Grid *grid = _engine->_grid;
	Input *input = _engine->_input;
	// Increase celling grid index
	if (input->toggleActionIfActive(TwinEActionType::IncreaseCellingGridIndex)) {
		grid->_cellingGridIdx++;
		if (grid->_cellingGridIdx > 133) {
			grid->_cellingGridIdx = 133;
		}
	}
	// Decrease celling grid index
	else if (input->toggleActionIfActive(TwinEActionType::DecreaseCellingGridIndex)) {
		grid->_cellingGridIdx--;
		if (grid->_cellingGridIdx < 0) {
			grid->_cellingGridIdx = 0;
		}
	}
	// Enable/disable celling grid
	else if (input->toggleActionIfActive(TwinEActionType::ApplyCellingGrid)) {
		if (grid->_useCellingGrid == -1) {
			grid->_useCellingGrid = 1;
			//grid->createGridMap();
			grid->initCellingGrid(grid->_cellingGridIdx);
			debug("Enable Celling Grid index: %d", grid->_cellingGridIdx);
			_engine->_scene->_needChangeScene = SCENE_CEILING_GRID_FADE_2; // tricky to make the fade
		} else if (grid->_useCellingGrid == 1) {
			grid->_useCellingGrid = -1;
			grid->createGridMap();
			_engine->_redraw->_reqBgRedraw = true;
			debug("Disable Celling Grid index: %d", grid->_cellingGridIdx);
			_engine->_scene->_needChangeScene = SCENE_CEILING_GRID_FADE_2; // tricky to make the fade
		}
	}
}

} // namespace TwinE
