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

#ifndef ULTIMA8_AUDIO_SONARCAUDIOSAMPLE_H
#define ULTIMA8_AUDIO_SONARCAUDIOSAMPLE_H

#include "ultima/ultima8/audio/audio_sample.h"

namespace Ultima {
namespace Ultima8 {

class SonarcAudioSample : public AudioSample {
	struct SonarcDecompData {
		uint32      _pos;
		uint32      _samplePos;
	};

	static bool _generatedOneTable;
	static int  _oneTable[256];

	static void GenerateOneTable();

	static void decode_EC(int mode, int samplecount,
						  const uint8 *source, int sourcesize,
						  uint8 *dest);
	static void decode_LPC(int order, int nsamples,
						   uint8 *dest, const uint8 *factors);
	static int audio_decode(const uint8 *source, uint8 *dest);

	uint32      _srcOffset;

public:
	SonarcAudioSample(const uint8 *buffer, uint32 size);
	~SonarcAudioSample(void) override;

	void initDecompressor(void *DecompData) const override;
	uint32 decompressFrame(void *DecompData, void *samples) const override;
	void rewind(void *DecompData) const override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
