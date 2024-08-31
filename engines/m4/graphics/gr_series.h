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

#ifndef M4_GRAPHICS_GR_SERIES_H
#define M4_GRAPHICS_GR_SERIES_H

#include "m4/m4_types.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {

constexpr uint32 SERIES_FORWARD = 0;
constexpr uint32 SERIES_PINGPONG = 1;
constexpr uint32 SERIES_BACKWARD = 2;
constexpr uint32 SERIES_RANDOM = 4;			// series is played in random order, trigger after number of frames in range played
constexpr uint32 SERIES_NO_TOSS = 8;		// series is not tossed at the end of playing
constexpr uint32 SERIES_STICK = 16;			// series sticks on last frame, then sends trigger
constexpr uint32 SERIES_LOOP_TRIGGER = 32;	// get trigger back every loop
constexpr uint32 SERIES_LOAD_PALETTE = 64;	// load master_palette with colours?
constexpr uint32 SERIES_HORZ_FLIP = 128;	// horizontal flip

// Old constants
constexpr uint32 FORWARD = 0;
constexpr uint32 PINGPONG = 1;
constexpr uint32 BACKWARD = 2;
constexpr uint32 STICK = 4;
constexpr uint32 NO_TOSS = 8;

enum {
	HASH_SERIES_PLAY_MACHINE = 0,
	HASH_SERIES_SHOW_MACHINE = 1,
	// HASH_TIMER_MACHINE = 2,  // defined in adv.h

	HASH_STREAM_MACHINE = 6
};

/**
 * Since series are normally started in pairs, this simplifies doing so
 */
struct Series {
	machine *_series = nullptr;
	machine *_seriesS = nullptr;

	void play(const char *seriesName, frac16 layer, uint32 flags = 0,
		int16 triggerNum = -1, int32 frameRate = 6, int32 loopCount = 0, int32 s = 100,
		int32 x = 0, int32 y = 0, int32 firstFrame = 0, int32 lastFrame = -1);
	void show(const char *seriesName, frac16 layer, uint32 flags = 0,
		int16 triggerNum = -1, int32 duration = -1, int32 index = 0, int32 s = 100,
		int32 x = 0, int32 y = 0);
	void show(const char *series1, const char *series2, int layer);
	void show_index2(const char *series1, const char *series2, int layer, int index1, int index2);
	void terminate();

	operator bool() const {
		return _series != nullptr;
	}

	machine *&operator[](uint idx) {
		return (idx == 0) ? _series : _seriesS;
	}

	frac16 *regs() const {
		return _series->myAnim8->myRegs;
	}

	static void series_play(const char *seriesName, frac16 layer, uint32 flags = 0,
		int16 triggerNum = -1, int32 frameRate = 6, int32 loopCount = 0, int32 s = 100,
		int32 x = 0, int32 y = 0, int32 firstFrame = 0, int32 lastFrame = -1);
	static void series_show(const char *seriesName, frac16 layer, uint32 flags = 0,
		int16 triggerNum = -1, int32 duration = -1, int32 index = 0, int32 s = 100,
		int32 x = 0, int32 y = 0);
};

int32 series_load(const char *seriesName, int32 assetIndex = -1, RGB8 *myPal = nullptr);
void series_unload(int32 assetIndex);
bool series_draw_sprite(int32 spriteHash, int32 index, Buffer *destBuff, int32 x, int32 y);

bool series_show_frame(int32 spriteHash, int32 index, Buffer *destBuff, int32 x, int32 y);
machine *series_place_sprite(const char *seriesName, int32 index, int32 x, int32 y, int32 s, int32 layer);
machine *series_show_sprite(const char *seriesName, int32 index, int32 layer);

machine *series_play(const char *seriesName, frac16 layer, uint32 flags = 0,
	int16 triggerNum = -1, int32 frameRate = 6, int32 loopCount = 0, int32 s = 100,
	int32 x = 0, int32 y = 0, int32 firstFrame = 0, int32 lastFrame = -1);
machine *series_simple_play(const char *seriesName, frac16 layer, bool stickWhenDone);

machine *series_show(const char *seriesName, frac16 layer, uint32 flags = 0,
	int16 triggerNum = -1, int32 duration = -1, int32 index = 0, int32 s = 100,
	int32 x = 0, int32 y = 0);

machine *series_ranged_play(const char *seriesName, int32 loopCount, uint32 flags,
	int32 firstFrame, int32 lastFrame, int32 s, uint32 layer,
	int32 frameRate, int32 trigger = -1, bool stick_when_done = false);
machine *series_ranged_play_xy(const char *seriesName, int loopCount, int flags,
	int firstFrame, int lastFrame, int x, int y, int s, int layer,
	int frameRate, int trigger = -1, bool stick_when_done = false);
machine *series_plain_play(const char *seriesName, int32 loopCount, uint32 flags,
	int32 s, int32 layer, int32 frameRate, int32 trigger = -1, bool stickWhenDone = false);
machine *series_play_xy(const char *seriesName, int loopCount, int flags,
	int x, int y, int scale, int depth, int layer, int frameRate);

machine *series_stream(const char *seriesName, int32 frameRate, int32 layer, int32 trigger);
bool series_stream_break_on_frame(machine *m, int32 frameNum, int32 trigger);
void series_set_frame_rate(machine *m, int32 newFrameRate);
void series_stream_check_series(machine *m, int val);

} // namespace M4

#endif
