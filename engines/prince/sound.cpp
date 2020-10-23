/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/archive.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"

#include "prince/prince.h"
#include "prince/hero.h"
#include "prince/script.h"

namespace Prince {

void PrinceEngine::playSample(uint16 sampleId, uint16 loopType) {
	if (_audioStream[sampleId]) {
		if (_mixer->isSoundIDActive(sampleId)) {
			return;
		}
		_audioStream[sampleId]->rewind();
		if (sampleId < 28) {
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle[sampleId], _audioStream[sampleId], sampleId, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
		} else {
			_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle[sampleId], _audioStream[sampleId], sampleId, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
		}
	}
}

void PrinceEngine::stopSample(uint16 sampleId) {
	_mixer->stopID(sampleId);
}

void PrinceEngine::stopAllSamples() {
	_mixer->stopAll();
}

void PrinceEngine::freeSample(uint16 sampleId) {
	stopSample(sampleId);
	if (_audioStream[sampleId] != nullptr) {
		delete _audioStream[sampleId];
		_audioStream[sampleId] = nullptr;
	}
}

void PrinceEngine::freeAllSamples() {
	for (int sampleId = 0; sampleId < kMaxSamples; sampleId++) {
		freeSample(sampleId);
	}
}

bool PrinceEngine::loadSample(uint32 sampleSlot, const Common::String &streamName) {
	// FIXME: This is just a workaround streamName is a path
	// SOUND\\SCIERKA1.WAV for now only last path component is used
	Common::String normalizedPath = lastPathComponent(streamName, '\\');

	// WALKAROUND: Wrong name in script, not existing sound in data files
	if (!normalizedPath.compareTo("9997BEKA.WAV")) {
		return 0;
	}

	debugEngine("loadSample slot %d, name %s", sampleSlot, normalizedPath.c_str());

	freeSample(sampleSlot);
	Common::SeekableReadStream *sampleStream = SearchMan.createReadStreamForMember(normalizedPath);
	if (sampleStream == nullptr) {
		delete sampleStream;
		error("Can't load sample %s to slot %d", normalizedPath.c_str(), sampleSlot);
	}
	_audioStream[sampleSlot] = Audio::makeWAVStream(sampleStream, DisposeAfterUse::NO);
	delete sampleStream;
	return true;
}

bool PrinceEngine::loadVoice(uint32 slot, uint32 sampleSlot, const Common::String &streamName) {
	if (getFeatures() & GF_NOVOICES)
		return false;

	_missingVoice = false;

	debugEngine("Loading wav %s slot %d", streamName.c_str(), slot);

	if (slot >= kMaxTexts) {
		error("Text slot bigger than MAXTEXTS %d", kMaxTexts - 1);
		return false;
	}

	freeSample(sampleSlot);
	Common::SeekableReadStream *sampleStream = SearchMan.createReadStreamForMember(streamName);
	if (sampleStream == nullptr) {
		warning("loadVoice: Can't open %s", streamName.c_str());
		_missingVoice = true;	// Insert END tag if needed
		_textSlots[slot]._time = 1; // Set phrase time to none
		_mainHero->_talkTime = 1;
		return false;
	}

	uint32 id = sampleStream->readUint32LE();
	if (id != MKTAG('F', 'F', 'I', 'R')) {
		error("It's not RIFF file %s", streamName.c_str());
		return false;
	}

	sampleStream->skip(0x20);
	id = sampleStream->readUint32LE();
	if (id != MKTAG('a', 't', 'a', 'd')) {
		error("No data section in %s id %04x", streamName.c_str(), id);
		return false;
	}

	id = sampleStream->readUint32LE();
	debugEngine("SetVoice slot %d time %04x", slot, id);
	id <<= 3;
	id /= 22050;
	id += 2;

	_textSlots[slot]._time = id;
	if (!slot) {
		_mainHero->_talkTime = id;
	} else if (slot == 1) {
		_secondHero->_talkTime = id;
	}

	debugEngine("SetVoice slot %d time %04x", slot, id);
	sampleStream->seek(SEEK_SET);
	_audioStream[sampleSlot] = Audio::makeWAVStream(sampleStream, DisposeAfterUse::NO);
	delete sampleStream;
	return true;
}

void PrinceEngine::setVoice(uint16 slot, uint32 sampleSlot, uint16 flag) {
	Common::String sampleName;
	uint32 currentString = _interpreter->getCurrentString();

	if (currentString >= 80000) {
		uint32 nr = currentString - 80000;
		sampleName = Common::String::format("%02d0%02d-%02d.WAV", nr / 100, nr % 100, flag);
	} else if (currentString >= 70000) {
		sampleName = Common::String::format("inv%02d-01.WAV", currentString - 70000);
	} else if (currentString >= 60000) {
		sampleName = Common::String::format("M%04d-%02d.WAV", currentString - 60000, flag);
	// String 316.
	// Fixes PRINCE: conversation with the priest bug #11771
	// When Galador starts conversation with the priest with any gesture,
	// the priest sits down to his place and conversation cannot be continued.
	} else if (currentString == 316 || currentString >= 2000) {
		return;
	} else if (flag >= 100) {
		sampleName = Common::String::format("%03d-%03d.WAV", currentString, flag);
	} else {
		sampleName = Common::String::format("%03d-%02d.WAV", currentString, flag);
	}

	loadVoice(slot, sampleSlot, sampleName);
}

} // End of namespace Prince
