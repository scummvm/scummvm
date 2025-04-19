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

#ifndef AWE_SFX_PLAYER_H
#define AWE_SFX_PLAYER_H

#include "awe/intern.h"

namespace Awe {

struct SfxInstrument {
	uint8_t *data = nullptr;
	uint16_t volume = 0;
};

struct SfxModule {
	const uint8_t *data = nullptr;
	uint16_t curPos = 0;
	uint8_t curOrder = 0;
	uint8_t numOrder = 0;
	uint8_t *orderTable = nullptr;
	SfxInstrument samples[15];
};

struct SfxPattern {
	uint16_t note_1 = 0;
	uint16_t note_2 = 0;
	uint16_t sampleStart = 0;
	uint8_t *sampleBuffer = nullptr;
	uint16_t sampleLen = 0;
	uint16_t loopPos = 0;
	uint16_t loopLen = 0;
	uint16_t sampleVolume = 0;
};

struct SfxChannel {
	uint8_t *sampleData = nullptr;
	uint16_t sampleLen = 0;
	uint16_t sampleLoopPos = 0;
	uint16_t sampleLoopLen = 0;
	uint16_t volume = 0;
	Frac pos;
};

struct Resource;

struct SfxPlayer {
	enum {
		NUM_CHANNELS = 4
	};

	Resource *_res;

	uint16_t _delay;
	uint16_t _resNum;
	SfxModule _sfxMod;
	int16_t *_syncVar;
	bool _playing;
	int _rate;
	int _samplesLeft;
	SfxChannel _channels[NUM_CHANNELS];

	SfxPlayer(Resource *res);

	void setEventsDelay(uint16_t delay);
	void loadSfxModule(uint16_t resNum, uint16_t delay, uint8_t pos);
	void prepareInstruments(const uint8_t *p);
	void play(int rate);
	void mixSamples(int16_t *buf, int len);
	void readSamples(int16_t *buf, int len);
	void start();
	void stop();
	void handleEvents();
	void handlePattern(uint8_t channel, const uint8_t *patternData);
};

} // namespace Awe

#endif
