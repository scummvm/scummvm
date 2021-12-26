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

#ifndef AGS_ENGINE_GUI_ANIMATING_GUI_BUTTON_H
#define AGS_ENGINE_GUI_ANIMATING_GUI_BUTTON_H

#include "ags/engine/ac/runtime_defines.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
}
}
using namespace AGS; // FIXME later

struct AnimatingGUIButton {
	// index into _GP(guibuts) array, GUI, button
	short buttonid = 0, ongui = 0, onguibut = 0;
	// current animation status
	short view = 0, loop = 0, frame = 0;
	short speed = 0, repeat = 0, wait = 0;

	void ReadFromFile(Shared::Stream *in);
	void WriteToFile(Shared::Stream *out);
};

} // namespace AGS3

#endif
