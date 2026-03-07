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

#ifndef AUDIO_ATARI_STYM_STREAM_H
#define AUDIO_ATARI_STYM_STREAM_H

#include "audio/audiostream.h"
#include "common/array.h"
#include "common/ptr.h"
#include "common/stream.h"

namespace Audio {

class ElviraPrgDriver;

class AtariSTYMStream final : public AudioStream {
public:
	AtariSTYMStream(Common::SeekableReadStream *stream, uint32 outputRate);
	AtariSTYMStream(Common::SeekableReadStream *stream, uint32 outputRate, uint16 elvira1Tune);
	~AtariSTYMStream() override;

	bool isValid() const {
		return _isValid;
	}

	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override {
		return false;
	}
	int getRate() const override {
		return (int)_rate;
	}
	bool endOfData() const override {
		return _ended && _samplesLeftInWait == 0;
	}

private:
	friend class ElviraPrgDriver;

	static const uint32 kDefaultFrameHz = 25;
	static const uint32 kElvira1PrgFrameHz = 50;

	Common::ScopedPtr<Common::SeekableReadStream> _stream;
	Common::Array<uint8> _data;
	size_t _pos = 0;

	uint32 _rate = 44100;
	uint32 _samplesPerFrame = 882;
	uint32 _frameHz = kDefaultFrameHz;
	uint32 _samplesLeftInWait = 0;
	bool _ended = false;
	uint16 _elvira1Tune = 1;
	ElviraPrgDriver *_elviraPrgDriver = nullptr;
	bool _isValid = true;

	void resetSynth();

	enum Mode {
		kModeElvira2PKD,
		kModeElvira1PRG,
	};

	Mode _mode = kModeElvira2PKD;

	static uint16 readBE16(const uint8 *p) {
		return (uint16)((uint16)p[0] << 8) | (uint16)p[1];
	}
	static uint32 readBE32(const uint8 *p) {
		return ((uint32)p[0] << 24) | ((uint32)p[1] << 16) | ((uint32)p[2] << 8) | (uint32)p[3];
	}
	static void writeBE32(uint8 *p, uint32 v) {
		p[0] = (uint8)(v >> 24);
		p[1] = (uint8)(v >> 16);
		p[2] = (uint8)(v >> 8);
		p[3] = (uint8)(v);
	}

	void parseUntilWait();

	typedef int64 yms64;
	typedef int8 yms8;
	typedef int16 yms16;
	typedef int32 yms32;
	typedef uint8 ymu8;
	typedef uint16 ymu16;
	typedef uint32 ymu32;
	typedef yms16 ymsample;

	static const int YM_ATARI_CLOCK = 2000000;
	static const int YM_ATARI_CLOCK_COUNTER = (YM_ATARI_CLOCK / 8);

	static const int YM_BUFFER_250_SIZE = 32768;
	static const int YM_BUFFER_250_SIZE_MASK = (YM_BUFFER_250_SIZE - 1);

	static const ymu32 YmVolume4to5[32];

	static const int ENV_GODOWN = 0;
	static const int ENV_GOUP = 1;
	static const int ENV_DOWN = 2;
	static const int ENV_UP = 3;
	static const int YmEnvDef[16][3];

	static ymu16 YmEnvWaves[16][32 * 3];

	static const ymu16 volumeTable[16][16][16];
	static ymu16 ymout5_u16[32][32][32];
	static yms16 *ymout5;
	static bool _tablesBuilt;

	static const ymu16 YM_MASK_1VOICE = 0x1f;
	static const ymu16 YM_MASK_A = 0x1f;
	static const ymu16 YM_MASK_B = (0x1f << 5);
	static const ymu16 YM_MASK_C = (0x1f << 10);

	static const ymu16 YM_SQUARE_UP = 0x1f;
	static const ymu16 YM_SQUARE_DOWN = 0x00;

	ymu16 _toneAPer = 1, _toneACount = 0, _toneAVal = YM_SQUARE_UP;
	ymu16 _toneBPer = 1, _toneBCount = 0, _toneBVal = YM_SQUARE_UP;
	ymu16 _toneCPer = 1, _toneCCount = 0, _toneCVal = YM_SQUARE_UP;
	ymu16 _noisePer = 1, _noiseCount = 0, _noiseVal = 0;
	ymu16 _envPer = 1, _envCount = 0;

	ymu32 _envPos = 0;
	int _envShape = 0;

	ymu32 _mixerTA = 0, _mixerTB = 0, _mixerTC = 0;
	ymu32 _mixerNA = 0, _mixerNB = 0, _mixerNC = 0;

	ymu32 _rndRack = 1;
	ymu16 _freqDiv2 = 0;

	ymu16 _envMask3Voices = 0;
	ymu16 _vol3Voices = 0;

	uint8 _soundRegs[14];

	ymsample _YMBuffer250[YM_BUFFER_250_SIZE];
	int _YMBuffer250PosWrite = 0;
	int _YMBuffer250PosRead = 0;

	uint32 _posFractWeightedN = 0;

	void setOutputRate(int outputRate);
	void reset();
	void writeReg(int reg, uint8 data);
	void generate(int16 *dst, int count);

	static ymu16 mergeVoice(ymu16 c, ymu16 b, ymu16 a);
	static void envBuild();
	static void interpolateVolumetable(ymu16 volumetable[32][32][32]);
	static void normalise5bitTable(ymu16 *in5bit, yms16 *out5bit, unsigned int level);
	static void initOnce();
	static ymu16 tonePer(ymu8 rHigh, ymu8 rLow);
	static ymu16 noisePer(ymu8 rNoise);
	static ymu16 envPer(ymu8 rHigh, ymu8 rLow);

	ymu32 rndCompute();
	void doSamples250(int samplesToGenerate250);
	ymsample nextSample();
};

} // namespace Audio

#endif
