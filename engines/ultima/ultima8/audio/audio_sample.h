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

#ifndef ULTIMA8_AUDIO_AUDIOSAMPLE_H
#define ULTIMA8_AUDIO_AUDIOSAMPLE_H

namespace Ultima {
namespace Ultima8 {

class AudioSample {
protected:
	uint32  _sampleRate;
	uint32  _bits;
	bool    _stereo;
	int     _frameSize;
	uint32  _decompressorSize;
	uint32  _length;

	uint32  _bufferSize;
	uint8   const *_buffer;

	bool 	_deleteBuffer;

public:
	AudioSample(const uint8 *buffer, uint32 size, uint32 bits, bool stereo, bool deleteBuffer);
	virtual ~AudioSample(void);

	inline uint32 getRate() const {
		return _sampleRate;
	}
	inline uint32 getBits() const {
		return _bits;
	}
	inline bool isStereo() const {
		return _stereo;
	}
	inline uint32 getFrameSize() const {
		return _frameSize;
	}
	inline uint32 getDecompressorDataSize() const {
		return _decompressorSize;
	}

	//! get AudioSample _length (in samples)
	inline uint32 getLength() const {
		return _length;
	}

	virtual void initDecompressor(void *DecompData) const = 0;
	virtual uint32 decompressFrame(void *DecompData, void *samples) const = 0;
	virtual void rewind(void *DecompData) const = 0;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
