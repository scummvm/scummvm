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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/**
 * @file
 * Sound decoder used in engines:
 * - sword1 (PSX port of the game)
 * - sword2 (PSX port of the game)
 */

#ifndef SOUND_VAG_H
#define SOUND_VAG_H

#include "sound/audiostream.h"
#include "common/stream.h"

namespace Audio {

class VagStream : public Audio::AudioStream {
public:
	VagStream(Common::SeekableReadStream *stream, bool loop = false, int rate = 11025);
	~VagStream();

	bool isStereo() const { return false; }
	bool endOfData() const { return _stream->pos() == _stream->size(); }
	int getRate() const { return _rate; }
	int readBuffer(int16 *buffer, const int numSamples);
	void rewind();

private:
	Common::SeekableReadStream *_stream;

	bool _loop;
	byte _predictor;
	double _samples[28];
	byte _samplesRemaining;
	int _rate;
	double _s1, _s2;
};

} // End of namespace Sword1

#endif
