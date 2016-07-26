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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "common/file.h"
#include "common/memstream.h"
#include "voyeur/sound.h"
#include "voyeur/staticres.h"

namespace Voyeur {

SoundManager::SoundManager(Audio::Mixer *mixer) {
	_mixer = mixer;
	_vocOffset = 0;
}

void SoundManager::playVOCMap(byte *voc, int vocSize) {
	Common::MemoryReadStream *dataStream = new Common::MemoryReadStream(voc, vocSize, DisposeAfterUse::NO);
	Audio::AudioStream *audioStream = Audio::makeVOCStream(dataStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, audioStream);
}

void SoundManager::abortVOCMap() {
	_mixer->stopHandle(_soundHandle);
}

void SoundManager::stopVOCPlay() {
	_mixer->stopHandle(_soundHandle);
	_vocOffset = 0;
}

void SoundManager::setVOCOffset(int offset) {
	_vocOffset = offset;
}

Common::String SoundManager::getVOCFileName(int idx) {
	assert(idx >= 0);
	return Common::String::format("%s.voc", SZ_FILENAMES[idx]);
}

void SoundManager::startVOCPlay(const Common::String &filename) {
	Common::File f;
	if (!f.open(filename))
		error("Could not find voc file - %s", filename.c_str());

	Audio::SeekableAudioStream *audioStream = Audio::makeVOCStream(f.readStream(f.size()),
		Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, audioStream);
	audioStream->seek(Audio::Timestamp(_vocOffset * 1000, 11025));
}

void SoundManager::startVOCPlay(int soundId) {
	startVOCPlay(getVOCFileName(soundId));
}

int SoundManager::getVOCStatus() {
	return _mixer->isSoundHandleActive(_soundHandle);
}

uint32 SoundManager::getVOCFrame() {
	Audio::Timestamp timestamp = _mixer->getElapsedTime(_soundHandle);
	return timestamp.secs();
}

} // End of namespace Voyeur
