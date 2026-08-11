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

#ifndef AUDIO_SOFTSYNTH_AY8912_H
#define AUDIO_SOFTSYNTH_AY8912_H

#include "audio/chip.h"
#include "common/mutex.h"

namespace Audio {

class AY8912Stream : public EmulatedChip {
public:
	enum ChipType {
		AY_TYPE_AY,
		AY_TYPE_YM
	};

	enum StereoType {
		AY_MONO = 0,
		AY_ABC,
		AY_ACB,
		AY_BAC,
		AY_BCA,
		AY_CAB,
		AY_CBA
	};

	AY8912Stream(int rate = 44100, int chipFreq = 1773400);
	~AY8912Stream();

	// AudioStream interface
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override { return true; }
	bool endOfData() const override { return false; }
	bool endOfStream() const override { return false; }
	int getRate() const override { return _rate; }

	void setReg(int reg, unsigned char value);
	void setRegs(const unsigned char *regs);

	AudioStream *toAudioStream() { return this; }

protected:
	// EmulatedChip interface
	void generateSamples(int16 *buffer, int numSamples) override;

private:
	struct RegData {
		int tone_a;
		int tone_b;
		int tone_c;
		int noise;
		int R7_tone_a;
		int R7_tone_b;
		int R7_tone_c;
		int R7_noise_a;
		int R7_noise_b;
		int R7_noise_c;
		int vol_a;
		int vol_b;
		int vol_c;
		int env_a;
		int env_b;
		int env_c;
		int env_freq;
		int env_style;
	};

	Common::Mutex _mutex;
	int _rate;
	int _chipFreq;

	// Emulator state
	int _table[32];
	int _eq[6];
	RegData _regs;

	int _bit_a, _bit_b, _bit_c, _bit_n;
	int _cnt_a, _cnt_b, _cnt_c, _cnt_n, _cnt_e;
	int _chipTactsPerOutcount;
	int _ampGlobal;
	int _vols[6][32];
	int _envPos;
	unsigned int _curSeed;

	void prepareGeneration();
	static void genEnv();
	static bool _envGenInit;
	static int _envelope[16][128];
};

} // End of namespace Audio

#endif // AUDIO_SOFTSYNTH_AY8912_H
