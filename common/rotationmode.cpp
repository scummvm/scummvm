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

#include "common/rotationmode.h"

#include "common/gui_options.h"
#include "common/str.h"
#include "common/translation.h"


namespace Common {


const RotationModeDescription g_rotationModes[] = {
	{ _s("No rotation"), kRotationNormal },
	{ _s("Clockwise"), kRotation90 },
	{ _s("180 degrees"), kRotation180 },
	{ _s("Counter-clockwise"), kRotation270 },
	{ nullptr, kRotationNormal}
};

RotationMode parseRotationMode(int val) {
	if ((val % 90) != 0 || val < 0 || val > 270)
		return kRotationNormal;

	return static_cast<RotationMode>(val);
}
}
