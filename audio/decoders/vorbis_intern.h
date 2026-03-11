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

/**
 * Internal interfaces to the Ogg Vorbis decoder.
 *
 * These can be used to make custom Vorbis decoder subclasses.
 * Each .cpp that includes this header must define
 * FORBIDDEN_SYMBOL_EXCEPTION_FILE and FORBIDDEN_SYMBOL_EXCEPTION_fseek
 * before any #include, because vorbisfile.h uses FILE and fseek.
 */

#ifndef AUDIO_VORBIS_INTERN_H
#define AUDIO_VORBIS_INTERN_H

#include "common/scummsys.h"

#ifdef USE_VORBIS

#include "common/ptr.h"
#include "common/stream.h"

#include "audio/audiostream.h"
#include "audio/timestamp.h"

#ifdef USE_TREMOR
#include <tremor/ivorbisfile.h>
#else
#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>
#endif

namespace Audio {

class VorbisStream : public SeekableAudioStream {
protected:
	Common::DisposablePtr<Common::SeekableReadStream> _inStream;

	bool _isStereo;
	int _rate;

	Timestamp _length;

	OggVorbis_File _ovFile;

	int16 _buffer[4096];
	const int16 *_bufferEnd;
	const int16 *_pos;

public:
	VorbisStream(Common::SeekableReadStream *inStream, DisposeAfterUse::Flag dispose);
	~VorbisStream();

	int readBuffer(int16 *buffer, const int numSamples) override;

	bool endOfData() const override { return _pos >= _bufferEnd; }
	bool isStereo() const override { return _isStereo; }
	int getRate() const override { return _rate; }

	bool seek(const Timestamp &where) override;
	Timestamp getLength() const override { return _length; }

protected:
	bool refill();
};

} // End of namespace Audio

#endif // USE_VORBIS
#endif // AUDIO_VORBIS_INTERN_H
