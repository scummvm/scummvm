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

#include "harvester/harvester.h"

#include "common/algorithm.h"
#include "common/serializer.h"
#include "harvester/detection.h"
#include "harvester/flow.h"
#include "harvester/resources.h"
#include "harvester/script.h"

namespace Harvester {

namespace {

static void syncSerializedBool(Common::Serializer &s, bool &value) {
	byte serialized = value ? 1 : 0;
	s.syncAsByte(serialized);
	if (s.isLoading())
		value = serialized != 0;
}

static const char kHarvesterSaveMagic[] = { 'H', 'S', 'A', 'V' };
// Increment this if a future save layout change intentionally breaks
// compatibility with existing Harvester saves.
static const uint32 kHarvesterSaveVersion = 1;

static void logStartupSaveRoomState(const char *operation, const SaveRoomState &state) {
	debugC(1, kDebugRoom,
		"Harvester: %s startup save room state valid=%d disc=%d entrance='%s' room='%s' spawn=(%d,%d,%d) facing=%d music='%s'",
		operation, state.valid, state.discNumber, state.entranceName.c_str(), state.roomName.c_str(),
		state.playerX, state.playerY, state.playerZ, state.playerFacing, state.musicPath.c_str());
}

static void syncStartupSaveRoomState(Common::Serializer &s, SaveRoomState &state) {
	syncSerializedBool(s, state.valid);
	s.syncString(state.entranceName);
	s.syncString(state.roomName);
	s.syncString(state.musicPath);
	s.syncAsSint32LE(state.playerX);
	s.syncAsSint32LE(state.playerY);
	s.syncAsSint32LE(state.playerZ);
	s.syncAsSint32LE(state.playerFacing);
}

static int normalizeDiscNumber(int discNumber) {
	return discNumber >= ResourceManager::kFirstDiscNumber &&
			discNumber <= ResourceManager::kLastDiscNumber
		? discNumber
		: ResourceManager::kFirstDiscNumber;
}

static bool reloadTownWorldForSaveRestore(HarvesterEngine &engine, int previousDisc,
		int discNumber) {
	Script *script = engine.getScript();
	ResourceManager *resources = engine.getResources();
	if (!script || !resources)
		return false;

	if (!script->reloadTownWorld(*resources)) {
		warning("Harvester: unable to reload town script after save restore disc switch %d -> %d",
			previousDisc, discNumber);
		return false;
	}

	debugC(1, kDebugResources,
		"Harvester: reloaded town script after save restore disc switch %d -> %d",
		previousDisc, discNumber);
	return true;
}

static bool loadedRoomTargetAvailableOnDisc(HarvesterEngine &engine,
		const SaveRoomState &roomState, int discNumber) {
	ResourceManager *resources = engine.getResources();
	if (!resources)
		return false;
	if (!engine.activateDisc(discNumber))
		return false;

	Script probeScript;
	if (!probeScript.load(*resources))
		return false;

	if (probeScript.hasRoomSetupTarget(roomState.entranceName, roomState.roomName))
		return true;

	debugC(1, kDebugResources,
		"Harvester: loaded room target entrance='%s' room='%s' not present on disc %d",
		roomState.entranceName.c_str(), roomState.roomName.c_str(), discNumber);
	return false;
}

static bool prepareLoadedRoomDisc(HarvesterEngine &engine, SaveRoomState &roomState,
		int preferredDisc) {
	Script *script = engine.getScript();
	ResourceManager *resources = engine.getResources();
	if (!script || !resources)
		return false;

	const int originalDisc = resources->getCurrentDisc();
	preferredDisc = normalizeDiscNumber(preferredDisc);
	int resolvedDisc = 0;
	if (loadedRoomTargetAvailableOnDisc(engine, roomState, preferredDisc)) {
		resolvedDisc = preferredDisc;
	} else {
		for (int discNumber = ResourceManager::kFirstDiscNumber;
				discNumber <= ResourceManager::kLastDiscNumber; ++discNumber) {
			if (discNumber == preferredDisc)
				continue;
			if (!loadedRoomTargetAvailableOnDisc(engine, roomState, discNumber))
				continue;

			debugC(1, kDebugResources,
				"Harvester: resolved loaded room target entrance='%s' room='%s' on fallback disc %d",
				roomState.entranceName.c_str(), roomState.roomName.c_str(), discNumber);
			resolvedDisc = discNumber;
			break;
		}
	}

	if (resolvedDisc <= 0) {
		warning("Harvester: unable to resolve loaded room target entrance='%s' room='%s' on any mounted disc",
			roomState.entranceName.c_str(), roomState.roomName.c_str());
		return false;
	}

	roomState.discNumber = resolvedDisc;
	if (!engine.activateDisc(roomState.discNumber))
		return false;
	return reloadTownWorldForSaveRestore(engine, originalDisc, roomState.discNumber);
}

} // End of anonymous namespace

void HarvesterEngine::captureCurrentSaveRoomState(const Common::String &entranceName,
		const Common::String &roomName, int playerX, int playerY, int playerZ,
		int playerFacing, int discNumber, const Common::String &musicPath) {
	_currentSaveRoomState.entranceName = entranceName;
	_currentSaveRoomState.roomName = roomName;
	_currentSaveRoomState.musicPath =
		(_resources && !musicPath.empty()) ? _resources->normalizeResourcePath(musicPath) : musicPath;
	_currentSaveRoomState.playerX = playerX;
	_currentSaveRoomState.playerY = playerY;
	_currentSaveRoomState.playerZ = playerZ;
	_currentSaveRoomState.playerFacing = playerFacing;
	_currentSaveRoomState.discNumber = normalizeDiscNumber(discNumber);
	_currentSaveRoomState.valid = !roomName.empty();
}

void HarvesterEngine::clearCurrentSaveRoomState() {
	_currentSaveRoomState.clear();
}

void HarvesterEngine::clearPendingLoadedSaveRoomState() {
	_pendingLoadedSaveRoomState.clear();
	_pendingLoadedDisc = 0;
}

Common::Error HarvesterEngine::syncGame(Common::Serializer &s) {
	if (!_script)
		return s.isLoading() ? Common::kReadingFailed : Common::kWritingFailed;
	if (s.isLoading()) {
		clearPendingLoadedSaveRoomState();
		clearPendingLoadedDialogueStateBlob();
	}
	if (s.isSaving() && !_currentSaveRoomState.valid)
		return Common::kWritingFailed;
	if (!s.matchBytes(kHarvesterSaveMagic, sizeof(kHarvesterSaveMagic)))
		return Common::kReadingFailed;
	if (!s.syncVersion(kHarvesterSaveVersion) || s.getVersion() != kHarvesterSaveVersion) {
		if (s.isLoading()) {
			warning("Harvester: unsupported save version %u, expected %u",
				(uint)s.getVersion(), (uint)kHarvesterSaveVersion);
		}
		return s.isLoading() ? Common::kReadingFailed : Common::kWritingFailed;
	}

	SaveRoomState roomState = s.isLoading()
		? SaveRoomState()
		: _currentSaveRoomState;

	int32 serializedDisc = roomState.discNumber > 0
		? roomState.discNumber
		: ((_resources && _resources->getCurrentDisc() > 0) ? _resources->getCurrentDisc() : 1);
	// Save the room's source disc rather than the transient active asset disc.
	// CD prompts can leave CD3 resources active while a CD1 room is still current.
	s.syncAsSint32LE(serializedDisc);
	const int restoredDisc = normalizeDiscNumber(serializedDisc);
	if (s.isLoading())
		roomState.discNumber = restoredDisc;

	if (s.isSaving())
		logStartupSaveRoomState("saving", roomState);
	syncStartupSaveRoomState(s, roomState);
	_script->syncRuntimeSaveState(s);

	Common::Array<byte> dialogueStateBlob;
	uint32 dialogueStateSize = 0;
	if (s.isSaving()) {
		if (!_activeFlow || !_activeFlow->buildDialogueSaveStateBlob(dialogueStateBlob, s.getVersion()))
			return Common::kWritingFailed;
		dialogueStateSize = dialogueStateBlob.size();
		if (dialogueStateSize == 0)
			return Common::kWritingFailed;
	}
	s.syncAsUint32LE(dialogueStateSize);
	if (s.isLoading()) {
		if (dialogueStateSize == 0)
			return Common::kReadingFailed;
		dialogueStateBlob.resize(dialogueStateSize);
	}
	s.syncBytes(dialogueStateBlob.data(), dialogueStateSize);
	if (s.isLoading()) {
		_pendingLoadedDialogueStateBlob = Common::move(dialogueStateBlob);
		_pendingLoadedDialogueStateBlobVersion = s.getVersion();
	}
	if (s.err())
		return s.isLoading() ? Common::kReadingFailed : Common::kWritingFailed;

	if (s.isLoading()) {
		if (!roomState.valid || roomState.roomName.empty())
			return Common::kReadingFailed;
		if (!prepareLoadedRoomDisc(*this, roomState, restoredDisc))
			return Common::kReadingFailed;
		const int pendingLoadedDisc = roomState.discNumber;
		logStartupSaveRoomState("loaded", roomState);
		_currentSaveRoomState = roomState;
		_pendingLoadedSaveRoomState = Common::move(roomState);
		_pendingLoadedDisc = pendingLoadedDisc;
	}
	return Common::kNoError;
}

Common::Error HarvesterEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer serializer(nullptr, stream);
	return syncGame(serializer);
}

Common::Error HarvesterEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer serializer(stream, nullptr);
	return syncGame(serializer);
}

} // End of namespace Harvester
