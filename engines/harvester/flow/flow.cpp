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

#include "harvester/flow/flow.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/memstream.h"
#include "common/serializer.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/scene/input.h"
#include "harvester/entity.h"
#include "harvester/resources.h"
#include "harvester/script/script.h"

namespace Harvester {

namespace {

static const int kCursorSequence7 = 7;

} // End of anonymous namespace

bool captureScreenBackdrop(const Graphics::Screen &screen, IndexedBitmap &bitmap) {
	if (screen.w <= 0 || screen.h <= 0 || screen.format.bytesPerPixel != 1)
		return false;

	bitmap.width = (uint32)screen.w;
	bitmap.height = (uint32)screen.h;
	bitmap.pixels.resize(bitmap.width * bitmap.height);
	for (int y = 0; y < screen.h; ++y)
		memcpy(bitmap.pixels.data() + y * bitmap.width, screen.getBasePtr(0, y), bitmap.width);

	return true;
}

static Common::Rect getRegionBounds(const RegionRecord &region) {
	if (region.right > region.left && region.bottom > region.top)
		return Common::Rect(region.left, region.top, region.right, region.bottom);

	return Common::Rect();
}

bool doesPlayerFacingMatchRegion(int playerFacing, const RegionRecord &region) {
	return region.desiredFacing < 0 || playerFacing == region.desiredFacing;
}

bool doesPlayerOverlapRegion(const Entity &playerEntity, const RegionRecord &region) {
	const Common::Rect regionBounds = getRegionBounds(region);
	if (regionBounds.isEmpty())
		return false;

	const Common::Rect playerBounds = playerEntity.getScreenRect();
	if (playerBounds.right < regionBounds.left ||
			regionBounds.right < playerBounds.left ||
			playerBounds.bottom < regionBounds.top ||
			regionBounds.bottom < playerBounds.top) {
		return false;
	}

	const int playerMaxZ = (int)(playerEntity.getZ() + playerEntity.getZExtent());
	return playerMaxZ >= region.minZ && region.maxZ >= (int)playerEntity.getZ();
}

RoomHoverState Flow::getRoomHoverState(const SceneState &scene) const {
	return Input::resolveRoomHoverState(_engine, scene.state, scene.sceneObjects,
		scene.state.roomNpcs, scene.sceneRegions, _mousePos, _menuTextConfig, &_dialogue);
}

Flow::Flow(HarvesterEngine &engine)
	: _engine(engine), _mousePos(320, 200), _dialogue(engine, _mousePos), _inventory(engine),
	  _menu(engine, _mousePos, _menuItems), _cheats(engine),
	  _room(engine, _inventory, _cheats) {
}

bool Flow::buildDialogueSaveStateBlob(Common::Array<byte> &blob, uint32 saveVersion) {
	Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::NO);
	Common::Serializer serializer(nullptr, &stream);
	serializer.setVersion(saveVersion);
	_dialogue.syncRuntimeSaveState(serializer);
	if (serializer.err())
		return false;

	blob.resize(stream.size());
	if (!blob.empty())
		memcpy(blob.data(), stream.getData(), blob.size());
	return true;
}

bool Flow::loadDialogueSaveStateBlob(const Common::Array<byte> &blob, uint32 saveVersion) {
	if (blob.empty())
		return false;

	Common::MemoryReadStream stream(blob.data(), blob.size());
	Common::Serializer serializer(&stream, nullptr);
	serializer.setVersion(saveVersion);
	_dialogue.syncRuntimeSaveState(serializer);
	return !serializer.err();
}

Common::Error Flow::run() {
	if (!ensureCursorEntity())
		return Common::kReadingFailed;
	Common::Error passwordError = _menu.validateParentalPassword(*this);
	if (passwordError.getCode() != Common::kNoError || _engine.shouldQuit())
		return passwordError;

	clearPendingMainMenuReturn();
	clearPendingGameOverReturn();
	clearPendingNewGameRestart();
	clearPendingCloseupParentRestart();
	resetRoomNpcDialogueState();
	Common::Error error = Common::kNoError;
	if (_engine.hasPendingLoadedSaveRoomState()) {
		Common::String initialTarget = _engine.getPendingLoadedSaveRoomState().entranceName;
		if (initialTarget.empty())
			initialTarget = _engine.getPendingLoadedSaveRoomState().roomName;
		error = runRoomLoop(initialTarget);
	} else {
		_engine.clearCurrentSaveRoomState();
		_engine.clearPendingLoadedDialogueStateBlob();
		_engine.getScript()->resetRuntimeState();
		if (_engine.isDemo()) {
			error = runDemoIntroduction();
			if (error.getCode() != Common::kNoError)
				return error;
		}
		error = runQuickTips();
		if (error.getCode() != Common::kNoError)
			return error;

		clearPendingMainMenuReturn();
		_engine.getScript()->resetRuntimeState();
		error = runRoomLoop("START");
	}
	if (error.getCode() != Common::kNoError)
		return error;
	if (!takePendingMainMenuReturn())
		return Common::kNoError;

	_engine.stopMusic();
	_engine.stopSound();
	return runMainMenu();
}

