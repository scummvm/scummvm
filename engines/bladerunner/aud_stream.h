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

#ifndef BLADERUNNER_AUD_STREAM_H
#define BLADERUNNER_AUD_STREAM_H

#include "bladerunner/adpcm_decoder.h"

#include "audio/audiostream.h"
#include "common/endian.h"
#include "common/types.h"

namespace BladeRunner {

class AudioCache;

class AudStream : public Audio::RewindableAudioStream {
	byte       *_data;
	byte       *_p;
	byte       *_end;
	AudioCache *_cache;
	int32       _hash;
	uint16      _deafBlockRemain;
	uint16      _frequency;
	uint32      _size;
	uint32      _sizeDecompressed;
	byte        _flags;
	byte        _compressionType;
	int         _overrideFrequency;

	ADPCMWestwoodDecoder _decoder;

	void init(byte *data);

public:
	AudStream(byte *data, int overrideFrequency = -1);
	AudStream(AudioCache *cache, int32 hash, int overrideFrequency = -1);
	~AudStream() override;

	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override { return false; }
	int getRate() const override { return _overrideFrequency > 0 ? _overrideFrequency : _frequency; };
	bool endOfData() const override { return _p == _end; }
	bool rewind() override;
	uint32 getLength() const;
};

} // End of namespace BladeRunner

#endif
