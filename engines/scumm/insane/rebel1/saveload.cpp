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

#include "common/ptr.h"
#include "common/stream.h"
#include "common/endian.h"

#include "graphics/thumbnail.h"

#include "scumm/scumm.h"
#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel1/rebel.h"

namespace Scumm {

const uint32 kRA1SaveTag = MKTAG('R', 'A', '1', 'S');
const uint32 kRA1SaveVersion = 1;
const uint32 kRA1ScummSaveVersion = 124;

struct Rebel1SavegameHeader {
	uint32 type;
	uint32 size;
	uint32 version;
	char name[32];
};

bool writeRebel1SavegameHeader(Common::WriteStream *out, const Common::String &desc) {
	Rebel1SavegameHeader hdr;
	hdr.type = MKTAG('S', 'C', 'V', 'M');
	hdr.size = 0;
	hdr.version = kRA1ScummSaveVersion;
	memset(hdr.name, 0, sizeof(hdr.name));
	Common::strlcpy(hdr.name, desc.c_str(), sizeof(hdr.name));

	out->writeUint32BE(hdr.type);
	out->writeUint32LE(hdr.size);
	out->writeUint32LE(hdr.version);
	out->write(hdr.name, sizeof(hdr.name));
	return !out->err();
}

bool readRebel1SavegameHeader(Common::SeekableReadStream *in, Common::String *desc, uint32 *version) {
	Rebel1SavegameHeader hdr;
	hdr.type = in->readUint32BE();
	hdr.size = in->readUint32LE();
	hdr.version = in->readUint32LE();
	in->read(hdr.name, sizeof(hdr.name));
	if (in->err() || hdr.type != MKTAG('S', 'C', 'V', 'M'))
		return false;

	if (hdr.version > 0xFFFFFF)
		hdr.version = SWAP_BYTES_32(hdr.version);
	if (hdr.version < VER(52) || hdr.version > kRA1ScummSaveVersion)
		return false;

	hdr.name[sizeof(hdr.name) - 1] = 0;
	if (desc)
		*desc = hdr.name;
	if (version)
		*version = hdr.version;
	return true;
}

int InsaneRebel1::getCurrentSaveLevel() const {
	return CLIP<int>(_resumeLevel, 1, kNumLevels);
}

int InsaneRebel1::getAutosaveTargetSlot() const {
	return _activeSaveSlot >= 0 ? _activeSaveSlot : 0;
}

Common::Error InsaneRebel1::writeSaveState(int slot, const Common::String &desc, const SaveState &state) const {
	Common::String filename;
	Common::SeekableWriteStream *out = _vm->openSaveFileForWriting(slot, false, filename);
	if (!out)
		return Common::kWritingFailed;

	writeRebel1SavegameHeader(out, desc);
	Graphics::saveThumbnail(*out);
	_vm->saveInfos(out);

	out->writeUint32BE(kRA1SaveTag);
	out->writeUint32LE(kRA1SaveVersion);
	out->writeSint16LE((int16)CLIP<int>(state.resumeLevel, 1, kNumLevels));
	out->writeSint16LE((int16)MAX<int>(state.lives, 0));
	out->writeSint32LE(state.score);
	out->writeSint32LE(state.prevScore);
	out->writeSint16LE((int16)CLIP<int>(state.difficulty, 0, 2));
	out->writeSint16LE((int16)CLIP<int>(state.maxChapterUnlocked, 0, kNumLevels));

	out->finalize();
	const bool failed = out->err();
	delete out;

	if (failed) {
		warning("RA1: failed to write save '%s'", filename.c_str());
		return Common::kWritingFailed;
	}

	debugC(DEBUG_INSANE, "saved slot=%d level=%d lives=%d score=%d desc='%s'",
		slot, state.resumeLevel, state.lives, state.score, desc.c_str());
	return Common::kNoError;
}

bool InsaneRebel1::readSaveState(int slot, SaveState &state, Common::String *desc) const {
	Common::String filename;
	Common::ScopedPtr<Common::SeekableReadStream> in(_vm->openSaveFileForReading(slot, false, filename));
	if (!in)
		return false;

	uint32 headerVersion = 0;
	if (!readRebel1SavegameHeader(in.get(), desc, &headerVersion)) {
		warning("RA1: invalid save header in '%s'", filename.c_str());
		return false;
	}

	if (headerVersion >= VER(52) && !Graphics::skipThumbnail(*in)) {
		warning("RA1: save thumbnail could not be skipped in '%s'", filename.c_str());
		return false;
	}

	if (headerVersion >= VER(56)) {
		SaveStateMetaInfos infos;
		if (!_vm->loadInfos(in.get(), &infos)) {
			warning("RA1: save info section could not be found in '%s'", filename.c_str());
			return false;
		}
		_vm->setTotalPlayTime(infos.playtime * 1000);
	} else {
		_vm->setTotalPlayTime();
	}

	if (in->readUint32BE() != kRA1SaveTag) {
		warning("RA1: missing RA1 save data in '%s'", filename.c_str());
		return false;
	}

	const uint32 version = in->readUint32LE();
	if (version == 0 || version > kRA1SaveVersion) {
		warning("RA1: unsupported save version %u in '%s'", version, filename.c_str());
		return false;
	}

	state.resumeLevel = CLIP<int>(in->readSint16LE(), 1, kNumLevels);
	state.lives = MAX<int>(in->readSint16LE(), 0);
	state.score = in->readSint32LE();
	state.prevScore = in->readSint32LE();
	state.difficulty = CLIP<int>(in->readSint16LE(), 0, 2);
	state.maxChapterUnlocked = CLIP<int>(in->readSint16LE(), 0, kNumLevels);

	if (in->err()) {
		warning("RA1: truncated save data in '%s'", filename.c_str());
		return false;
	}

	return true;
}

Common::Error InsaneRebel1::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	if (isAutosave) {
		autosaveProgress();
		return Common::kNoError;
	}

