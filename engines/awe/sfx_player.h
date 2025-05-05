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
	uint8 *data = nullptr;
	uint16 volume = 0;

	void clear();
};

struct SfxModule {
	const uint8 *data = nullptr;
	uint16 curPos = 0;
	uint8 curOrder = 0;
	uint8 numOrder = 0;
	uint8 *orderTable = nullptr;
	SfxInstrument samples[15];

	void clear();
	void clearSamples();
};

struct SfxPattern {
	uint16 note_1 = 0;
	uint16 note_2 = 0;
	uint16 sampleStart = 0;
	uint8 *sampleBuffer = nullptr;
	uint16 sampleLen = 0;
	uint16 loopPos = 0;
	uint16 loopLen = 0;
	uint16 sampleVolume = 0;
};

struct SfxChannel {
	uint8 *sampleData = nullptr;
	uint16 sampleLen = 0;
	uint16 sampleLoopPos = 0;
	uint16 sampleLoopLen = 0;
	uint16 volume = 0;
	Frac pos;
};

struct Resource;

struct SfxPlayer {
	enum {
		NUM_CHANNELS = 4
	};

	Resource *_res;

	uint16 _delay = 0;
	uint16 _resNum = 0;
	SfxModule _sfxMod;
	int16 *_syncVar = nullptr;
	bool _playing = false;
	int _rate = 0;
	int _samplesLeft = 0;
	SfxChannel _channels[NUM_CHANNELS];

	SfxPlayer(Resource *res);

	void setEventsDelay(uint16 delay);
	void loadSfxModule(uint16 resNum, uint16 delay, uint8 pos);
	void prepareInstruments(const uint8 *p);
	void play(int rate);
	void mixSamples(int16 *buf, int len);
	void readSamples(int16 *buf, int len);
	void start();
	void stop();
	void handleEvents();
	void handlePattern(uint8 channel, const uint8 *patternData);
};

} // namespace Awe

#endif
