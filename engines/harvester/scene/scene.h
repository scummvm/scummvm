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

#ifndef HARVESTER_SCENE_H
#define HARVESTER_SCENE_H

#include "common/error.h"
#include "common/str.h"
#include "harvester/combat/combat.h"
#include "harvester/scene/input.h"
#include "harvester/player/player.h"
#include "harvester/scene/interaction.h"
#include "harvester/scene/room_presentation.h"
#include "harvester/scene/scene_entities.h"
#include "harvester/scene/room_support.h"
#include "harvester/entity.h"

namespace Harvester {

class EntityManager;
class Flow;
class InventorySystem;

class SceneRunner {
public:
	SceneRunner(HarvesterEngine &engine, InventorySystem &inventory,
		CheatSystem &cheats);

	Common::Error runRoomLoop(Flow &flow, const Common::String &targetName, bool targetIsRoomName);

private:
	HarvesterEngine &_engine;
	InventorySystem &_inventory;
	CheatSystem &_cheats;
};

struct RoomPendingTransition {
	Common::String roomName;
	bool isRoomName = false;
	bool usesSavedRoomState = false;
	SaveRoomState savedRoomState;
};

class Scene : public InteractionCallbacks {
public:
	Scene(HarvesterEngine &engine, Flow &flow, SceneState &scene, EntityManager *entityManager,
		InventorySystem &inventory, CheatSystem &cheats, const RoomViewResources &view, bool canExitCloseupToParent);

	Player &getPlayer() { return _player; }
	InputState &getInputState() { return _inputState; }

	void initializePlayerState();
	InputResult runRoomEntryInteraction(const InteractionResult &interaction);
	InputResult runRoomFrames(const Common::String &currentRoomTarget);
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
	InputResult drawRoomFrame(bool &needsRedraw);
	InputResult pollRoomEvents(Input &input, bool &needsRedraw);
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
	InputResult dispatchExpiredTimers(bool &needsRedraw);
	void updateIdleAnimation(bool &needsRedraw);

	HarvesterEngine &_engine;
	Flow &_flow;
	SceneState &_scene;
	EntityManager *_entityManager;
	InventorySystem &_inventory;
	SceneEntities _sceneRuntime;
	Player _player;
	CombatEffects _combatEffects;
	InputState _inputState;
	Common::String _pendingRegionName;
	CheatSystem &_cheats;
	const RoomViewResources _view;
	bool _canExitCloseupToParent;
	RoomPresentationState _presentationState;
	RoomPendingTransition _pendingTransition;
	ItemInteractions _roomItems;
	RoomPresentation _presentation;
	Interaction _interactionProcessor;
	Combat _combat;
};

} // End of namespace Harvester

#endif // HARVESTER_SCENE_H
