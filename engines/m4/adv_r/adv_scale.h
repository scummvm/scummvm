
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

#ifndef M4_ADV_R_ADV_SCALE_H
#define M4_ADV_R_ADV_SCALE_H

#include "m4/m4_types.h"

namespace M4 {

constexpr int kScaleEditor = 1;

struct ADVScale_Globals {
	int32 _old_front = -1;
	int32 _old_back = -1;
	int _myback = -1;
	int _myfront = -1;
	int _mybs = -1;
	int _myfs = -1;
};

void scale_editor_draw();
void scale_editor_cancel();
void scale_editor_toggle();

} // End of namespace M4

#endif
