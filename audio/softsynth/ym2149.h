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

#ifndef AUDIO_SOFTSYNTH_YM2149_H
#define AUDIO_SOFTSYNTH_YM2149_H

#include "audio/ym2149.h"

namespace Audio {

class YM2149Emu : public YM2149::YM2149, public EmulatedChip {
public:
	YM2149Emu();
	~YM2149Emu() override;

	bool init() override;
	void reset() override;
	void writeReg(int reg, uint8 value) override;
	bool isStereo() const override { return false; }

protected:
	void generateSamples(int16 *buffer, int numSamples) override;

private:
	static const int YM_ATARI_CLOCK = 2000000;
	static const int YM_ATARI_CLOCK_COUNTER = (YM_ATARI_CLOCK / 8);

	static const int YM_BUFFER_250_SIZE = 32768;
	static const int YM_BUFFER_250_SIZE_MASK = (YM_BUFFER_250_SIZE - 1);

	static const uint32 YmVolume4to5[32];

	static const int ENV_GODOWN = 0;
	static const int ENV_GOUP = 1;
	static const int ENV_DOWN = 2;
	static const int ENV_UP = 3;
	static const int YmEnvDef[16][3];

	static uint16 YmEnvWaves[16][32 * 3];
	static const uint16 volumeTable[16][16][16];
	static uint16 ymout5_u16[32][32][32];
	static int16 *ymout5;
	static bool _tablesBuilt;

	static const uint16 YM_MASK_1VOICE = 0x1f;
	static const uint16 YM_MASK_A = 0x1f;
	static const uint16 YM_MASK_B = (0x1f << 5);
	static const uint16 YM_MASK_C = (0x1f << 10);

	static const uint16 YM_SQUARE_UP = 0x1f;
	static const uint16 YM_SQUARE_DOWN = 0x00;

	uint16 _toneAPer, _toneACount, _toneAVal;
	uint16 _toneBPer, _toneBCount, _toneBVal;
	uint16 _toneCPer, _toneCCount, _toneCVal;
	uint16 _noisePer, _noiseCount, _noiseVal;
	uint16 _envPer, _envCount;

	uint32 _envPos;
	int _envShape;

	uint32 _mixerTA, _mixerTB, _mixerTC;
	uint32 _mixerNA, _mixerNB, _mixerNC;

	uint32 _rndRack;
	uint16 _freqDiv2;

	uint16 _envMask3Voices;
	uint16 _vol3Voices;

	uint8 _soundRegs[14];

	int16 _YMBuffer250[YM_BUFFER_250_SIZE];
	int _YMBuffer250PosWrite;
	int _YMBuffer250PosRead;

	uint32 _posFractWeightedN;
	int _rate;

	void setOutputRate(int outputRate);
	void generate(int16 *dst, int count);
	static uint16 mergeVoice(uint16 c, uint16 b, uint16 a);
	static void envBuild();
	static void interpolateVolumetable(uint16 volumetable[32][32][32]);
	static void normalise5bitTable(uint16 *in5bit, int16 *out5bit, unsigned int level);
	static void initOnce();
	static uint16 tonePer(uint8 rHigh, uint8 rLow);
	static uint16 noisePer(uint8 rNoise);
	static uint16 envPer(uint8 rHigh, uint8 rLow);
	uint32 rndCompute();
	void doSamples250(int samplesToGenerate250);
	int16 nextSample();
};

} // End of namespace Audio

#endif
