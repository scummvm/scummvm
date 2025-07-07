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
#include "engines/nancy/action/puzzle/multibuildpuzzle.h"

namespace Nancy {
namespace Action {

void MultiBuildPuzzle::init() {
	// TODO
}

void MultiBuildPuzzle::execute() {
	if (_state == kBegin) {
		init();
		registerGraphics();
		_state = kRun;
	}

	// TODO
	// Stub - move to the winning screen

	SceneChangeDescription scene;
	uint16 sceneID = NancySceneState.getSceneInfo().sceneID;

	switch (sceneID) {
	case 2025:
		warning("STUB - Nancy 9 Sand castle puzzle");
		scene.sceneID = 2024;
		break;
	case 2575:
		warning("STUB - Nancy 9 Sandwich making puzzle");
		NancySceneState.setEventFlag(428, g_nancy->_true); // EV_Solved_Sandwich_Bad
		NancySceneState.setEventFlag(429, g_nancy->_true); // EV_Solved_Sandwich_Good
		scene.sceneID = 2572;
		break;
	case 2585:
		warning("STUB - Nancy 9 Book sorting puzzle");
		NancySceneState.setEventFlag(397, g_nancy->_true); // Set puzzle flag to solved
		scene.sceneID = 2583;
		break;
	default:
		warning("MultiBuildPuzzle: Unknown scene %d", sceneID);
		return;
	}

	NancySceneState.resetStateToInit();
	NancySceneState.changeScene(scene);
}

void MultiBuildPuzzle::readData(Common::SeekableReadStream &stream) {
	// TODO
	stream.skip(stream.size() - stream.pos());
}

void MultiBuildPuzzle::handleInput(NancyInput &input) {
	// TODO
}

} // End of namespace Action
} // End of namespace Nancy
