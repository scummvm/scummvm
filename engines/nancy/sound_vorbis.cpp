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

// Disable symbol overrides for FILE and fseek as those are used in the
// Vorbis headers.
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fseek

#include "engines/nancy/sound_vorbis.h"
#include "audio/decoders/vorbis_intern.h"
#include "common/textconsole.h"

namespace Nancy {

class HISVorbisStream : public Audio::VorbisStream {
public:
	HISVorbisStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag dispose);

	bool seek(const Audio::Timestamp &where) override;
};

HISVorbisStream::HISVorbisStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag dispose)
	: Audio::VorbisStream(stream, dispose) {
	// Start from the true beginning of the stream.
	if (!endOfData()) {
		if (ov_raw_seek(&_ovFile, 0) != 0) {
			warning("HISVorbisStream: initial raw seek to 0 failed");
			_pos = _bufferEnd;
		} else {
			refill();
		}
	}
}

bool HISVorbisStream::seek(const Audio::Timestamp &where) {
	// Zero-time rewinds use raw seeking to match the original engine.
	if (Audio::convertTimeToStreamPos(where, getRate(), false).totalNumberOfFrames() == 0) {
		int res = ov_raw_seek(&_ovFile, 0);
		if (res) {
			warning("HISVorbisStream: raw seek to 0 failed (%d)", res);
			_pos = _bufferEnd;
			return false;
		}
		return refill();
	}
	return Audio::VorbisStream::seek(where);
}

Audio::SeekableAudioStream *makeHISVorbisStream(
	Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse) {
	Audio::SeekableAudioStream *s = new HISVorbisStream(stream, disposeAfterUse);
	if (s && s->endOfData()) {
		delete s;
		return nullptr;
	}
	return s;
}

} // End of namespace Nancy
