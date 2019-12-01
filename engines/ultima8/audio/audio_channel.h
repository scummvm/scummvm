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

#ifndef ULTIMA8_AUDIO_AUDIOCHANNEL_H
#define ULTIMA8_AUDIO_AUDIOCHANNEL_H

namespace Ultima8 {
namespace Pentagram {

class AudioSample;

class AudioChannel {
	// We have:
	// 1x decompressor size
	// 2x frame size
	uint8           *playdata;          //
	uint32          playdata_size;
	uint32          decompressor_size;  // Persistent data for the decompressor
	uint32          frame_size;         //

	uint32          sample_rate;
	bool            stereo;

	int32           loop;
	AudioSample     *sample;

	// Info for sampling
	uint32          frame_evenodd;  // which buffer is 'frame0'
	uint32          frame0_size;    // Size of the frame0 buffer in samples
	uint32          frame1_size;    // Size of the frame1 buffer in samples
	uint32          position;       // Position in frame0 buffer
	int             lvol, rvol;     // 0-256
	uint32          pitch_shift;    // 0x10000 = no shift
	int             priority;       // anything.
	bool            paused;         // true/false

public:
	AudioChannel(uint32 sample_rate, bool stereo);
	~AudioChannel(void);

	void stop() {
		sample = 0;
	}

	void playSample(AudioSample *sample, int loop, int priority, bool paused, uint32 pitch_shift, int lvol, int rvol);
	void resampleAndMix(int16 *stream, uint32 bytes);

	bool isPlaying() {
		return sample != 0;
	}

	void setPitchShift(int pitch_shift_) {
		pitch_shift = pitch_shift_;
	}
	uint32 getPitchShift() const {
		return pitch_shift;
	}

	void setLoop(int loop_) {
		loop = loop_;
	}
	int32 getLoop() const {
		return loop;
	}

	void setVolume(int lvol_, int rvol_) {
		lvol = lvol_;
		rvol = rvol_;
	}
	void getVolume(int &lvol_, int &rvol_) const {
		lvol_ = lvol;
		rvol_ = rvol;
	}

	void setPriority(int priority_) {
		priority = priority_;
	}
	int getPriority() const {
		return priority;
	}

	void setPaused(bool paused_) {
		paused = paused_;
	}
	bool isPaused() const {
		return paused;
	}
private:

	//
	void DecompressNextFrame();

	//
	// Resampling
	//
	class CubicInterpolator {
	protected:
		int x0, x1, x2, x3;
		int a, b, c, d;

	public:
		CubicInterpolator() : x0(0), x1(0), x2(0), x3(0) {
			updateCoefficients();
		}

		CubicInterpolator(int a0, int a1, int a2, int a3) : x0(a0), x1(a1), x2(a2), x3(a3) {
			updateCoefficients();
		}

		CubicInterpolator(int a1, int a2, int a3) : x0(2 * a1 - a2), x1(a1), x2(a2), x3(a3) {
			// We use a simple linear interpolation for x0
			updateCoefficients();
		}

		inline void init(int a0, int a1, int a2, int a3) {
			x0 = a0;
			x1 = a1;
			x2 = a2;
			x3 = a3;
			updateCoefficients();
		}

		inline void init(int a1, int a2, int a3) {
			// We use a simple linear interpolation for x0
			x0 = 2 * a1 - a2;
			x1 = a1;
			x2 = a2;
			x3 = a3;
			updateCoefficients();
		}

		inline void feedData() {
			x0 = x1;
			x1 = x2;
			x2 = x3;
			x3 = 2 * x2 - x1; // Simple linear interpolation
			updateCoefficients();
		}

		inline void feedData(int xNew) {
			x0 = x1;
			x1 = x2;
			x2 = x3;
			x3 = xNew;
			updateCoefficients();
		}

		/* t must be a 16.16 fixed point number between 0 and 1 */
		inline int interpolate(uint32 fp_pos) {
			int result = 0;
			int t = fp_pos >> 8;
			result = (a * t + b) >> 8;
			result = (result * t + c) >> 8;
			result = (result * t + d) >> 8;
			result = (result / 3 + 1) >> 1;

			return result;
		}

	protected:
		inline void updateCoefficients() {
			a = ((-x0 * 2) + (x1 * 5) - (x2 * 4) + x3);
			b = ((x0 + x2 - (2 * x1)) * 6) << 8;
			c = ((-4 * x0) + x1 + (x2 * 4) - x3) << 8;
			d = (x1 * 6) << 8;
		}
	};

	// Resampler stuff
	CubicInterpolator   interp_l;
	CubicInterpolator   interp_r;
	int                 fp_pos;
	int                 fp_speed;

	void resampleFrameM8toS(int16 *&samples, uint32 &bytes);
	void resampleFrameM8toM(int16 *&samples, uint32 &bytes);



};

} // End of namespace Pentagram
} // End of namespace Ultima8

#endif
