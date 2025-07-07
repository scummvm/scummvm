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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/onebuildpuzzle.h"

namespace Nancy {
namespace Action {

void OneBuildPuzzle::init() {
	// TODO
}

void OneBuildPuzzle::execute() {
	if (_state == kBegin) {
		init();
		registerGraphics();
		_state = kRun;
	}

	// TODO
	const uint16 sceneId = NancySceneState.getSceneInfo().sceneID;
	SceneChangeDescription scene;

	if (sceneId == 6519) {
		// Stub - move to the winning screen
		warning("STUB - Nancy 9 Pipe joining puzzle under sink");
		NancySceneState.setEventFlag(425, g_nancy->_true); // EV_Solved_Pipes
		scene.sceneID = 6520;
		NancySceneState.resetStateToInit();
		NancySceneState.changeScene(scene);
	} else if (sceneId == 2916) {
		// Stub - move to the winning screen
		warning("STUB - Nancy 9 Carborosaurus Puzzle");
		NancySceneState.setEventFlag(424, g_nancy->_true); // EV_Solved_Permit_Task
		scene.sceneID = 2915;
		NancySceneState.resetStateToInit();
		NancySceneState.changeScene(scene);
	} else {
		warning("STUB - Nancy 9 One Build Puzzle");
	}
}

void OneBuildPuzzle::readData(Common::SeekableReadStream &stream) {
	// TODO
	stream.skip(stream.size() - stream.pos());
}

void OneBuildPuzzle::handleInput(NancyInput &input) {
	// TODO
}

} // End of namespace Action
} // End of namespace Nancy
