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

#include "mads/madsv2/console.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"

namespace MADS {
namespace MADSV2 {

Console::Console() : GUI::Debugger() {
	registerCmd("depth", WRAP_METHOD(Console, cmdDepth));
	registerCmd("teleport", WRAP_METHOD(Console, cmdTeleport));
	registerCmd("walkable", WRAP_METHOD(Console, cmdWalkable));
}

static int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

bool Console::cmdDepth(int argc, const char **argv) {
	const byte *srcP = scr_depth.data;
	byte *destP = scr_orig.data;

	for (int i = 0; i < scr_orig.x * scr_orig.y; i += 2, ++srcP) {
		*destP++ = *srcP >> 4;
		*destP++ = *srcP & 0xf;
	}

	matte_refresh_work();
	return false;
}

bool Console::cmdTeleport(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Current room is: %d\n", new_room);
		debugPrintf("Usage: %s <room number> [<previous room>]\n", argv[0]);
		return true;
	} else {
		if (argc >= 3)
			room_id = strToInt(argv[2]);
		new_room = strToInt(argv[1]);

		return false;
	}
}

bool Console::cmdWalkable(int argc, const char **argv) {
	const byte *srcP = scr_walk.data - 1;
	byte *destP = scr_orig.data;
	assert((scr_walk.x * 8) == scr_orig.x);

	// Draw the walkable areas
	for (int i = 0; i < scr_orig.x * scr_orig.y; ++i, ++destP) {
		if ((i % 8) == 0)
			++srcP;
		if (!(*srcP & (1 << (7 - (i % 8)))))
			*destP = 10;
	}

	// Draw cross-hairs at the locations of the rails within the room
	for (int i = 0; i < room->num_rails; ++i) {
		const Rail &r = room->rail[i];

		buffer_hline(scr_orig, r.x - 2, r.x + 2, r.y, 0);
		buffer_vline(scr_orig, r.x, r.y - 2, r.y + 2, 0);
	}

	matte_refresh_work();
	return false;
}

} // namespace MADSV2
} // namespace MADS
