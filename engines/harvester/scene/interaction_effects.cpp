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
#include "common/textconsole.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/flow/flow.h"
#include "harvester/fst_player.h"
#include "harvester/harvester.h"
#include "harvester/player/player.h"

namespace Harvester {

Common::Error Interaction::applySameSceneEffects(const InteractionResult &interaction,
		const RoomPlayerState &playerState, bool &didTransition, const Common::String &usedItemName,
		bool interruptPlayerMovement) {
	applyPlayerDeathRequest(interaction);

	if (interaction.requestPlayerGotoXZ)
		_callbacks.applyPlayerGotoXZ(interaction.playerGotoX, interaction.playerGotoZ);
	else if (interaction.requestPlayerGotoZ)
		_callbacks.applyPlayerGotoXZ(playerState.centerX, interaction.playerGotoZ);

	if (!interaction.moveEntityToPlayerZName.empty()) {
		EntityManager *entityManager = _engine.getRuntimeEntities();
		Entity *entity = entityManager
			? entityManager->findSceneEntityByName(interaction.moveEntityToPlayerZName)
			: nullptr;
		if (entity && playerState.entity) {
			entity->setPosition(entity->getX(), entity->getY(), playerState.entity->getZ());
			entityManager->reinsertSceneEntity(entity);
			debugC(1, kDebugPlayer, "Harvester: MOVE_BM2PCZ entity='%s' z=%.2f",
				interaction.moveEntityToPlayerZName.c_str(), entity->getZ());
		}
	}

	if (interaction.lightingCommand != kStartupLightingCommandNone) {
		Common::Error lightingError = _callbacks.applyLightingCommand(interaction.lightingCommand);
		if (lightingError.getCode() != Common::kNoError)
			return lightingError;
	}

	if (!interaction.cutscenePath.empty()) {
		FstPlayer fstPlayer(_engine);
		if (!fstPlayer.play(interaction.cutscenePath))
			return Common::kReadingFailed;
	}

	if (!interaction.modalText.value.empty()) {
		Common::Error modalError = _callbacks.runModalShowText(interaction.modalText);
		if (modalError.getCode() != Common::kNoError)
			return modalError;
	}

	if (!interaction.dialogueNpcName.empty()) {
		Common::Error dialogueError = runScriptedDialogue(
			interaction.dialogueNpcName, usedItemName, interaction.dialogueContinuationTag,
			didTransition);
		if (dialogueError.getCode() != Common::kNoError)
			return dialogueError;
	}

	if (didTransition)
		return Common::kNoError;

	if (!interaction.continuationTag.empty()) {
		InteractionResult continuationInteraction;
		if (_engine.getScript()->executeActionTag(
				interaction.continuationTag, continuationInteraction, true,
				_scene.executionContext())) {
			Common::Error interactionError =
				handleInteractionResult(continuationInteraction, didTransition, usedItemName, interruptPlayerMovement);
			if (interactionError.getCode() != Common::kNoError)
				return interactionError;
		}
	}

	return Common::kNoError;
}

Common::Error Interaction::runScriptedDialogue(const Common::String &npcName,
		const Common::String &usedItemName, const Common::String &continuationTag,
		bool &didTransition) {
	didTransition = false;

	const NpcRecord *dialogueNpc = _engine.getScript()->findRuntimeNpcRecord(npcName);
	if (dialogueNpc) {
		IndexedBitmap dialogueBackdrop;
		if (!_callbacks.captureDialogueBackdrop(dialogueBackdrop))
			return Common::kReadingFailed;

		Common::Error dialogueError = _flow.runRoomNpcDialogue(
			dialogueBackdrop, _scene.palette, _scene.targetPaletteBrightness,
			_scene.executionContext(), *dialogueNpc, usedItemName);
		if (dialogueError.getCode() != Common::kNoError)
			return dialogueError;
	} else {
		warning("Harvester: unresolved startup dialogue npc '%s' while processing room dialogue",
			npcName.c_str());
	}

	InteractionResult dialogueInteraction;
	bool abortRemainingCommandChain = false;
	if (_flow.takeQueuedDialogueInteraction(dialogueInteraction)) {
		abortRemainingCommandChain = dialogueInteraction.abortRemainingCommandChain;
		Common::Error interactionError =
			handleInteractionResult(dialogueInteraction, didTransition, usedItemName);
		if (interactionError.getCode() != Common::kNoError)
			return interactionError;
		if (_flow.hasPendingMainMenuReturn())
			return Common::kNoError;
	}
	if (!didTransition && !abortRemainingCommandChain && !continuationTag.empty()) {
		InteractionResult continuationInteraction;
		if (_engine.getScript()->executeActionTag(
				continuationTag, continuationInteraction, true, _scene.executionContext())) {
			Common::Error interactionError =
				handleInteractionResult(continuationInteraction, didTransition, usedItemName);
			if (interactionError.getCode() != Common::kNoError)
				return interactionError;
			if (_flow.hasPendingMainMenuReturn())
				return Common::kNoError;
		}
	}

	_flow.resetCursorAnimationSequence();
	_callbacks.resetIdleState();
	return Common::kNoError;
}

} // End of namespace Harvester
