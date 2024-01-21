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
#include "twp/ids.h"
#include "twp/squtil.h"

#ifndef USE_VORBIS
	#error TWP engine requires USE_VORBIS flag
#endif

namespace Twp {

void SoundStream::open(SoundDefinition *sndDef) {
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
	for (int i = 0; i < 32; i++) {
		if (_slots[i].busy && _slots[i].sndDef->getId() == id) {
			return g_engine->_mixer->isSoundHandleActive(_slots[i].handle);
		}
	}
	// sound ID ?
	return g_engine->_mixer->isSoundIDActive(id);
}

bool AudioSystem::playing(SoundDefinition *soundDef) const {
	for (int i = 0; i < 32; i++) {
		if (_slots[i].busy && _slots[i].sndDef == soundDef) {
			return g_engine->_mixer->isSoundHandleActive(_slots[i].handle);
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
	for (int i = 0; i < 32; i++) {
		if (_slots[i].busy && _slots[i].sndDef->getId() == id) {
			g_engine->_mixer->stopHandle(_slots[i].handle);
		}
	}
	// sound ID ?
	g_engine->_mixer->stopID(id);
}

void AudioSystem::setMasterVolume(float vol) {
	_masterVolume = Twp::clamp(vol, 0.f, 1.f);

	// update sounds
	for (int i = 0; i < 32; i++) {
		if (_slots[i].busy && g_engine->_mixer->isSoundHandleActive(_slots[i].handle)) {
			g_engine->_mixer->setChannelVolume(_slots[i].handle, _slots[i].volume * _masterVolume);
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
		Object *obj = sqobj(slot->objId);
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
	for (int i = 0; i < 32; i++) {
		if (_slots[i].busy && ((_slots[i].sndDef->getId() == id) || (g_engine->_mixer->getSoundID(_slots[i].handle) == id))) {
			_slots[i].volume = vol;
			updateVolume(&_slots[i]);
		}
	}
}

void AudioSystem::update(float elapsed) {
	for (int i = 0; i < 32; i++) {
		if (_slots[i].busy && !g_engine->_mixer->isSoundHandleActive(_slots[i].handle)) {
			_slots[i].busy = false;
		}
	}
	// sound definition ID or sound ID ?
	for (int i = 0; i < 32; i++) {
		if (_slots[i].busy) {
			updateVolume(&_slots[i]);
		}
	}
}

AudioSlot *AudioSystem::getFreeSlot() {
	for (int i = 0; i < 32; i++) {
		AudioSlot *slot = &_slots[i];
		if (!slot->busy || !g_engine->_mixer->isSoundHandleActive(slot->handle)) {
			slot->busy = false;
			return slot;
		}
	}
	return nullptr;
}

int AudioSystem::play(SoundDefinition *sndDef, Audio::Mixer::SoundType cat, int loopTimes, float fadeInTimeMs, float volume, int objId) {
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
	return id;
}

} // namespace Twp
