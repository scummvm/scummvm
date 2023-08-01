
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

#ifndef M4_BURGER_WILBUR_H
#define M4_BURGER_WILBUR_H

#include "m4/m4_types.h"

namespace M4 {
namespace Burger {

struct Wilbur_Globals {
	const char *_name = nullptr;
	int _channel = 0;
	int _room = 0;
	int _vol = 0;
	int32 _trigger = 0;
	bool _flag = false;
};

extern void reset_walker_sprites();
extern bool load_walker_sprites();

extern void wilbur_speech(const char *name, int trigger, int room = -1, byte flags = 0,
	int vol = 256, int channel = 1);
extern void wilbur_say();
extern void wilburs_speech_finished();

} // namespace Burger
} // namespace M4

#endif
