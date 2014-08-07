/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/mutex.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/mp3.h"
#include "engines/grim/debug.h"
#include "engines/grim/resource.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/emi/sound/mp3track.h"

namespace Grim {

/**
 * This is a an extension of Audio::SubLooppingAudioStream that adds a start
 * time parameter as well as a getter for the stream position.
 */
class EMISubLoopingAudioStream : public Audio::AudioStream {
public:
	EMISubLoopingAudioStream(Audio::SeekableAudioStream *stream, uint loops,
		const Audio::Timestamp start,
		const Audio::Timestamp loopStart,
		const Audio::Timestamp loopEnd,
		DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES)
		: _parent(stream, disposeAfterUse),
		_pos(convertTimeToStreamPos(start, getRate(), isStereo())),
		_loopStart(convertTimeToStreamPos(loopStart, getRate(), isStereo())),
		_loopEnd(convertTimeToStreamPos(loopEnd, getRate(), isStereo())),
		_done(false), _hasLooped(false) {
		assert(loopStart < loopEnd);

		if (!_parent->seek(_pos))
			_done = true;
	}

	int readBuffer(int16 *buffer, const int numSamples) {
		if (_done)
			return 0;

		int framesLeft = MIN(_loopEnd.frameDiff(_pos), numSamples);
		int framesRead = _parent->readBuffer(buffer, framesLeft);
		_pos = _pos.addFrames(framesRead);

		if (framesRead < framesLeft && _parent->endOfData()) {
			// TODO: Proper error indication.
			_done = true;
			return framesRead;
		}
		else if (_pos == _loopEnd) {
			if (!_parent->seek(_loopStart)) {
				// TODO: Proper error indication.
				_done = true;
				return framesRead;
			}

			_pos = _loopStart;
			framesLeft = numSamples - framesLeft;
			_hasLooped = true;
			return framesRead + readBuffer(buffer + framesRead, framesLeft);
		}
		else {
			return framesRead;
		}
	}

	bool hasLooped() const { return _hasLooped; }
	bool endOfData() const { return _done; }

	bool isStereo() const { return _parent->isStereo(); }
	int getRate() const { return _parent->getRate(); }
	Audio::Timestamp getPos() const { return _pos; }

private:
	Common::DisposablePtr<Audio::SeekableAudioStream> _parent;

	Audio::Timestamp _pos;
	Audio::Timestamp _loopStart, _loopEnd;

	bool _done;
	bool _hasLooped;
};

void MP3Track::parseRIFFHeader(Common::SeekableReadStream *data) {
	uint32 tag = data->readUint32BE();
	if (tag == MKTAG('R','I','F','F')) {
		_endFlag = false;
		data->seek(18, SEEK_CUR);
		_channels = data->readByte();
		data->readByte();
		_freq = data->readUint32LE();
		data->seek(6, SEEK_CUR);
		_bits = data->readByte();
		data->seek(5, SEEK_CUR);
		_regionLength = data->readUint32LE();
		_headerSize = 44;
	} else {
		error("Unknown file header");
	}
}

MP3Track::JMMCuePoints MP3Track::parseJMMFile(const Common::String &filename) {
	JMMCuePoints cuePoints;
	Common::SeekableReadStream *stream = g_resourceloader->openNewStreamFile(filename);
	if (stream) {
		TextSplitter ts(filename, stream);
		float startMs = 0.0f;
		float loopStartMs = 0.0f, loopEndMs = 0.0f;

		ts.scanString(".start %f", 1, &startMs);
		if (ts.checkString(".jump"))
			ts.scanString(".jump %f %f", 2, &loopEndMs, &loopStartMs);

		// Use microsecond precision for the timestamps.
		cuePoints._start = Audio::Timestamp(startMs / 1000, (int)(startMs * 1000) % 1000000, 1000000);
		cuePoints._loopStart = Audio::Timestamp(loopStartMs / 1000, (int)(loopStartMs * 1000) % 1000000, 1000000);
		cuePoints._loopEnd = Audio::Timestamp(loopEndMs / 1000, (int)(loopEndMs * 1000) % 1000000, 1000000);
	}
	delete stream;
	return cuePoints;
}

MP3Track::MP3Track(Audio::Mixer::SoundType soundType) {
	_soundType = soundType;
	_headerSize = 0;
	_regionLength = 0;
	_freq = 0;
	_bits = 0,
	_channels = 0;
	_endFlag = false;
	_looping = false;
}

MP3Track::~MP3Track() {
	stop();
	if (_handle) {
		g_system->getMixer()->stopHandle(*_handle);
		delete _handle;
	}
}

bool MP3Track::openSound(const Common::String &filename, const Common::String &soundName, const Audio::Timestamp *start) {
	Common::SeekableReadStream *file = g_resourceloader->openNewStreamFile(filename);
	if (!file) {
		Debug::debug(Debug::Sound, "Stream for %s not open", soundName.c_str());
		return false;
	}
	_soundName = soundName;
#ifndef USE_MAD
	warning("Cannot open %s, MP3 support not enabled", soundName.c_str());
	return true;
#else
	parseRIFFHeader(file);
	
	MP3Track::JMMCuePoints cuePoints;
	if (soundName.size() > 4) {
		cuePoints = parseJMMFile(Common::String(filename.c_str(), filename.size() - 4) + ".jmm");
	}

	if (start)
		cuePoints._start = *start;

	Audio::SeekableAudioStream *mp3Stream = Audio::makeMP3Stream(file, DisposeAfterUse::YES);

	if (cuePoints._loopEnd <= cuePoints._loopStart) {
		_stream = mp3Stream;
		mp3Stream->seek(cuePoints._start);
		_looping = false;
	} else {
		_stream = new EMISubLoopingAudioStream(mp3Stream, 0, cuePoints._start, cuePoints._loopStart, cuePoints._loopEnd);
		_looping = true;
	}
	_handle = new Audio::SoundHandle();
	return true;
#endif
}

bool MP3Track::hasLooped() {
	if (!_stream || !_looping)
		return false;
	EMISubLoopingAudioStream *las = static_cast<EMISubLoopingAudioStream*>(_stream);
	return las->hasLooped();
}

bool MP3Track::isPlaying() {
	if (!_handle)
		return false;

	return g_system->getMixer()->isSoundHandleActive(*_handle);
}

Audio::Timestamp MP3Track::getPos() {
	if (!_stream)
		return Audio::Timestamp(0);
	if (_looping) {
		EMISubLoopingAudioStream *slas = static_cast<EMISubLoopingAudioStream*>(_stream);
		return slas->getPos();
	} else {
		return g_system->getMixer()->getSoundElapsedTime(*_handle);
	}
}

} // end of namespace Grim