	SaveState state;
	state.resumeLevel = getCurrentSaveLevel();
	state.lives = _lives;
	state.score = _score;
	state.prevScore = _prevScore;
	state.difficulty = _difficulty;
	state.maxChapterUnlocked = _maxChapterUnlocked;

	Common::String saveDesc = desc;
	if (saveDesc.empty())
		saveDesc = Common::String::format("Level %d", state.resumeLevel);

	Common::Error result = writeSaveState(slot, saveDesc, state);
	if (result.getCode() == Common::kNoError)
		_activeSaveSlot = slot;
	return result;
}

Common::Error InsaneRebel1::loadGameState(int slot, bool startupLoad) {
	SaveState state;
	if (!readSaveState(slot, state))
		return Common::kReadingFailed;

	_resumeLevel = state.resumeLevel;
	_startLevel = state.resumeLevel;
	_lives = state.lives;
	_score = state.score;
	_prevScore = state.prevScore;
	_difficulty = state.difficulty;
	_maxChapterUnlocked = state.maxChapterUnlocked;
	_health = kMaxHealth;
	_activeSaveSlot = slot;

	loadTuningForLevel(_resumeLevel - 1);
	if (!startupLoad) {
		_loadRequested = true;
		_vm->_smushVideoShouldFinish = true;
	}

	debugC(DEBUG_INSANE, "loaded slot=%d level=%d lives=%d score=%d", slot, _resumeLevel, _lives, _score);
	return Common::kNoError;
}

void InsaneRebel1::autosaveProgress() {
	SaveState state;
	state.resumeLevel = getCurrentSaveLevel();
	state.lives = _lives;
	state.score = _score;
	state.prevScore = _prevScore;
	state.difficulty = _difficulty;
	state.maxChapterUnlocked = _maxChapterUnlocked;

	const int slot = getAutosaveTargetSlot();
	SaveState oldState;
	Common::String oldDesc;
	if (readSaveState(slot, oldState, &oldDesc)) {
		if (oldState.resumeLevel > state.resumeLevel ||
				(oldState.resumeLevel == state.resumeLevel && oldState.lives >= state.lives)) {
			debugC(DEBUG_INSANE, "skipping autosave slot=%d level=%d lives=%d; existing level=%d lives=%d",
				slot, state.resumeLevel, state.lives, oldState.resumeLevel, oldState.lives);
			return;
		}
	}

	Common::String desc = oldDesc;
	if (slot == 0 || desc.empty())
		desc = "Autosave";

	(void)writeSaveState(slot, desc, state);
}

} // End of namespace Scumm
