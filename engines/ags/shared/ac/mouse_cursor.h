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

#ifndef AGS_SHARED_AC_MOUSE_CURSOR_H
#define AGS_SHARED_AC_MOUSE_CURSOR_H

#include "ags/shared/core/types.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

#define MCF_ANIMMOVE 1
#define MCF_DISABLED 2
#define MCF_STANDARD 4
#define MCF_HOTSPOT  8  // only animate when over hotspot

// IMPORTANT: exposed to plugin API as AGSCursor!
// do not change topmost fields, unless planning breaking compatibility.
struct MouseCursor {
	int   pic = 0;
	short hotx = 0, hoty = 0;
	short view = -1;
	char  name[10]{};
	char  flags = 0;

	// up to here is a part of plugin API
	int   animdelay = 5;

	MouseCursor() {}

	void clear();
	void ReadFromFile(Shared::Stream *in);
	void WriteToFile(Shared::Stream *out);
	void ReadFromSavegame(Shared::Stream *in, int cmp_ver);
	void WriteToSavegame(Shared::Stream *out) const;
};

} // namespace AGS3

#endif
