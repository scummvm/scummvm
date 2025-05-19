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
#include "engines/nancy/action/puzzle/whalesurvivorpuzzle.h"

namespace Nancy {
namespace Action {

void WhaleSurvivorPuzzle::init() {
	// TODO
}

void WhaleSurvivorPuzzle::execute() {
	if (_state == kBegin) {
		init();
		registerGraphics();
		_state = kRun;
	}

	// TODO
	// Stub - move to the winning screen
	warning("STUB - Whale survivor puzzle");
	NancySceneState.setEventFlag(425, g_nancy->_true); // Set puzzle flag to solved
	SceneChangeDescription scene;
	scene.sceneID = 2990;
	NancySceneState.resetStateToInit();
	NancySceneState.changeScene(scene);
}

void WhaleSurvivorPuzzle::readData(Common::SeekableReadStream &stream) {
	// TODO
	stream.skip(stream.size() - stream.pos());
}

void WhaleSurvivorPuzzle::handleInput(NancyInput &input) {
	// TODO
}

} // End of namespace Action
} // End of namespace Nancy
