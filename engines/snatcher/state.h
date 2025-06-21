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

#ifndef SNATCHER_STATE_H
#define SNATCHER_STATE_H

#include "common/rect.h"
#include "common/scummsys.h"

namespace Snatcher {

class SnatcherEngine;
class SceneModule;
class FIO;

struct Config {
	Config() : volumeMusic(0), volumeSFX(0), volumeSpeech(0), lightGunAvailable(1), useLightGun(0), controllerSetup(0), disableStereo(0), hasRAMCart(1), hasFreeBuram(1), lightGunBias(0, -16) {}
	int16 volumeMusic;
	int16 volumeSFX;
	int16 volumeSpeech;
	int16 lightGunAvailable;
	int16 useLightGun;
	int16 controllerSetup;
	int16 disableStereo;
	int16 hasRAMCart;
	int16 hasFreeBuram;
	Common::Point lightGunBias;
};

struct Script {
	Script(uint32 textResourceOffset, int16 &chaptr) : sentenceDone(0), sentencePos(0), data(nullptr), dataSize(0), curPos(0), newPos(0xFFFF), curFileNo(0), curGfxScript(-1), phase(chaptr), textResOffset(textResourceOffset) {}
	~Script() {
		delete[] data;
	}
	const uint8 *getTextResource() const {
		return data + textResOffset;
	}
	uint8 sentenceDone;
	uint8 sentencePos;
	uint8 curFileNo;
	int16 curGfxScript;
	const uint8 *data;
	uint32 dataSize;
	uint16 curPos;
	uint16 newPos;
	int16 &phase;
	const uint32 textResOffset;
};

struct GameState {
	GameState() : frameNo(0), frameState(0), finish(0), modProcessTop(0), modProcessSub(0), counter(0), modIndex(0), menuSelect(0), prologue(0), phase(0), phaseFlags(0), saveSlotUsage(0), saveCount(0), conf(Config()), script(0x3800, phase), totalPlayTime(0) {}
	int16 frameNo;
	int16 frameState;
	int16 finish;
	int16 modProcessTop;
	int16 modProcessSub;
	int16 counter;
	int16 modIndex;
	int16 menuSelect;
	int16 prologue;
	int16 phase;
	int16 phaseFlags;
	int16 saveSlotUsage;
	int16 saveCount;
	Config conf;
	Script script;
	uint32 totalPlayTime;
};

} // End of namespace Snatcher

#endif // SNATCHER_STATE_H
