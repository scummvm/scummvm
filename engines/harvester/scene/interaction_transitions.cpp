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

#include "harvester/scene/interaction.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "harvester/detection.h"
#include "harvester/flow/flow.h"
#include "harvester/harvester.h"

namespace Harvester {

Common::Error Interaction::queueImplicitRoomRestart(const InteractionResult &interaction,
		bool discChanged, bool &didTransition) {
	_pendingRoomChangeIsRoomName = false;
	_pendingRoomChangeUsesSavedRoomState = false;
	_pendingRoomChangeSavedRoomState.clear();
	if (!interaction.roomRestartTargetName.empty()) {
		_pendingRoomChange = interaction.roomRestartTargetName;
	} else if (discChanged && interaction.cdChangeDisc == 3) {
		// Disc-3 prompt reloads re-enter room_setup through the startup target
		// buffer, which remains "START" during ordinary play and resolves to CD3 RECEPTION.
		_pendingRoomChange = "START";
	} else if (!_scene.state.entranceName.empty()) {
		_pendingRoomChange = _scene.state.entranceName;
	} else {
		_pendingRoomChange = _scene.state.roomName;
		_pendingRoomChangeIsRoomName = true;
	}
	didTransition = !_pendingRoomChange.empty();
	debugC(1, kDebugRoom,
		"Harvester: implicit room restart target='%s' explicit='%s' discChanged=%d cdChangeDisc=%d sceneEntrance='%s' sceneRoom='%s'",
		_pendingRoomChange.c_str(), interaction.roomRestartTargetName.c_str(),
		discChanged ? 1 : 0, interaction.cdChangeDisc,
		_scene.state.entranceName.c_str(), _scene.state.roomName.c_str());
	return didTransition ? Common::kNoError : Common::kReadingFailed;
}

Common::Error Interaction::queueSavedRoomRestart(const SaveRoomState &savedState, bool &didTransition) {
	_pendingRoomChange.clear();
	_pendingRoomChangeIsRoomName = false;
	_pendingRoomChangeUsesSavedRoomState = false;
	_pendingRoomChangeSavedRoomState.clear();
	_pendingRoomChange = !savedState.entranceName.empty()
		? savedState.entranceName
		: savedState.roomName;
	_pendingRoomChangeIsRoomName = savedState.entranceName.empty();
	_pendingRoomChangeUsesSavedRoomState = !_pendingRoomChange.empty();
	_pendingRoomChangeSavedRoomState = savedState;
	debugC(1, kDebugRoom,
		"Harvester: queued saved room restart target='%s' room='%s' entrance='%s' spawn=(%d,%d,%d) facing=%d music='%s'",
		_pendingRoomChange.c_str(), savedState.roomName.c_str(),
		savedState.entranceName.c_str(), savedState.playerX, savedState.playerY,
		savedState.playerZ, savedState.playerFacing, savedState.musicPath.c_str());
	didTransition = !_pendingRoomChange.empty();
	return didTransition ? Common::kNoError : Common::kReadingFailed;
}

Common::Error Interaction::queueRoomChange(const Common::String &nextRoomName,
		bool &didTransition) {
	Common::String resolvedTransitionTarget = nextRoomName;
	Common::Error resolveError = _flow.resolveRoomTransitionTarget(
		nextRoomName, resolvedTransitionTarget);
	if (resolveError.getCode() != Common::kNoError)
		return resolveError;
	if (resolvedTransitionTarget.empty())
		return Common::kNoError;

	Common::Error exitError = _callbacks.runRoomExitCommands();
	if (exitError.getCode() != Common::kNoError)
		return exitError;

	// CHANGE_ROOM queues a room handoff for the live loop instead of nesting.
	_pendingRoomChange = Common::move(resolvedTransitionTarget);
	didTransition = true;
	return Common::kNoError;
}

Common::Error Interaction::runCloseupInteraction(const InteractionResult &interaction,
		bool discChanged, const Common::String &restoreMusicPath, bool &didTransition) {
	Common::Error exitError = _callbacks.runRoomExitCommands();
	if (exitError.getCode() != Common::kNoError)
		return exitError;

	SaveRoomState parentRoomState;
	const bool hasParentRoomState = _engine.hasCurrentSaveRoomState();
	if (hasParentRoomState)
		parentRoomState = _engine.getCurrentSaveRoomState();
	Common::Error roomError = _flow.runRoomLoop(interaction.nextRoomName);
	if (_flow.hasPendingMainMenuReturn())
		return Common::kNoError;
	if (_flow.takePendingCloseupParentRestart()) {
		if (hasParentRoomState && parentRoomState.valid)
			return queueSavedRoomRestart(parentRoomState, didTransition);
		return queueImplicitRoomRestart(interaction, discChanged, didTransition);
	}
	if (_flow.hasPendingDebugRoomChange()) {
		didTransition = true;
		return Common::kNoError;
	}
	if (roomError.getCode() != Common::kReadingFailed &&
			roomError.getCode() != Common::kNoError) {
		return roomError;
	}

	if (!_callbacks.refreshCurrentScene(true))
		return Common::kReadingFailed;

	_flow.executeStartupAudioCommands(_scene.state.audioCommands);
	if (!restoreMusicPath.empty())
		(void)_engine.playMusic(restoreMusicPath);
	else
		_engine.stopMusic();
	didTransition = true;
	return Common::kNoError;
}

Common::Error Interaction::queueCloseupExit(bool &didTransition) {
	if (!_canExitCloseupToParent) {
		debugC(1, kDebugRoom,
			"Harvester: EXIT_CLOSEUP ignored for room='%s' without parent room loop",
			_scene.state.roomName.c_str());
		return Common::kNoError;
	}

	_pendingRoomChange = kExitCloseupPendingRoomChange;
	didTransition = true;
	return Common::kNoError;
}

} // End of namespace Harvester
