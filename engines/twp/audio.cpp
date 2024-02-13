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

#include "common/debug.h"
#include "audio/mixer.h"
#include "audio/mixer_intern.h"
#include "audio/audiostream.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"
#include "twp/audio.h"
#include "twp/twp.h"
#include "twp/squtil.h"

#ifndef USE_VORBIS
	#error TWP engine requires USE_VORBIS flag
#endif

namespace Twp {

void SoundStream::open(Common::SharedPtr<SoundDefinition> sndDef) {
	sndDef->load();
	_stream.open(sndDef->_buffer.data(), sndDef->_buffer.size());
}

uint32 SoundStream::read(void *dataPtr, uint32 dataSize) {
	return _stream.read(dataPtr, dataSize);
}

bool SoundStream::eos() const {
	return _stream.eos();
}

int64 SoundStream::pos() const {
	return _stream.pos();
}

int64 SoundStream::size() const {
	return _stream.size();
}

bool SoundStream::seek(int64 offset, int whence) {
	return _stream.seek(offset, whence);
}

SoundDefinition::SoundDefinition(const Common::String &name) : _name(name), _id(newSoundDefId()) {
}

void SoundDefinition::load() {
	if (!_loaded) {
		GGPackEntryReader entry;
		entry.open(g_engine->_pack, _name);
		_buffer.resize(entry.size());
		entry.read(_buffer.data(), entry.size());
	}
}

bool AudioSystem::playing(int id) const {
	// channel ID ?
	if (id >= 1 && id <= 32) {
		if (!_slots[id].busy)
			return false;
		id = g_engine->_mixer->getSoundID(_slots[id].handle);
	}
	// sound definition ID ?
	for (const auto & _slot : _slots) {
		if (_slot.busy && _slot.sndDef->getId() == id) {
			return g_engine->_mixer->isSoundHandleActive(_slot.handle);
		}
	}
	// sound ID ?
	return g_engine->_mixer->isSoundIDActive(id);
}

bool AudioSystem::playing(Common::SharedPtr<SoundDefinition> soundDef) const {
	for (const auto & _slot : _slots) {
		if (_slot.busy && _slot.sndDef == soundDef) {
			return g_engine->_mixer->isSoundHandleActive(_slot.handle);
		}
	}
	return false;
}

void AudioSystem::fadeOut(int id, float fadeTime) {
	if (fadeTime < 0.01f) {
		stop(id);
	} else {
		for (int i = 0; i < 32; i++) {
			if (_slots[i].busy && _slots[i].id == id) {
				_slots[i].fadeOutTimeMs = fadeTime;
			}
		}
	}
}

void AudioSystem::stop(int id) {
	// channel ID ?
	if (id >= 1 && id <= 32) {
		if (!_slots[id].busy)
			return;
		id = g_engine->_mixer->getSoundID(_slots[id].handle);
	}
	// sound definition ID ?
	for (auto & _slot : _slots) {
		if (_slot.busy && _slot.sndDef->getId() == id) {
			g_engine->_mixer->stopHandle(_slot.handle);
		}
	}
	// sound ID ?
	g_engine->_mixer->stopID(id);
}

void AudioSystem::setMasterVolume(float vol) {
	_masterVolume = Twp::clamp(vol, 0.f, 1.f);

	// update sounds
	for (auto & _slot : _slots) {
		if (_slot.busy && g_engine->_mixer->isSoundHandleActive(_slot.handle)) {
			g_engine->_mixer->setChannelVolume(_slot.handle, _slot.volume * _masterVolume);
		}
	}
}

float AudioSystem::getMasterVolume() const {
	return _masterVolume;
}

void AudioSystem::updateVolume(AudioSlot *slot) {
	float vol = _masterVolume * slot->volume;
	if (slot->fadeInTimeMs) {
		vol *= (g_engine->_mixer->getElapsedTime(slot->handle).msecs() / slot->total);
	}
	if (slot->fadeOutTimeMs) {
		float startFade = slot->total - slot->fadeOutTimeMs;
		float progress = (g_engine->_mixer->getElapsedTime(slot->handle).msecs() - startFade) / slot->fadeOutTimeMs;
		if ((progress >= 0) && (progress <= 1.f)) {
			vol *= (1.f - progress);
		}
		if (progress > 1.0f) {
			g_engine->_mixer->stopHandle(slot->handle);
			return;
		}
	}
	if (slot->objId) {
		Common::SharedPtr<Object> obj = sqobj(slot->objId);
		if (obj) {
			float volObj = 0.f;
			if (obj->_room == g_engine->_room) {
				float width = g_engine->_room->getScreenSize().getX();
				float x = g_engine->cameraPos().getX();
				float diff = abs(x - obj->_node->getAbsPos().getX());
				if (diff > (1.5f * width)) {
					volObj = 0.f;
				} else if (diff < (0.25f * width)) {
					volObj = 1.f;
				} else {
					volObj = (width - (diff - (0.25f * width))) / width;
				}

				float pan = clamp((obj->_node->getAbsPos().getX() - x) / (width / 2), -1.0f, 1.0f);
				g_engine->_mixer->setChannelBalance(slot->handle, (int8)(pan * 127));
			}
			vol *= volObj;
		}
	}
	g_engine->_mixer->setChannelVolume(slot->handle, vol * Audio::Mixer::kMaxChannelVolume);
}

void AudioSystem::setVolume(int id, float vol) {
	// channel ID ?
	if (id >= 1 && id <= 32) {
		if (!_slots[id].busy)
			return;
		id = g_engine->_mixer->getSoundID(_slots[id].handle);
	}
	// sound definition ID or sound ID ?
	for (auto & _slot : _slots) {
		if (_slot.busy && ((_slot.sndDef->getId() == id) || (g_engine->_mixer->getSoundID(_slot.handle) == id))) {
			_slot.volume = vol;
			updateVolume(&_slot);
		}
	}
}

void AudioSystem::update(float) {
	for (auto & _slot : _slots) {
		if (_slot.busy && !g_engine->_mixer->isSoundHandleActive(_slot.handle)) {
			if((_slot.loopTimes == -1) || _slot.loopTimes > 0) {
				if(_slot.loopTimes != -1) {
					_slot.loopTimes--;
				}
				Audio::SeekableAudioStream *audioStream;
				Common::String name = _slot.sndDef->getName();
				_slot.stream.seek(0);
				if (name.hasSuffixIgnoreCase(".ogg")) {
					audioStream = Audio::makeVorbisStream(&_slot.stream, DisposeAfterUse::NO);
				} else if (name.hasSuffixIgnoreCase(".wav")) {
					audioStream = Audio::makeWAVStream(&_slot.stream, DisposeAfterUse::NO);
				} else {
					error("Unexpected audio format: %s", name.c_str());
				}
				g_engine->_mixer->playStream(_slot.soundType, &_slot.handle, audioStream, _slot.id, _slot.volume);
			} else {
				_slot.busy = false;
			}
		}
	}
	// sound definition ID or sound ID ?
	for (auto & _slot : _slots) {
		if (_slot.busy) {
			updateVolume(&_slot);
		}
	}
}

AudioSlot *AudioSystem::getFreeSlot() {
	for (auto & _slot : _slots) {
		AudioSlot *slot = &_slot;
		if (!slot->busy || !g_engine->_mixer->isSoundHandleActive(slot->handle)) {
			slot->busy = false;
			return slot;
		}
	}
	return nullptr;
}

int AudioSystem::play(Common::SharedPtr<SoundDefinition> sndDef, Audio::Mixer::SoundType cat, int loopTimes, float fadeInTimeMs, float volume, int objId) {
	AudioSlot *slot = getFreeSlot();
	if (!slot)
		return 0;

	const Common::String &name = sndDef->getName();
	Audio::SeekableAudioStream *audioStream;
	if (name.hasSuffixIgnoreCase(".ogg")) {
		slot->stream.open(sndDef);
		audioStream = Audio::makeVorbisStream(&slot->stream, DisposeAfterUse::NO);
	} else if (name.hasSuffixIgnoreCase(".wav")) {
		slot->stream.open(sndDef);
		audioStream = Audio::makeWAVStream(&slot->stream, DisposeAfterUse::NO);
	} else {
		error("Unexpected audio format: %s", name.c_str());
	}
	if(!audioStream)
		error("Failed to load audio: %s", name.c_str());

	byte vol = (byte)(volume * 255);
	int id = newSoundId();
	if (fadeInTimeMs > 0.f) {
		vol = 0;
	}
	g_engine->_mixer->playStream(cat, &slot->handle, audioStream, id, vol * _masterVolume);
	slot->id = id;
	slot->objId = objId;
	slot->sndDef = sndDef;
	slot->busy = true;
	slot->volume = volume;
	slot->fadeInTimeMs = fadeInTimeMs;
	slot->total = audioStream->getLength().msecs();
	slot->loopTimes = loopTimes;
	slot->soundType = cat;
	return id;
}

int AudioSystem::getElapsed(int id) const {
	for (const auto & _slot : _slots) {
		if (_slot.id == id) {
			Audio::Timestamp t = g_engine->_mixer->getElapsedTime(_slot.handle);
			return t.msecs();
		}
	}
	return 0;
}

int AudioSystem::getDuration(int id) const {
	for (const auto & _slot : _slots) {
		if (_slot.id == id) {
			return _slot.total;
		}
	}
	return 0;
}

} // namespace Twp
