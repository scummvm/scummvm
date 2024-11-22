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

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "dgds/dgds.h"
#include "dgds/sound_raw.h"
#include "dgds/includes.h"

namespace Dgds {

SoundRaw::SoundRaw(ResourceManager *resourceMan, Decompressor *decompressor) : _decompressor(decompressor), _resourceMan(resourceMan) {
}

void SoundRaw::load(const Common::String &filename) {
	Common::SeekableReadStream *fileStream = _resourceMan->getResource(filename);
	if (!fileStream)
		error("SoundRaw::load: Couldn't get raw resource '%s'", filename.c_str());

	DgdsChunkReader chunk(fileStream);
	while (chunk.readNextHeader(EX_RAW, filename)) {
		chunk.readContent(_decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();
		if (chunk.isSection(ID_RAW)) {
			loadFromStream(stream, chunk.getSize());
			break;
		}
	}
}

SoundRaw::~SoundRaw() {
	stop();
}

void SoundRaw::loadFromStream(Common::SeekableReadStream *stream, int size) {
	_data.resize(size);
	stream->read(_data.data(), size);
}

void SoundRaw::play() {
	Audio::Mixer *mixer = DgdsEngine::getInstance()->_mixer;
	Audio::AudioStream *input = Audio::makeRawStream(_data.data(), _data.size(),
													 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
	mixer->playStream(Audio::Mixer::kSFXSoundType, &_handle, input, -1, 255, 0, DisposeAfterUse::YES);
}

void SoundRaw::stop() {
	Audio::Mixer *mixer = DgdsEngine::getInstance()->_mixer;
	mixer->stopHandle(_handle);
}

bool SoundRaw::isPlaying() const {
	Audio::Mixer *mixer = DgdsEngine::getInstance()->_mixer;
	return mixer->isSoundHandleActive(_handle);
}

} // end namespace Dgds
