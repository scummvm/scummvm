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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/media/scene_audio.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "common/debug.h"
#include "common/ptr.h"
#include "common/serializer.h"
#include "common/util.h"

#include "ripper/detection.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"

namespace Ripper {

SceneAudioManager::Slot::Slot() : volumePercent(100), targetVolumePercent(100),
		triggerFrame(0), volumeStartFrame(0), volumeTiming(0), volumeRampStep(0),
		volumeRampProgress(0), volumeRampDirection(0), control(0), occupied(false),
		preserve(false), volumeRampPending(false), sparseVolumeRamp(false) {
}

SceneAudioManager::SceneAudioManager(RipperEngine *engine, Audio::Mixer *mixer) :
		_engine(engine), _mixer(mixer) {
}

SceneAudioManager::~SceneAudioManager() {
	for (uint i = 0; i < kSlotCount; ++i)
		_mixer->stopHandle(_slots[i].handle);
}

Common::String SceneAudioManager::keyFromPath(const Common::String &path) {
	const size_t separator = path.findLastOf("/\\:");
	const size_t start = separator == Common::String::npos ? 0 : separator + 1;
	const size_t dot = path.findFirstOf('.', start);
	const size_t end = dot == Common::String::npos ? path.size() : dot;
	return path.substr(start, end - start);
}

bool SceneAudioManager::load(const Common::String &path, bool preserve) {
	Common::ScopedPtr<Common::SeekableReadStream> file(_engine->getResources()->createReadStream(
		path, ResourceManager::kSourceSceneAudio));
	if (!file) {
		warning("Ripper: could not load audio '%s'", path.c_str());
		return false;
	}

	uint slotIndex = 0;
	while (slotIndex < kSlotCount && _slots[slotIndex].occupied)
		++slotIndex;
	if (slotIndex == kSlotCount) {
		if (debugChannelSet(2, kDebugAudio))
			debugC(2, kDebugAudio,
				"Ripper: audio slot load ignored path='%s' reason=all-20-slots-occupied slots=[%s]",
				path.c_str(), describeSlots().c_str());
		return true;
	}

	Slot &slot = _slots[slotIndex];
	slot = Slot();
	slot.path = path;
	slot.key = keyFromPath(path);
	slot.occupied = true;
	slot.preserve = preserve;
	if (debugChannelSet(2, kDebugAudio)) {
		uint occupiedCount = 0;
		for (uint i = 0; i < kSlotCount; ++i) {
			if (_slots[i].occupied)
				++occupiedCount;
		}
		debugC(2, kDebugAudio,
			"Ripper: loaded audio slot=%u key='%s' path='%s' preserve=%d occupied=%u/%u",
			slotIndex, slot.key.c_str(), slot.path.c_str(), preserve, occupiedCount, kSlotCount);
	}
	return true;
}

SceneAudioManager::Slot *SceneAudioManager::find(const Common::String &key) {
	for (uint i = 0; i < kSlotCount; ++i) {
		if (_slots[i].occupied && _slots[i].key.equalsIgnoreCase(key))
			return &_slots[i];
	}
	return nullptr;
}

const SceneAudioManager::Slot *SceneAudioManager::find(const Common::String &key) const {
	for (uint i = 0; i < kSlotCount; ++i) {
		if (_slots[i].occupied && _slots[i].key.equalsIgnoreCase(key))
			return &_slots[i];
	}
	return nullptr;
}

Common::String SceneAudioManager::describeSlots() const {
	Common::String description;
	for (uint i = 0; i < kSlotCount; ++i) {
		if (!_slots[i].occupied)
			continue;
		if (!description.empty())
			description += ",";
		description += Common::String::format("%u:%s", i, _slots[i].key.c_str());
	}
	return description;
}

Audio::AudioStream *SceneAudioManager::prepareStream(const Slot &slot) const {
	Common::SeekableReadStream *file = _engine->getResources()->createReadStream(
		slot.path, ResourceManager::kSourceSceneAudio);
	if (!file) {
		warning("Ripper: could not start audio slot key='%s' path='%s' slots=[%s]",
			slot.key.c_str(), slot.path.c_str(), describeSlots().c_str());
		return nullptr;
	}
	Audio::SeekableAudioStream *wavStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
	if (!wavStream)
		return nullptr;
	Audio::AudioStream *stream = wavStream;
	if ((slot.control & 1) != 0)
		stream = Audio::makeLoopingAudioStream(wavStream, 0);
	return stream;
}

bool SceneAudioManager::start(Slot &slot) {
	if (_mixer->isSoundHandleActive(slot.handle))
		return true;
	Audio::AudioStream *stream = prepareStream(slot);
	if (!stream)
		return false;
	start(slot, stream);
	return true;
}

void SceneAudioManager::start(Slot &slot, Audio::AudioStream *stream) {
	const byte volume = (byte)(slot.volumePercent * Audio::Mixer::kMaxChannelVolume / 100);
	// Looping scene beds use music volume; one-shot triggers use SFX volume.
	const Audio::Mixer::SoundType soundType = (slot.control & 1) ?
		Audio::Mixer::kMusicSoundType : Audio::Mixer::kSFXSoundType;
	_mixer->playStream(soundType, &slot.handle, stream, -1, volume);
	debugC(1, kDebugAudio,
		"Ripper: started audio slot key='%s' path='%s' volume=%u trigger=%u control=%u loop=%d active=%d",
		slot.key.c_str(), slot.path.c_str(), slot.volumePercent, slot.triggerFrame,
		slot.control, (slot.control & 1) != 0, _mixer->isSoundHandleActive(slot.handle));
}

bool SceneAudioManager::configure(const Common::String &key, uint volumePercent,
		uint triggerFrame, byte control) {
	Slot *slot = find(key);
	if (!slot) {
		if (debugChannelSet(2, kDebugAudio))
			debugC(2, kDebugAudio,
				"Ripper: audio configuration ignored key='%s' volume=%u trigger=%u control=%u reason=slot-not-found slots=[%s]",
				key.c_str(), volumePercent, triggerFrame, control, describeSlots().c_str());
		return true;
	}
	slot->volumePercent = MIN<uint>(volumePercent == 0 ? 100 : volumePercent, 100);
	slot->targetVolumePercent = slot->volumePercent;
	slot->triggerFrame = triggerFrame;
	slot->control = control;
	debugC(2, kDebugAudio,
		"Ripper: configured audio slot key='%s' volume=%u trigger=%u control=%u loop=%d immediate=%d",
		key.c_str(), slot->volumePercent, triggerFrame, control, (control & 1) != 0,
		triggerFrame == 0);
	return triggerFrame != 0 || start(*slot);
}

void SceneAudioManager::clearSlot(Slot &slot) {
	if (!slot.occupied)
		return;
	_mixer->stopHandle(slot.handle);
	slot = Slot();
}

void SceneAudioManager::clear(const Common::String &key) {
	Slot *slot = find(key);
	if (!slot) {
		if (debugChannelSet(2, kDebugAudio))
			debugC(2, kDebugAudio,
				"Ripper: audio slot clear ignored key='%s' reason=slot-not-found slots=[%s]",
				key.c_str(), describeSlots().c_str());
		return;
	}
	debugC(2, kDebugAudio, "Ripper: cleared audio slot key='%s'", key.c_str());
	clearSlot(*slot);
}

void SceneAudioManager::stop(const Common::String &key) {
	Slot *slot = find(key);
	if (!slot) {
		if (debugChannelSet(2, kDebugAudio))
			debugC(2, kDebugAudio,
				"Ripper: audio slot stop ignored key='%s' reason=slot-not-found slots=[%s]",
				key.c_str(), describeSlots().c_str());
		return;
	}
	const bool active = _mixer->isSoundHandleActive(slot->handle);
	_mixer->stopHandle(slot->handle);
	debugC(2, kDebugAudio, "Ripper: stopped audio slot key='%s' active=%d retained=1",
		key.c_str(), active);
}

void SceneAudioManager::applyVolume(Slot &slot) {
	if (_mixer->isSoundHandleActive(slot.handle)) {
		const byte volume = (byte)(slot.volumePercent * Audio::Mixer::kMaxChannelVolume / 100);
		_mixer->setChannelVolume(slot.handle, volume);
	}
}

void SceneAudioManager::setVolume(const Common::String &key, uint targetVolumePercent,
		uint startFrame, uint timing) {
	Slot *slot = find(key);
	if (!slot) {
		if (debugChannelSet(2, kDebugAudio))
			debugC(2, kDebugAudio,
				"Ripper: audio volume change ignored key='%s' target=%u start=%u timing=%u reason=slot-not-found slots=[%s]",
				key.c_str(), targetVolumePercent, startFrame, timing, describeSlots().c_str());
		return;
	}
	slot->targetVolumePercent = MIN<uint>(targetVolumePercent, 100);
	slot->volumeStartFrame = startFrame;
	slot->volumeTiming = timing;
	if (startFrame == 0) {
		slot->volumePercent = slot->targetVolumePercent;
		slot->volumeRampPending = false;
		applyVolume(*slot);
		debugC(2, kDebugAudio, "Ripper: applied audio volume key='%s' volume=%u immediately",
			key.c_str(), slot->volumePercent);
		return;
	}

	slot->volumeRampDirection = slot->targetVolumePercent < slot->volumePercent ? -1 : 1;
	const uint difference = slot->targetVolumePercent < slot->volumePercent ?
		slot->volumePercent - slot->targetVolumePercent :
		slot->targetVolumePercent - slot->volumePercent;
	slot->volumeRampStep = difference;
	slot->volumeRampProgress = 0;
	slot->sparseVolumeRamp = difference <= timing;
	if (!slot->sparseVolumeRamp) {
		slot->volumeRampStep = timing;
		slot->volumeTiming = difference;
	}
	slot->volumeRampPending = difference != 0;
	debugC(2, kDebugAudio,
		"Ripper: scheduled audio volume key='%s' current=%u target=%u start=%u timing=%u step=%u sparse=%d",
		key.c_str(), slot->volumePercent, slot->targetVolumePercent, startFrame,
		slot->volumeTiming, slot->volumeRampStep, slot->sparseVolumeRamp);
}

void SceneAudioManager::service(uint frame) {
	// Frame counters are one-based and published after presentation.
	for (uint i = 0; i < kSlotCount; ++i) {
		Slot &slot = _slots[i];
		if (!slot.occupied)
			continue;
		if (slot.triggerFrame == frame && !_mixer->isSoundHandleActive(slot.handle)) {
			debugC(3, kDebugAudio, "Ripper: audio frame trigger slot=%u key='%s' frame=%u",
				i, slot.key.c_str(), frame);
			if (!start(slot))
				warning("Ripper: audio frame trigger failed slot=%u key='%s' frame=%u",
					i, slot.key.c_str(), frame);
		}
		if (!slot.volumeRampPending || frame < slot.volumeStartFrame)
			continue;
		if (slot.volumeTiming == 0) {
			slot.volumePercent = slot.targetVolumePercent;
		} else if (slot.sparseVolumeRamp) {
			slot.volumeRampProgress += slot.volumeRampStep;
			if (slot.volumeRampProgress >= slot.volumeTiming) {
				slot.volumeRampProgress -= slot.volumeTiming;
				slot.volumePercent += slot.volumeRampDirection;
			}
		} else {
			do {
				slot.volumeRampProgress += slot.volumeRampStep;
				slot.volumePercent += slot.volumeRampDirection;
				if (slot.volumePercent == slot.targetVolumePercent)
					break;
			} while (slot.volumeRampProgress < slot.volumeTiming);
			if (slot.volumeRampProgress >= slot.volumeTiming)
				slot.volumeRampProgress -= slot.volumeTiming;
		}
		applyVolume(slot);
		if (slot.volumePercent == slot.targetVolumePercent) {
			slot.volumeRampPending = false;
			debugC(3, kDebugAudio,
				"Ripper: completed audio volume ramp slot=%u key='%s' frame=%u volume=%u",
				i, slot.key.c_str(), frame, slot.volumePercent);
		}
	}
}

void SceneAudioManager::resetTriggers() {
	for (uint i = 0; i < kSlotCount; ++i) {
		_slots[i].triggerFrame = 0;
		_slots[i].volumeStartFrame = 0;
		_slots[i].volumeRampPending = false;
	}
	if (debugChannelSet(3, kDebugAudio))
		debugC(3, kDebugAudio, "Ripper: reset per-frame audio triggers slots=[%s]",
			describeSlots().c_str());
}

void SceneAudioManager::clearAll(bool includePreserved) {
	uint cleared = 0;
	for (uint i = 0; i < kSlotCount; ++i) {
		if (_slots[i].occupied && (includePreserved || !_slots[i].preserve)) {
			clearSlot(_slots[i]);
			++cleared;
		}
	}
	if (debugChannelSet(2, kDebugAudio))
		debugC(2, kDebugAudio,
			"Ripper: scene transition cleared audio slots=%u includePreserved=%d retained=[%s]",
			cleared, includePreserved, describeSlots().c_str());
}

bool SceneAudioManager::isActive() const {
	for (uint i = 0; i < kSlotCount; ++i) {
		if (_slots[i].occupied && _mixer->isSoundHandleActive(_slots[i].handle))
			return true;
	}
	return false;
}

SceneAudioManager::Snapshot::Snapshot() {
	memset(active, 0, sizeof(active));
}

SceneAudioManager::Snapshot::~Snapshot() {
}

void SceneAudioManager::captureSnapshot(Snapshot &state) const {
	for (uint i = 0; i < kSlotCount; ++i) {
		state.streams[i].reset();
		state.slots[i] = _slots[i];
		state.active[i] = _slots[i].occupied && _mixer->isSoundHandleActive(_slots[i].handle);
		state.slots[i].handle = Audio::SoundHandle();
	}
	if (debugChannelSet(2, kDebugAudio))
		debugC(2, kDebugAudio, "Ripper: captured scene audio snapshot slots=[%s]",
			describeSlots().c_str());
}

bool SceneAudioManager::restoreSnapshot(Snapshot &state) {
	if (!prepareSnapshot(state)) {
		debugC(1, kDebugAudio, "Ripper: could not prepare scene audio snapshot");
		return false;
	}
	applySnapshot(state);
	return true;
}

bool SceneAudioManager::syncGame(Common::Serializer &serializer) {
	Snapshot state;
	if (!syncGame(serializer, state))
		return false;
	if (serializer.isLoading())
		applySnapshot(state);
	return true;
}

bool SceneAudioManager::syncGame(Common::Serializer &serializer, Snapshot &state) {
	if (serializer.getVersion() <= 2) {
		Common::String audioPath;
		byte active = 0;
		uint32 volumePercent = 100;
		byte loop = 0;
		serializer.syncString(audioPath);
		serializer.syncAsByte(active);
		serializer.syncAsUint32LE(volumePercent);
		serializer.syncAsByte(loop);
		if (serializer.err() || audioPath.size() > 256 || volumePercent > 100)
			return false;
		if (serializer.isSaving())
			return true;
		Slot &slot = state.slots[0];
		slot.path = audioPath;
		slot.key = keyFromPath(audioPath);
		slot.occupied = !audioPath.empty();
		slot.volumePercent = volumePercent;
		slot.targetVolumePercent = volumePercent;
		slot.control = loop != 0 ? 1 : 0;
		state.active[0] = active != 0;
	} else {
		for (uint i = 0; i < kSlotCount; ++i) {
			Slot &slot = state.slots[i];
			if (serializer.isSaving())
				slot = _slots[i];
			Common::String path = serializer.isSaving() ? slot.path : Common::String();
			byte occupied = slot.occupied ? 1 : 0;
			byte preserve = slot.preserve ? 1 : 0;
			byte active = slot.occupied && _mixer->isSoundHandleActive(slot.handle) ? 1 : 0;
			uint32 volumePercent = slot.volumePercent;
			uint32 targetVolumePercent = slot.targetVolumePercent;
			uint32 triggerFrame = slot.triggerFrame;
			uint32 volumeStartFrame = slot.volumeStartFrame;
			uint32 volumeTiming = slot.volumeTiming;
			uint32 volumeRampStep = slot.volumeRampStep;
			uint32 volumeRampProgress = slot.volumeRampProgress;
			int32 volumeRampDirection = slot.volumeRampDirection;
			byte control = slot.control;
			byte volumeRampPending = slot.volumeRampPending ? 1 : 0;
			byte sparseVolumeRamp = slot.sparseVolumeRamp ? 1 : 0;

			serializer.syncString(path);
			serializer.syncAsByte(occupied);
			serializer.syncAsByte(preserve);
			serializer.syncAsByte(active);
			serializer.syncAsUint32LE(volumePercent);
			serializer.syncAsUint32LE(targetVolumePercent);
			serializer.syncAsUint32LE(triggerFrame);
			serializer.syncAsUint32LE(volumeStartFrame);
			serializer.syncAsUint32LE(volumeTiming);
			serializer.syncAsUint32LE(volumeRampStep);
			serializer.syncAsUint32LE(volumeRampProgress);
			serializer.syncAsSint32LE(volumeRampDirection);
			serializer.syncAsByte(control);
			serializer.syncAsByte(volumeRampPending);
			serializer.syncAsByte(sparseVolumeRamp);
			if (serializer.err() || path.size() > 256 || (occupied != 0 && path.empty()) ||
					occupied > 1 || preserve > 1 || active > 1 || volumePercent > 100 ||
					targetVolumePercent > 100 || volumeRampDirection < -1 ||
					volumeRampDirection > 1 || volumeRampPending > 1 || sparseVolumeRamp > 1)
				return false;
			if (serializer.isSaving() || occupied == 0)
				continue;
			slot.path = path;
			slot.key = keyFromPath(path);
			slot.occupied = true;
			slot.preserve = preserve != 0;
			slot.volumePercent = volumePercent;
			slot.targetVolumePercent = targetVolumePercent;
			slot.triggerFrame = triggerFrame;
			slot.volumeStartFrame = volumeStartFrame;
			slot.volumeTiming = volumeTiming;
			slot.volumeRampStep = volumeRampStep;
			slot.volumeRampProgress = volumeRampProgress;
			slot.volumeRampDirection = volumeRampDirection;
			slot.control = control;
			slot.volumeRampPending = volumeRampPending != 0;
			slot.sparseVolumeRamp = sparseVolumeRamp != 0;
			state.active[i] = active != 0;
		}
	}
	if (serializer.err())
		return false;
	return !serializer.isLoading() || prepareSnapshot(state);
}

bool SceneAudioManager::prepareSnapshot(Snapshot &state) const {
	for (uint i = 0; i < kSlotCount; ++i) {
		state.streams[i].reset();
		const Slot &slot = state.slots[i];
		if (!slot.occupied)
			continue;
		if (state.active[i]) {
			state.streams[i].reset(prepareStream(slot));
			if (!state.streams[i])
				return false;
		} else {
			Common::ScopedPtr<Common::SeekableReadStream> file(_engine->getResources()->createReadStream(
				slot.path, ResourceManager::kSourceSceneAudio));
			if (!file)
				return false;
		}
	}
	return true;
}

void SceneAudioManager::applySnapshot(Snapshot &state) {
	clearAll(true);
	uint restoredSlots = 0;
	for (uint i = 0; i < kSlotCount; ++i) {
		_slots[i] = Common::move(state.slots[i]);
		if (!_slots[i].occupied)
			continue;
		if (state.streams[i])
			start(_slots[i], state.streams[i].release());
		++restoredSlots;
	}
	if (debugChannelSet(1, kDebugAudio))
		debugC(1, kDebugAudio,
			"Ripper: restored named audio slots=%u active=%d slots=[%s]",
			restoredSlots, isActive(), describeSlots().c_str());
}

} // End of namespace Ripper
