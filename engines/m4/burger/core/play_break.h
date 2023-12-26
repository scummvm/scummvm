
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

#ifndef M4_BURGER_PLAY_BREAK_H
#define M4_BURGER_PLAY_BREAK_H

#include "m4/m4_types.h"
#include "m4/adv_r/adv_trigger.h"

namespace M4 {
namespace Burger {

struct seriesPlayBreak {
	int32 firstFrame;
	int32 lastFrame;
	const char *sound;
	int32 channel;
	int32 volume;
	int32 trigger;
	uint32 flags;
	int32 loopCount;
	int32 *variable;
	int32 value;
};

enum {
	WITH_SHADOW		= 1,
	PRELOAD_SOUNDS	= 2
};

#define PLAY_BREAK_END { -1, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 }

int32 series_play_with_breaks(const seriesPlayBreak list[], const char *name,
	frac16 depth = 0, int32 trigger = NO_TRIGGER, uint32 flags = 0, int32 framerate = 6,
	int32 scale = 100, int32 x = 0, int32 y = 0);
void digi_preload_play_breaks(const seriesPlayBreak list[], bool loadAll = false);
void digi_unload_play_breaks(const seriesPlayBreak list[]);
void handle_series_play_break(int32 slot);

} // namespace Burger
} // namespace M4

#endif
