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

	uint32 tag = MKTAG('S', 'N', 'D', ' ');
	uint16 sndId = (uint16)(_castId + _cast->_castIDoffset);

	if (_cast->_version >= kFileVer400 && _children.size() > 0) {
		sndId = _children[0].index;
		tag = _children[0].tag;
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
			Common::String filename = ci->fileName;

			if (!ci->directory.empty())
				filename = ci->directory + g_director->_dirSeparator + ci->fileName;

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
