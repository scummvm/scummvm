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
// FIXME: This pre-release version-16 bump exists only to carry monster corpse
// runtimeState. Fold it into the final Harvester save layout before release.
static const uint32 kHarvesterSaveVersion = 16;

static void logStartupSaveRoomState(const char *operation, const SaveRoomState &state) {
	debugC(1, kDebugRoom,
		"Harvester: %s startup save room state valid=%d entrance='%s' room='%s' spawn=(%d,%d,%d) facing=%d music='%s'",
		operation, state.valid, state.entranceName.c_str(), state.roomName.c_str(),
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

} // End of anonymous namespace

void HarvesterEngine::captureCurrentSaveRoomState(const Common::String &entranceName,
		const Common::String &roomName, int playerX, int playerY, int playerZ, int playerFacing,
		const Common::String &musicPath) {
	_currentSaveRoomState.entranceName = entranceName;
	_currentSaveRoomState.roomName = roomName;
	_currentSaveRoomState.musicPath =
		(_resources && !musicPath.empty()) ? _resources->normalizeResourcePath(musicPath) : musicPath;
	_currentSaveRoomState.playerX = playerX;
	_currentSaveRoomState.playerY = playerY;
	_currentSaveRoomState.playerZ = playerZ;
	_currentSaveRoomState.playerFacing = playerFacing;
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
	if (!s.syncVersion(kHarvesterSaveVersion))
		return Common::kReadingFailed;

	int32 serializedDisc = (_resources && _resources->getCurrentDisc() > 0) ? _resources->getCurrentDisc() : 1;
	// FIXME: Remove the pre-release version-14 fallback once Harvester ships and only
	// clean release-era saves remain.
	s.syncAsSint32LE(serializedDisc, 14);
	const int restoredDisc = serializedDisc > 0 ? serializedDisc : 1;

	SaveRoomState roomState = s.isLoading()
		? SaveRoomState()
		: _currentSaveRoomState;
	if (s.isSaving())
		logStartupSaveRoomState("saving", roomState);
	syncStartupSaveRoomState(s, roomState);
	_script->syncRuntimeSaveState(s);
	// FIXME: Drop the pre-release version-3 dialogue blob fallback after release; clean
	// Harvester saves will always carry dialogue state.
	if (s.getVersion() >= 3) {
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
				_pendingLoadedDialogueStateBlob = dialogueStateBlob;
				_pendingLoadedDialogueStateBlobVersion = s.getVersion();
			}
		}
		if (s.err())
			return s.isLoading() ? Common::kReadingFailed : Common::kWritingFailed;

	if (s.isLoading()) {
		if (!roomState.valid || roomState.roomName.empty())
			return Common::kReadingFailed;
		const int previousDisc = (_resources && _resources->getCurrentDisc() > 0)
			? _resources->getCurrentDisc()
			: 0;
		if (!activateDisc(restoredDisc))
			return Common::kReadingFailed;
		if (previousDisc > 0 && previousDisc != restoredDisc) {
			if (!_script->reloadTownWorld(*_resources)) {
				warning("Harvester: unable to reload town script after save restore disc switch %d -> %d",
					previousDisc, restoredDisc);
				return Common::kReadingFailed;
			}

			debugC(1, kDebugResources,
				"Harvester: reloaded town script after save restore disc switch %d -> %d",
				previousDisc, restoredDisc);
		}
		_currentSaveRoomState = roomState;
		_pendingLoadedSaveRoomState = roomState;
		_pendingLoadedDisc = restoredDisc;
		logStartupSaveRoomState("loaded", roomState);
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
