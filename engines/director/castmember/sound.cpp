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

#include "director/director.h"
#include "director/cast.h"
#include "director/sound.h"
#include "director/castmember/sound.h"

namespace Director {

SoundCastMember::SoundCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastSound;
	_audio = nullptr;
	_looping = 0;
}

SoundCastMember::~SoundCastMember() {
	if (_audio)
		delete _audio;
}

Common::String SoundCastMember::formatInfo() {
	return Common::String::format(
		"looping: %d", _looping
	);
}

void SoundCastMember::load() {
	if (_loaded)
		return;

	uint32 tag = 0;
	uint16 sndId = 0;

	if (_cast->_version < kFileVer400) {
		tag = MKTAG('S', 'N', 'D', ' ');
		sndId = (uint16)(_castId + _cast->_castIDoffset);
	} else if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer600) {
		for (auto &it : _children) {
			if (it.tag == MKTAG('s', 'n', 'd', ' ') || it.tag == MKTAG('S', 'N', 'D', ' ')) {
				sndId = it.index;
				tag = it.tag;
				break;
			}
		}
		if (!sndId) {
			warning("SoundCastMember::load(): No snd resource found in %d children, falling back to D3", _children.size());
			tag = MKTAG('S', 'N', 'D', ' ');
			sndId = (uint16)(_castId + _cast->_castIDoffset);
		}
	} else {
		warning("STUB: SoundCastMember::SoundCastMember(): Sounds not yet supported for version %d", _cast->_version);
	}

	Common::SeekableReadStreamEndian *sndData = _cast->getResource(tag, sndId);
	if (!sndData) {
		tag = MKTAG('s', 'n', 'd', ' ');
		sndData = _cast->getResource(tag, sndId);
	}

	if (sndData == nullptr || sndData->size() == 0) {
		// audio file is linked, load from the filesystem
		CastMemberInfo *ci = _cast->getCastMemberInfo(_castId);
		if (ci) {
			Common::String filename = ci->directory + g_director->_dirSeparator + ci->fileName;

			debugC(2, kDebugLoading, "****** Loading file '%s', cast id: %d", filename.c_str(), sndId);
			AudioFileDecoder *audio = new AudioFileDecoder(filename);
			_audio = audio;
		} else {
			warning("Sound::load(): no resource or info found for cast member %d, skipping", _castId);
		}
	} else {
		debugC(2, kDebugLoading, "****** Loading '%s' id: %d, %d bytes", tag2str(tag), sndId, (int)sndData->size());
		SNDDecoder *audio = new SNDDecoder();
		audio->loadStream(*sndData);
		_audio = audio;
		_size = sndData->size();
		if (_cast->_version < kFileVer400) {
			// The looping flag wasn't added to sound cast members until D4.
			// In older versions, always loop sounds that contain a loop start and end.
			_looping = audio->hasLoopBounds();
		} else {
			// Some sound cast members at version kFileVer400 have looping=true with
			// invalid loop bounds (bigger than sample size or non-consecutive).
			// Resetting loop bounds to sample bounds and disabling looping similar
			// to how D4 playback seems to work.
			if (!audio->hasValidLoopBounds()) {
				// only emit a warning for files > kFileVer400 as it's only kFileVer400 files that should be affected
				if (_cast->_version > kFileVer400) {
					warning("Sound::load(): Invalid loop bounds detected. Disabling looping for cast member id %d, sndId %d", _castId, sndId);
				} else {
					debugC(2, "Sound::load(): Invalid loop bounds detected. Disabling looping for cast member id %d, sndId %d", _castId, sndId);
				}
				_looping = false;
				audio->resetLoopBounds();
			}
		}
	}
	if (sndData)
		delete sndData;

	_loaded = true;
}

void SoundCastMember::unload() {
	if (!_loaded)
		return;

	delete _audio;
	_audio = nullptr;
	_size = 0;
	_looping = false;

	_loaded = false;
}

} // End of namespace Director
