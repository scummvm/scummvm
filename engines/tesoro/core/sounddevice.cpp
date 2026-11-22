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

#include "common/endian.h"
#include "common/textconsole.h"

#include "tesoro/core/sounddevice.h"

namespace Tesoro {

SoundDevice::SoundDevice(Audio::Mixer *mixer) : _mixer(mixer) {
	for (uint i = 0; i < kRecords; ++i) {
		_order[i] = i;
		_ids[i] = 0;
		_loaded[i] = false;
	}
}

uint32 SoundDevice::play(Common::SeekableReadStream &dat, uint32 archiveOffset) {
	if (_count >= kRecords) {
		warning("SoundDevice: all %d voices are busy", (int)kRecords);
		return 0;
	}

	const uint slot = _order[_count++];
	_loaded[slot] = false;

	// The RIFF size at 4 is the length of the rest of the file; the whole file is copied, as the mixer reads it later.
	byte riff[8] = {};
	dat.seek(archiveOffset, SEEK_SET);
	dat.read(riff, sizeof riff);
	Audio::SeekableAudioStream *stream = nullptr;
	if (memcmp(riff, "RIFF", 4) == 0) {
		const uint32 size = MIN<uint32>(READ_LE_UINT32(riff + 4) + 8, dat.size() - archiveOffset);
		dat.seek(archiveOffset, SEEK_SET);
		stream = Audio::makeWAVStream(dat.readStream(size), DisposeAfterUse::YES);
	}
	if (stream)
		_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_handles[slot], stream);
	_loaded[slot] = stream != nullptr;

	_ids[slot] = ++_lastId;
	return _lastId;
}

bool SoundDevice::active(uint32 id) const {
	for (uint32 i = 0; i < _count; ++i) {
		if (_ids[_order[i]] == id)
			return true;
	}
	return false;
}

void SoundDevice::stop(uint32 id) {
	for (uint32 i = 0; i < _count; ++i) {
		const uint slot = _order[i];
		if (_ids[slot] == id) {
			_mixer->stopHandle(_handles[slot]);
			_loaded[slot] = false;
			update();
			return;
		}
	}
}

void SoundDevice::update() {
	for (uint32 i = 0; i < _count; ++i) {
		const uint slot = _order[i];
		if (_loaded[slot] && _mixer->isSoundHandleActive(_handles[slot]))
			continue;
		// Finished: it goes after the active ones, where the next sound will take its record.
		_loaded[slot] = false;
		SWAP(_order[i], _order[_count - 1]);
		--_count;
		--i;
	}
}

void CdAudio::playTrack(uint8 track) {
	g_system->getAudioCDManager()->stop();
	_track = track;
	g_system->getAudioCDManager()->play(track, -1, 0, 0);
}

} // End of namespace Tesoro
