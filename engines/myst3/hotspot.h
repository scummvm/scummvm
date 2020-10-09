/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HOTSPOT_H_
#define HOTSPOT_H_

#include "common/rect.h"
#include "common/array.h"

namespace Myst3 {

class GameState;

struct Opcode {
	uint8 op;
	Common::Array<int16> args;
};

struct CondScript {
	uint16 condition;
	Common::Array<Opcode> script;
};

struct PolarRect {
	int16 centerPitch;
	int16 centerHeading;
	int16 height;
	int16 width;
};

class HotSpot {
public:
	HotSpot();

	int16 condition;
	Common::Array<PolarRect> rects;
	int16 cursor;
	Common::Array<Opcode> script;

	int32 isPointInRectsCube(float pitch, float heading);
	int32 isPointInRectsFrame(GameState *state, const Common::Point &p);
	bool isEnabled(GameState *state, uint16 var = 0);

private:
	bool isZip() { return cursor == 7; }
	int32 isZipDestinationAvailable(GameState *state);
};


} // End of namespace Myst3

#endif // HOTSPOT_H_
