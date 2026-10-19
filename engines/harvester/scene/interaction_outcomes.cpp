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

#include "common/debug.h"
#include "harvester/detection.h"
#include "harvester/flow/flow.h"
#include "harvester/fst_player.h"
#include "harvester/harvester.h"
#include "harvester/player/player.h"

namespace Harvester {

void Interaction::preparePlayerForInteraction(const RoomPlayerState &playerState,
		bool interruptPlayerMovement) {
	if (interruptPlayerMovement) {
		_player.cancelMovement();
		_pendingRegionName.clear();
	} else if (playerState.hasMoveTarget || playerState.turnActive) {
		debugC(2, kDebugPlayer,
			"Harvester: background action preserving player movement room='%s' pos=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f) region='%s' turning=%d",
			_scene.state.roomName.c_str(), playerState.centerX, playerState.bottomY,
			playerState.z, playerState.targetX, playerState.targetBottomY,
			playerState.targetZ, _pendingRegionName.c_str(), playerState.turnActive);
	}
}

Common::Error Interaction::handleDemoEnding(bool &didTransition) {
	Common::Error endingError = _flow.runDemoEnding();
	if (endingError.getCode() != Common::kNoError)
		return endingError;
	didTransition = true;
	return Common::kNoError;
}

Common::Error Interaction::handleMainMenuReturn(const Common::String &deathFlicPath) {
	_engine.stopMusic();
	_engine.stopSound();
	if (!deathFlicPath.empty()) {
		FstPlayer fstPlayer(_engine);
		if (!fstPlayer.play(deathFlicPath))
			return Common::kReadingFailed;
	}

	if (!deathFlicPath.empty())
		_flow.requestGameOverReturn();
	else
		_flow.requestMainMenuReturn();
	return Common::kNoError;
}

void Interaction::applyPlayerDeathRequest(const InteractionResult &interaction) {
	if (interaction.requestPlayerDeath &&
			_engine.getScript() &&
			_engine.getScript()->getPlayerCurrentHitPoints() <= 0) {
		_callbacks.stopPlayerRegionInteraction();
		_callbacks.startPlayerDefeatSequence("script_kill_pc", Common::String(),
			interaction.playerDeathDamageType);
	}
}

bool Interaction::hasPendingMainMenuReturn() const {
	return _flow.hasPendingMainMenuReturn();
}

Common::Error Interaction::handleCombatInteraction(const InteractionResult &interaction, bool &needsRedraw) {
	bool didTransition = false;
	Common::Error interactionError =
		handleInteractionResult(interaction, didTransition, Common::String());
	if (interactionError.getCode() != Common::kNoError)
		return interactionError;
	if (!_flow.hasPendingMainMenuReturn() &&
			_engine.getScript() &&
			_engine.getScript()->getPlayerCurrentHitPoints() <= 0) {
		_callbacks.stopPlayerRegionInteraction();
		_callbacks.startPlayerDefeatSequence("combat_interaction", Common::String(), 1);
	}

	needsRedraw = true;
	return Common::kNoError;
}

} // End of namespace Harvester
