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
#include "engines/nancy/action/puzzle/quizpuzzle.h"

namespace Nancy {
namespace Action {

void QuizPuzzle::init() {
	// TODO
}

void QuizPuzzle::execute() {
	// TODO

	if (g_nancy->getGameType() == kGameTypeNancy8) {
		warning("STUB - Nancy 8 Quiz Puzzle");
	} else if (g_nancy->getGameType() == kGameTypeNancy9) {
		const uint16 sceneId = NancySceneState.getSceneInfo().sceneID;
		if (sceneId == 6450) {
			warning("STUB - Nancy 9 Quiz Puzzle - Holt Scotto's quiz");
			// Set the puzzle event flags to flag it as done
			NancySceneState.setEventFlag(59, g_nancy->_true); // EV_Answered_SQ_Q06
			NancySceneState.setEventFlag(61, g_nancy->_true); // EV_Answered_SQ_Q09
		} else if (sceneId == 6342) {
			warning("STUB - Nancy 9 Quiz Puzzle - GPS new waypoint");
			// Set the GPS waypoint as discovered
			NancySceneState.setEventFlag(410, g_nancy->_true);	// EV_Solved_GPS_Beach
		} else if (sceneId == 6344) {
			warning("STUB - Nancy 9 Quiz Puzzle - GPS new waypoint - cache A");
			// Set the GPS waypoint as discovered
			NancySceneState.setEventFlag(411, g_nancy->_true); // EV_Solved_GPS_CacheA
		} else if (sceneId == 6345) {
			warning("STUB - Nancy 9 Quiz Puzzle - GPS new waypoint - cache B");
			// Set the GPS waypoint as discovered
			NancySceneState.setEventFlag(412, g_nancy->_true); // EV_Solved_GPS_CacheB
		} else if (sceneId == 6431) {
			warning("STUB - Nancy 9 Quiz Puzzle - Hilda Swenson's letter");
			NancySceneState.setEventFlag(179, g_nancy->_true); // EV_Hilda_Said_Objects
		} else if (sceneId == 6443) {
			warning("STUB - Nancy 9 Quiz Puzzle - Holt Scotto's chess problem");
			NancySceneState.setEventFlag(119, g_nancy->_true);	// EV_Finished_Chess_Quiz
		} else if (sceneId == 4184) {
			warning("STUB - Nancy 9 Quiz Puzzle - Lighthouse Morse code");
			SceneChangeDescription scene;
			scene.sceneID = 4190;
			NancySceneState.resetStateToInit();
			NancySceneState.changeScene(scene);
		} else {
			warning("STUB - Nancy 9 Quiz Puzzle");
		}
	}

	_isDone = true;
}

void QuizPuzzle::readData(Common::SeekableReadStream &stream) {
	// TODO
	stream.skip(stream.size() - stream.pos());
}

void QuizPuzzle::handleInput(NancyInput &input) {
	// TODO
}

} // End of namespace Action
} // End of namespace Nancy
