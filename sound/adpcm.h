/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef SOUND_ADPCM_H
#define SOUND_ADPCM_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "sound/audiostream.h"

class AudioStream;

enum typesADPCM {
	kADPCMOki,
	kADPCMIma
};

// TODO: Switch from a SeekableReadStream to a plain ReadStream. This requires
// some internal refactoring but is definitely possible and will increase the
// flexibility of this code.
class ADPCMInputStream : public AudioStream {
private:
	Common::SeekableReadStream *_stream;
	uint32 _endpos;
	int _channels;
	typesADPCM _type;
	uint32 _blockAlign;

	struct adpcmStatus {
		int32 last;
		int32 stepIndex;
	} _status;

	int16 stepAdjust(byte);
	int16 decodeOKI(byte);
	int16 decodeMSIMA(byte);

public:
	ADPCMInputStream(Common::SeekableReadStream *stream, uint32 size, typesADPCM type, int channels = 2, uint32 blockAlign = 0);
	~ADPCMInputStream() {};

	int readBuffer(int16 *buffer, const int numSamples);
	int readBufferOKI(int16 *buffer, const int numSamples);
	int readBufferMSIMA1(int16 *buffer, const int numSamples);
	int readBufferMSIMA2(int16 *buffer, const int numSamples);

	bool endOfData() const { return (_stream->eos() || _stream->pos() >= _endpos); }
	bool isStereo() const	{ return false; }
	int getRate() const	{ return 22050; }
};

#endif
