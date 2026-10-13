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

#ifndef HARVESTER_ROOM_SESSION_H
#define HARVESTER_ROOM_SESSION_H

#include "harvester/room_combat.h"
#include "harvester/room_input.h"
#include "harvester/room_interaction.h"
#include "harvester/room_presentation.h"
#include "harvester/room_scene_runtime.h"
#include "harvester/room_support.h"
#include "harvester/runtime_entity.h"

namespace Harvester {

class EntityManager;
class Flow;
class InventorySystem;

struct RoomPendingTransition {
	Common::String roomName;
	bool isRoomName = false;
	bool usesSavedRoomState = false;
	SaveRoomState savedRoomState;
};

// Scoped to one runRoomLoop iteration, including time spent in nested closeups.
// Entity pointers remain owned by EntityManager.
class RoomSession : public RoomInteractionCallbacks {
public:
	RoomSession(HarvesterEngine &engine, Flow &flow, RoomSceneResources &scene, EntityManager *entityManager,
		InventorySystem &inventory, CheatSystem &cheats, const RoomViewResources &view, bool canExitCloseupToParent);

	RoomPlayerState &getPlayerState() { return _playerState; }
	RoomIdleAnimationState &getIdleState() { return _idleState; }
	RoomInputState &getInputState() { return _inputState; }

	void initializePlayerState();
	RoomInputResult runRoomEntryInteraction(const InteractionResult &interaction);
	RoomInputResult runRoomFrames(const Common::String &currentRoomTarget);
	const RoomPendingTransition &getPendingTransition() const { return _pendingTransition; }
	void clearPendingRegionInteraction() { _pendingRegionName.clear(); }
	void capturePlayerAttackTarget(const Common::Point &mousePos, const NpcRecord *npc);
	Common::Error killDebugActiveMonster(const Common::Point &mousePos, bool &needsRedraw);

	void syncGlobalTimerEntities(const Common::Array<TimerRecord> &previousTimerRecords) override;
	bool captureDialogueBackdrop(IndexedBitmap &backdrop) override;
	Common::Error showCdChangePrompt(int discNumber) override;
	Common::Error runRoomExitCommands() override;
	Common::Error applyLightingCommand(StartupLightingCommand command) override;
	void applyPlayerGotoXZ(int x, int z) override;
	Common::Error runModalShowText(const ResolvedText &text) override;
	void startPlayerDefeatSequence(const char *reason, const Common::String &sourceName, int damageType) override;
	void resetIdleState() override;
	void notePlayerActivity() override;
	bool isPlayerCombatLocked() const;
	bool cancelPlayerAttackAnimation();
	void stopPlayerRegionInteraction() override;
	void syncPlayerPlacement();
	void captureCurrentSaveState();
	bool applyCurrentRoomRuntimeMutationsInPlace(bool preservePaletteState) override;
	bool refreshCurrentScene(bool preservePlayerPlacement) override;

private:
	struct PlayerControlState {
		bool alive;
		bool paused;
		bool canAct;
	};

	void syncAnimatedRoomActorPlacement();
	void requestPlayerGameOver(const char *reason, const Common::String &sourceName);
	void updateRoomInterface(bool &needsRedraw);
	bool drawRoomFrame(bool &needsRedraw);
	RoomInputResult pollRoomEvents(RoomInput &input, bool &needsRedraw);
	PlayerControlState preparePlayerControlState();
	Common::Error tryActivatePendingRegion(bool &needsRedraw);
	Common::Error tryActivatePassiveRegion(bool &needsRedraw);
	void advanceRuntimeEntities(bool &needsRedraw);
	Common::Error resolvePlayerAttackContact(bool &needsRedraw);
	void updatePlayerCombatAnimations(bool &needsRedraw);
	void updatePlayerControls(const PlayerControlState &control, bool &needsRedraw);
	Common::Error updateRoomMonsterCombat(bool &needsRedraw);
	void syncCombatHitEffects(bool &needsRedraw);
	void updatePlayerDeath(bool &needsRedraw);
	void pruneCombatEffects(bool &needsRedraw);
	RoomInputResult dispatchExpiredTimers(bool &needsRedraw);
	void updateIdleAnimation(bool &needsRedraw);

	HarvesterEngine &_engine;
	Flow &_flow;
	RoomSceneResources &_scene;
	EntityManager *_entityManager;
	InventorySystem &_inventory;
	RoomSceneRuntime _sceneRuntime;
	RoomPlayerState _playerState;
	RoomCombat _combat;
	RoomCombatEffects _combatEffects;
	RoomIdleAnimationState _idleState;
	RoomInputState _inputState;
	Common::String _pendingRegionName;
	CheatSystem &_cheats;
	const RoomViewResources _view;
	bool _canExitCloseupToParent;
	RoomPresentationState _presentationState;
	RoomPendingTransition _pendingTransition;
	RoomItemInteractions _roomItems;
	RoomPresentation _presentation;
	RoomInteractionProcessor _interactionProcessor;
};

} // End of namespace Harvester

#endif // HARVESTER_ROOM_SESSION_H
