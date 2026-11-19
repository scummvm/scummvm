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

#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/scene.h"

namespace Hollywood {

void RunawayRoomScene::startDesert() {
	_entering = true;
	resumeDesert();
}

void RunawayRoomScene::resumeDesert() {
	resumeAmbientAudio(_entering);
	if (!_entering) {
		return;
	}
	_entering = false;
	runCustomEntrySequence();
}

void RunawayRoomScene::visitDesertRoom(byte index, byte lines) {
	assert(index < ARRAYSIZE(_desert.visited));
	if (_desert.visited[index]) {
		return;
	}
	_desert.visited[index] = 1;
	for (byte i = 0; i < lines; ++i) {
		say(0, i);
	}
}

void RunawayRoomScene::updateDesert(uint elapsed) {
	if (updateCustomSequence()) {
		return;
	}
	updateRoomAnimation();
	updateConversation();
	advanceCustomGameplayLoop(elapsed);
	if (_roomAction && _actionFrames.empty() && !speaking() && !_walkPath.active()) {
		const uint16 action = _roomAction;
		_roomAction = 0;
		execute(action);
	}
	if (_hospitalExit.scene && !speaking() && !_roomAction && !_walkPath.active()) {
		_transition = _hospitalExit;
	}
}

void RunawayRoomScene::leaveDesertRoom(uint16 scene) {
	const RunawayRoom *room = findRunawayRoom(scene);
	if (!room && (scene < 5030 || scene > 5032)) {
		warning("Runaway: scene %u is not implemented yet", scene);
		return;
	}
	stopAmbientSpeech();
	_hospitalExit.scene = scene;
	_hospitalExit.position = room ? room->entry : Common::Point(1167, 468);
	_hospitalExit.facing = 3;
}

void RunawayRoomScene::deferDesertAction(uint16 action) {
	_roomAction = action;
	_actionFrames.clear();
}

} // End of namespace Hollywood
