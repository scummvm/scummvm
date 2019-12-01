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

namespace Ultima8 {

class IDataSource;

namespace Pentagram {

class AudioSample {
protected:
	uint32  sample_rate;
	uint32  bits;
	bool    stereo;
	int     frame_size;
	uint32  decompressor_size;
	uint32  length;

	uint32  buffer_size;
	uint8   *buffer;

public:
	AudioSample(uint8 *buffer, uint32 size);
	virtual ~AudioSample(void);

	inline uint32 getRate() const {
		return sample_rate;
	}
	inline uint32 getBits() const {
		return bits;
	}
	inline bool isStereo() const {
		return stereo;
	}
	inline uint32 getFrameSize() const {
		return frame_size;
	}
	inline uint32 getDecompressorDataSize() const {
		return decompressor_size;
	}

	//! get AudioSample length (in samples)
	inline uint32 getLength() const {
		return length;
	}

	virtual void initDecompressor(void *DecompData) const = 0;
	virtual uint32 decompressFrame(void *DecompData, void *samples) const = 0;
	virtual void rewind(void *DecompData) const = 0;
};

};

} // End of namespace Ultima8

#endif
