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

void RunawayRoomScene::resumeTown() {
	prepareCustomGameplayLoop();
	if (!_entering) {
		resumeAmbientAudio(false);
		return;
	}
	_entering = false;
	_town.arrived = 1;
	resumeAmbientAudio(true);
	runCustomEntrySequence();
}

void RunawayRoomScene::updateTown(uint32 delta) {
	advanceCustomGameplayLoop(delta);
	if (updateCustomSequence()) {
		return;
	}
	updateRoomAnimation();
	updateConversation();
	if (_roomAction && _actionFrames.empty() && !speaking() && !_walkPath.active()) {
		const uint16 action = _roomAction;
		_roomAction = 0;
		execute(action);
	}
	if (_hospitalExit.scene && !speaking() && !_roomAction && !_walkPath.active()) {
		_transition = _hospitalExit;
	}
}

void RunawayRoomScene::walkTown(Common::Point target) {
	if (!_pendingHotspot) {
		// Clicks stay on the ground; hotspot approaches may use the reserved zones.
		target.x = CLIP<int>(target.x, 0, _background.w - 1);
		target.y = CLIP<int>(target.y, 0, _background.h - 1);
		if (!adjustCustomWalkTargetToFloorMask(target)) {
			while (target.y < _background.h - 1 && !_maps[_mask[target.y * _background.w + target.x]]) {
				++target.y;
			}
			while (target.y > 0 && !_maps[_mask[target.y * _background.w + target.x]]) {
				--target.y;
			}
		}
	}
	_walkPath.findPath(RunawayWalkMap(_background.w, _background.h, _mask, _originalMaps), target);
}

void RunawayRoomScene::deferTownAction(uint16 action) {
	_roomAction = action;
	_actionFrames.clear();
}

void RunawayRoomScene::leaveTownRoom(uint16 scene) {
	const RunawayRoom *room = findRunawayRoom(scene);
	if (!room) {
		warning("Runaway: scene %u is not implemented yet", scene);
		return;
	}
	_hospitalExit.scene = scene;
	_hospitalExit.position = room->entry;
	_hospitalExit.facing = 3;
}

} // End of namespace Hollywood