Common::Error Flow::runMainMenu() {
	return _menu.runMainMenu(*this);
}

Common::Error Flow::runHelpScreen(const byte *palette, float paletteBrightness) {
	EntityManager *entityManager = _engine.getRuntimeEntities();
	if (entityManager)
		entityManager->pauseTimerCountdowns();
	Common::Error result = _menu.runHelpScreen(palette, paletteBrightness, *this);
	if (entityManager)
		entityManager->resumeTimerCountdowns();
	debugC(1, kDebugRoom, "Harvester: help screen close error=%d", result.getCode());
	return result;
}

Common::Error Flow::runRoomMenu(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, bool canSaveGame) {
	return _menu.runRoomMenu(backdrop, palette, paletteBrightness, *this, canSaveGame);
}

Common::Error Flow::runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const ScriptExecutionContext &executionContext,
		const NpcRecord &npc, const Common::String &usedItemName) {
	_queuedDialogueInteraction = InteractionResult();
	_hasQueuedDialogueInteraction = false;
	return _dialogue.runRoomNpcDialogue(backdrop, palette, paletteBrightness,
		executionContext, npc, usedItemName, *this);
}

Common::Error Flow::resolveRoomTransitionTarget(const Common::String &targetName,
		Common::String &resolvedTargetName) {
	resolvedTargetName = targetName;

	Script *script = _engine.getScript();
	if (!script || targetName.empty())
		return Common::kNoError;

	if (script->findMapEntranceRecord(targetName))
		return runTownMapSelector(targetName, resolvedTargetName);

	return Common::kNoError;
}

void Flow::queueDialogueInteraction(const InteractionResult &interaction) {
	_queuedDialogueInteraction = interaction;
	_hasQueuedDialogueInteraction = true;
}

bool Flow::takeQueuedDialogueInteraction(InteractionResult &interaction) {
	if (!_hasQueuedDialogueInteraction)
		return false;

	interaction = Common::move(_queuedDialogueInteraction);
	_queuedDialogueInteraction = InteractionResult();
	_hasQueuedDialogueInteraction = false;
	return true;
}

bool Flow::queueDebugInteraction(const InteractionResult &interaction) {
	if (_hasQueuedDebugInteraction)
		return false;

	// Console-triggered commands are consumed by the room loop so they follow the same
	// transition, dialogue, and continuation handling as in-world interactions.
	_queuedDebugInteraction = interaction;
	_hasQueuedDebugInteraction = true;
	return true;
}

bool Flow::takeQueuedDebugInteraction(InteractionResult &interaction) {
	if (!_hasQueuedDebugInteraction)
		return false;

	interaction = Common::move(_queuedDebugInteraction);
	_queuedDebugInteraction = InteractionResult();
	_hasQueuedDebugInteraction = false;
	return true;
}

void Flow::prepareForNewGame() {
	clearPendingMainMenuReturn();
	clearPendingGameOverReturn();
	clearPendingNewGameRestart();
	_pendingDebugRoomName.clear();
	_pendingDebugRoomDisc = 0;
	_queuedDebugInteraction = InteractionResult();
	_hasQueuedDebugInteraction = false;
	_engine.clearPendingLoadedSaveRoomState();
	_engine.clearPendingLoadedDialogueStateBlob();
	_engine.clearCurrentSaveRoomState();
	if (_engine.getScript())
		_engine.getScript()->resetRuntimeState();
	_cheats.reset();
	resetRoomNpcDialogueState();
}

void Flow::requestNewGameRestart() {
	_pendingNewGameRestart = true;
}

bool Flow::hasPendingNewGameRestart() const {
	return _pendingNewGameRestart;
}

bool Flow::takePendingNewGameRestart() {
	const bool requested = _pendingNewGameRestart;
	_pendingNewGameRestart = false;
	return requested;
}

void Flow::clearPendingNewGameRestart() {
	_pendingNewGameRestart = false;
}

void Flow::requestGameOverReturn() {
	requestMainMenuReturn();
	_pendingGameOverReturn = true;
}

bool Flow::hasPendingGameOverReturn() const {
	return _pendingGameOverReturn;
}

bool Flow::takePendingGameOverReturn() {
	const bool requested = _pendingGameOverReturn;
	_pendingGameOverReturn = false;
	return requested;
}

void Flow::clearPendingGameOverReturn() {
	_pendingGameOverReturn = false;
}

