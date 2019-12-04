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

#include "ultima8/audio/AudioChannel.h"
#include "ultima8/audio/audio_sample.h"

namespace Ultima8 {
namespace Pentagram {

// We divide the data by 2, to prevent overshots. Imagine this sample pattern:
// 0, 65535, 65535, 0. Now you want to compute a value between the two 65535.
// Obviously, it will be *bigger* than 65535 (it can get to about 80,000).
// It is possibly to clamp it, but that leads to a distored wave form. Compare
// this to turning up the volume of your stereo to much, it will start to sound
// bad at a certain level (depending on the power of your stereo, your speakers
// etc, this can be quite loud, though ;-). Hence we reduce the original range.
// A factor of roughly 1/1.2 = 0.8333 is sufficient. Since we want to avoid
// floating point, we approximate that by 27/32
#define RANGE_REDUX(x)  (((x) * 27) >> 5)

AudioChannel::AudioChannel(uint32 sample_rate_, bool stereo_) :
	playdata(0), playdata_size(0), decompressor_size(0), frame_size(0),
	sample_rate(sample_rate_), stereo(stereo_),
	loop(0), sample(0),
	frame_evenodd(0), frame0_size(0), frame1_size(0), position(0), paused(false),
	fp_pos(0), fp_speed(0) {
}

AudioChannel::~AudioChannel(void) {
}

void AudioChannel::playSample(AudioSample *sample_, int loop_, int priority_, bool paused_, uint32 pitch_shift_, int lvol_, int rvol_) {
	sample = sample_;
	loop = loop_;
	priority = priority_;
	lvol = lvol_;
	rvol = rvol_;
	paused = paused_;
	pitch_shift = pitch_shift_;

	if (!sample) return;

	// Setup buffers
	decompressor_size = sample->getDecompressorDataSize();
	frame_size = sample->getFrameSize();

	if ((decompressor_size + frame_size * 2) > playdata_size) {
		delete [] playdata;
		playdata_size = decompressor_size + frame_size * 2;
		playdata = new uint8[playdata_size];
	}

	// Init the sample decompressor
	sample->initDecompressor(playdata);

	// Reset counter and stuff
	frame_evenodd = 0;
	position = 0;
	fp_pos = 0;
	fp_speed = (pitch_shift * sample->getRate()) / sample_rate;

	// Decompress frame 0
	frame0_size = sample->decompressFrame(playdata, playdata + decompressor_size);

	// Decompress frame 1
	DecompressNextFrame();

	// Setup resampler
	if (sample->getBits() == 8 && !sample->isStereo()) {
		uint8 *src = playdata + decompressor_size;
		int a = *(src + 0);
		a = (a | (a << 8)) - 32768;
		int b = *(src + 1);
		b = (a | (b << 8)) - 32768;
		int c = *(src + 2);
		c = (a | (c << 8)) - 32768;

		interp_l.init(RANGE_REDUX(a), RANGE_REDUX(b), RANGE_REDUX(c));
	}

}

void AudioChannel::resampleAndMix(int16 *stream, uint32 bytes) {
	if (!sample || paused) return;

	// Update fp_speed
	fp_speed = (pitch_shift * sample->getRate()) / sample_rate;

	// Get and Mix data
	do {
		// 8 bit resampling
		if (sample->getBits() == 8) {
			// Mono resampling required
			if (!sample->isStereo() && stereo)
				resampleFrameM8toS(stream, bytes);
			if (!sample->isStereo() && !stereo)
				resampleFrameM8toM(stream, bytes);
		}
		// 16 bit resampling (or not)
		/*
		else
		{
		    uint8 *src = playdata + decompressor_size + (frame_size*frame_evenodd) + position;
		    uint8 *src_end = src + frame0_size;

		    do {
		        int c = *(src++);
		        c = (c|(c << 8))-32768;

		        int lresult = *(stream+0) + c;
		        int rresult = *(stream+1) + c;

		        // Enforce range in case of an "overshot". Shouldn't happen since we
		        // scale down already, but safe is safe.
		        if (lresult < -32768) lresult = -32768;
		        else if (lresult > 32767) lresult = 32767;

		        if (rresult < -32768) rresult = -32768;
		        else if (rresult > 32767) rresult = 32767;

		        *stream++ = lresult;
		        *stream++ = rresult;
		        bytes -= 4;
		    } while (bytes!=0 && src != src_end);

		    position = frame0_size - (src_end - src);
		}
		*/

		// We ran out of data
		if (bytes || (position == frame0_size)) {

			// No more data
			if (!frame1_size) {
				sample = 0;
				return;
			}

			// Invert evenodd
			frame_evenodd = 1 - frame_evenodd;

			// Set frame1 to be frame0
			frame0_size = frame1_size;
			position = 0;

			DecompressNextFrame();
		}

	} while (bytes != 0);
}

// Decompress a frame
void AudioChannel::DecompressNextFrame() {
	// Get next frame of data
	uint8 *src2 = playdata + decompressor_size + (frame_size * (1 - frame_evenodd));
	frame1_size = sample->decompressFrame(playdata, src2);

	// No stream, go back to beginning and get first frame
	if (!frame1_size && loop) {
		if (loop != -1) loop--;
		sample->rewind(playdata);
		frame1_size = sample->decompressFrame(playdata, src2);
	}
}

// Resample a frame of mono 8bit unsigned to Stereo 16bit
void AudioChannel::resampleFrameM8toS(int16 *&stream, uint32 &bytes) {
	uint8 *src = playdata + decompressor_size + (frame_size * frame_evenodd);
	uint8 *src2 = playdata + decompressor_size + (frame_size * (1 - frame_evenodd));

	uint8 *src_end = src + frame0_size;
	uint8 *src2_end = src2 + frame1_size;

	src += position;

	int result;

	do {
		// Add a new src sample (if required)
		if (fp_pos >= 0x10000) {
			if (src + 2 < src_end) {
				int c = *(src + 2);
				c = (c | (c << 8)) - 32768;
				interp_l.feedData(RANGE_REDUX(c));
			} else if (src2 < src2_end) {
				int c = *(src2);
				c = (c | (c << 8)) - 32768;
				interp_l.feedData(RANGE_REDUX(c));
				src2++;
			} else {
				interp_l.feedData();
			}
			src++;
			fp_pos -= 0x10000;
		}

		if (fp_pos < 0x10000) do {
				// Do the interpolation
				result = interp_l.interpolate(fp_pos);

				int lresult = *(stream + 0) + (result * lvol) / 256;
				int rresult = *(stream + 1) + (result * rvol) / 256;

				// Enforce range in case of an "overshot". Shouldn't happen since we
				// scale down already, but safe is safe.
				if (lresult < -32768) lresult = -32768;
				else if (lresult > 32767) lresult = 32767;

				if (rresult < -32768) rresult = -32768;
				else if (rresult > 32767) rresult = 32767;

				*stream++ = lresult;
				*stream++ = rresult;
				bytes -= 4;
				fp_pos += fp_speed;

			} while (fp_pos < 0x10000 && bytes != 0);

	} while (bytes != 0 && src != src_end);

	position = frame0_size - (src_end - src);
}

// Resample a frame of mono 8bit unsigned to Mono 16bit
void AudioChannel::resampleFrameM8toM(int16 *&stream, uint32 &bytes) {
	uint8 *src = playdata + decompressor_size + (frame_size * frame_evenodd);
	uint8 *src2 = playdata + decompressor_size + (frame_size * (1 - frame_evenodd));

	uint8 *src_end = src + frame0_size;
	uint8 *src2_end = src2 + frame1_size;

	src += position;

	int result;
	int volume = (rvol + lvol) / 2;

	do {
		// Add a new src sample (if required)
		if (fp_pos >= 0x10000) {
			if (src + 2 < src_end) {
				int c = *(src + 2);
				c = (c | (c << 8)) - 32768;
				interp_l.feedData(RANGE_REDUX(c));
			} else if (src2 < src2_end) {
				int c = *(src2);
				c = (c | (c << 8)) - 32768;
				interp_l.feedData(RANGE_REDUX(c));
				src2++;
			} else {
				interp_l.feedData();
			}
			src++;
			fp_pos -= 0x10000;
		}

		if (fp_pos < 0x10000) do {
				// Do the interpolation
				result = (interp_l.interpolate(fp_pos) * volume) / 256;

				result += *stream;

				// Enforce range in case of an "overshot". Shouldn't happen since we
				// scale down already, but safe is safe.
				if (result < -32768) result = -32768;
				else if (result > 32767) result = 32767;

				*stream++ = result;
				bytes -= 2;
				fp_pos += fp_speed;

			} while (fp_pos < 0x10000 && bytes != 0);

	} while (bytes != 0 && src != src_end);

	position = frame0_size - (src_end - src);
}

} // End of namespace Pentagram
} // End of namespace Ultima8
