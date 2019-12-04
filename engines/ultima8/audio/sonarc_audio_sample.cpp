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

#include "ultima8/misc/pent_include.h"
#include "Sonarcaudio_sample.h"
#include "ultima8/filesys/idata_source.h"

namespace Ultima8 {
namespace Pentagram {

bool SonarcAudioSample::GeneratedOneTable = false;
int SonarcAudioSample::OneTable[256];

SonarcAudioSample::SonarcAudioSample(uint8 *buffer_, uint32 size_) :
	AudioSample(buffer_, size_),
	src_offset(0x20)

{
	if (!GeneratedOneTable) GenerateOneTable();

	length = *buffer;
	length |= *(buffer + 1) << 8;
	length |= *(buffer + 2) << 16;
	length |= *(buffer + 3) << 24;

	sample_rate  = *(buffer + 4);
	sample_rate |= *(buffer + 5) << 8;
	bits = 8;
	stereo = false;

	// Get frame bytes... we need to compensate for 'large' files
	uint32 frame_bytes = *(buffer + src_offset);
	frame_bytes |= (*(buffer + src_offset + 1)) << 8;

	if (frame_bytes == 0x20 && length > 32767) {
		src_offset += 0x100;
	}

	// Get Num Frame Samples
	frame_size = *(buffer + src_offset + 2);
	frame_size |= (*(buffer + src_offset + 3)) << 8;


	decompressor_size = sizeof(SonarcDecompData);
}

SonarcAudioSample::~SonarcAudioSample(void) {
}

//
// Sonarc Audio Decompressor
//

void SonarcAudioSample::GenerateOneTable() {
	// OneTable[x] gives the number of consecutive 1's on the low side of x
	for (int i = 0; i < 256; ++i)
		OneTable[i] = 0;

	for (int power = 2; power < 32; power *= 2)
		for (int col = power - 1; col < 16; col += power)
			for (int row = 0; row < 16; ++row)
				OneTable[row * 16 + col]++;

	for (int i = 0; i < 16; ++i)
		OneTable[i * 16 + 15] += OneTable[i];
}

void SonarcAudioSample::decode_EC(int mode, int samplecount,
                                  const uint8 *source, int sourcesize,
                                  uint8 *dest) {
	bool zerospecial = false;
	uint32 data = 0;
	int inputbits = 0; // current 'fill rate' of data window

	if (mode >= 7) {
		mode -= 7;
		zerospecial = true;
	}

	while (samplecount) {
		// fill data window
		while (sourcesize && inputbits <= 24) {
			data |= (*source++) << inputbits;
			sourcesize--;
			inputbits += 8;
		}

		if (zerospecial && !(data & 0x1)) {
			*dest++ = 0x80; // output zero
			data >>= 1;
			inputbits--;
		} else {
			if (zerospecial) {
				data >>= 1; // strip one
				inputbits--;
			}

			uint8 lowByte = data & 0xFF;
			int ones = OneTable[lowByte];

			if (ones == 0) {
				data >>= 1; // strip zero
				// low byte contains (mode+1) bits of the sample
				int8 sample = data & 0xFF;
				sample <<= (7 - mode);
				sample >>= (7 - mode); // sign extend
				*dest++ = (uint8)(sample + 0x80);
				data >>= mode + 1;
				inputbits -= mode + 2;
			} else if (ones < 7 - mode) {
				data >>= ones + 1; // strip ones and zero
				// low byte contains (mode+ones) bits of the sample
				int8 sample = data & 0xFF;
				sample <<= (7 - mode - ones);
				sample &= 0x7F;
				if (!(sample & 0x40))
					sample |= 0x80; // reconstruct sign bit
				sample >>= (7 - mode - ones); // sign extend
				*dest++ = (uint8)(sample + 0x80);
				data >>= (mode + ones);
				inputbits -= mode + 2 * ones + 1;
			} else {
				data >>= (7 - mode); // strip ones
				// low byte contains 7 bits of the sample
				int8 sample = data & 0xFF;
				sample &= 0x7F;
				if (!(sample & 0x40))
					sample |= 0x80; // reconstruct sign bit
				*dest++ = (uint8)(sample + 0x80);
				data >>= 7;
				inputbits -= 2 * 7 - mode;
			}
		}
		samplecount--;
	}
}

void SonarcAudioSample::decode_LPC(int order, int nsamples,
                                   uint8 *dest, const uint8 *factors) {
	uint8 *startdest = dest;
	dest -= order;

	// basic linear predictive (de)coding
	// the errors this produces are fixed by decode_EC

	for (int i = 0; i < nsamples; ++i) {
		uint8 *loopdest = dest++;
		int accum = 0;
		for (int j = order - 1; j >= 0; --j) {
			int8 val1 = (loopdest < startdest) ? 0 : (*loopdest);
			loopdest++;
			val1 ^= 0x80;
			int16 val2 = factors[j * 2] + (factors[j * 2 + 1] << 8);
			accum += (int)val1 * val2;
		}

		accum += 0x00000800;
		*loopdest -= (int8)((accum >> 12) & 0xFF);
	}
}

int SonarcAudioSample::audio_decode(const uint8 *source, uint8 *dest) {
	int size = source[0] + (source[1] << 8);
	uint16 checksum = 0;
	for (int i = 0; i < size / 2; ++i) {
		uint16 val = source[2 * i] + (source[2 * i + 1] << 8);
		checksum ^= val;
	}

	if (checksum != 0xACED) return -1;

	int order = source[7];
	int mode = source[6] - 8;
	int samplecount = source[2] + (source[3] << 8);

	decode_EC(mode, samplecount,
	          source + 8 + 2 * order, size - 8 - 2 * order,
	          dest);
	decode_LPC(order, samplecount, dest, source + 8);

	// Try to fix a number of clipped samples
	for (int i = 1; i < samplecount; ++i)
		if (dest[i] == 0 && dest[i - 1] > 192) dest[i] = 0xFF;


	return 0;
}

//
// AudioSample Interface
//

void SonarcAudioSample::initDecompressor(void *DecompData) const {
	SonarcDecompData *decomp = reinterpret_cast<SonarcDecompData *>(DecompData);
	decomp->pos = src_offset;
	decomp->sample_pos = 0;
}

uint32 SonarcAudioSample::decompressFrame(void *DecompData, void *samples) const {
	SonarcDecompData *decomp = reinterpret_cast<SonarcDecompData *>(DecompData);

	if (decomp->pos == buffer_size) return 0;
	if (decomp->sample_pos == length) return 0;

	// Get Frame size
	uint32 frame_bytes  = *(buffer + decomp->pos);
	frame_bytes |= (*(buffer + decomp->pos + 1)) << 8;

	// Get Num Frame Samples
	uint32 frame_samples  = *(buffer + decomp->pos + 2);
	frame_samples |= (*(buffer + decomp->pos + 3)) << 8;

	audio_decode(buffer + decomp->pos, reinterpret_cast<uint8 *>(samples));

	decomp->pos += frame_bytes;
	decomp->sample_pos += frame_samples;

	return frame_samples;
}

void SonarcAudioSample::rewind(void *DecompData) const {
	SonarcDecompData *decomp = reinterpret_cast<SonarcDecompData *>(DecompData);
	decomp->pos = src_offset;
	decomp->sample_pos = 0;
}

} // End of namespace Pentagram
} // End of namespace Ultima8