void Flow::requestMainMenuReturn() {
	_pendingMainMenuReturn = true;
	_engine.clearCurrentSaveRoomState();
}

bool Flow::hasPendingMainMenuReturn() const {
	return _pendingMainMenuReturn;
}

bool Flow::takePendingMainMenuReturn() {
	const bool requested = _pendingMainMenuReturn;
	_pendingMainMenuReturn = false;
	return requested;
}

void Flow::clearPendingMainMenuReturn() {
	_pendingMainMenuReturn = false;
}

void Flow::requestCloseupParentRestart() {
	_pendingCloseupParentRestart = true;
}

bool Flow::hasPendingCloseupParentRestart() const {
	return _pendingCloseupParentRestart;
}

bool Flow::takePendingCloseupParentRestart() {
	const bool requested = _pendingCloseupParentRestart;
	_pendingCloseupParentRestart = false;
	return requested;
}

void Flow::clearPendingCloseupParentRestart() {
	_pendingCloseupParentRestart = false;
}

bool Flow::requestDebugRoomChange(const Common::String &roomName, int discNumber) {
	if (roomName.empty() || discNumber < ResourceManager::kFirstDiscNumber ||
			discNumber > ResourceManager::kLastDiscNumber)
		return false;

	_pendingDebugRoomName = roomName;
	_pendingDebugRoomDisc = discNumber;
	return true;
}

Common::String Flow::takePendingDebugRoomChange(int &discNumber) {
	const Common::String roomName = _pendingDebugRoomName;
	discNumber = _pendingDebugRoomDisc;
	_pendingDebugRoomName.clear();
	_pendingDebugRoomDisc = 0;
	return roomName;
}

Common::Error Flow::runRoomLoop(const Common::String &entranceName) {
	const bool nestedRoomLoop = _roomLoopDepth != 0;
	Common::String targetName = entranceName;
	bool targetIsRoomName = false;

	++_roomLoopDepth;
	while (true) {
		const Common::Error error = _room.runRoomLoop(*this, targetName, targetIsRoomName);
		if (error.getCode() != Common::kNoError) {
			--_roomLoopDepth;
			return error;
		}
		if (!hasPendingDebugRoomChange()) {
			--_roomLoopDepth;
			return Common::kNoError;
		}
		if (nestedRoomLoop) {
			--_roomLoopDepth;
			return Common::kNoError;
		}

		int discNumber = 0;
		targetName = takePendingDebugRoomChange(discNumber);
		ResourceManager *resources = _engine.getResources();
		Script *script = _engine.getScript();
		if (!resources || !script) {
			--_roomLoopDepth;
			return Common::kReadingFailed;
		}
		if (discNumber != resources->getCurrentDisc() &&
				(!_engine.activateDisc(discNumber) || !script->reloadTownWorld(*resources))) {
			warning("Harvester: unable to prepare debug room '%s' on disc %d", targetName.c_str(), discNumber);
			--_roomLoopDepth;
			return Common::kReadingFailed;
		}
		debugC(1, kDebugRoom, "Harvester: debug room change room='%s' disc=%d", targetName.c_str(), discNumber);
		targetIsRoomName = true;
	}
}

bool Flow::ensureCursorEntity() {
	EntityManager *entityManager = _engine.getRuntimeEntities();
	if (!entityManager)
		return false;
	if (entityManager->getCursorEntity())
		return true;

	return entityManager->spawnCursorEntity(_mousePos) != nullptr;
}

void Flow::executeStartupAudioCommands(const Common::Array<AudioCommand> &commands) {
	for (const AudioCommand &command : commands)
		(void)_engine.executeAudioCommand(command);
}

void Flow::resetRoomNpcDialogueState() {
	_dialogue.resetRoomNpcDialogueState();
}

void Flow::resetCursorAnimationSequence() {
	if (!ensureCursorEntity())
		return;

	Entity *cursor = _engine.getRuntimeEntities()->getCursorEntity();
	if (!cursor)
		return;

	cursor->setVisible(true);
	cursor->setAnimationSequence(kCursorSequence7);
	(void)_engine.getRuntimeEntities()->syncCursorEntityPosition(_mousePos);
}

bool Flow::tickRuntimeEntities() {
	EntityManager *entityManager = _engine.getRuntimeEntities();
	if (!entityManager)
		return false;
	const bool sceneChanged = entityManager->tickSceneEntities();
	const bool cursorChanged = entityManager->syncCursorEntityPosition(_mousePos);
	return sceneChanged || cursorChanged;
}

bool Flow::handleSystemEvent(const Common::Event &event, Common::Error &result) {
	switch (event.type) {
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		result = Common::kNoError;
		return true;
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		_mousePos = event.mouse;
		break;
	default:
		break;
	}

	return false;
}

} // End of namespace Harvester
