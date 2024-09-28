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

#ifndef COMMON_ROTATIONMODE_H
#define COMMON_ROTATIONMODE_H

#include "common/scummsys.h"

namespace Common {

/**
 * @defgroup common_rotationmode Rotation modes
 * @ingroup common
 *
 * @brief API for rotation modes.
 *
 * @{
 */

class String;

/**
 * List of rotation modes.
 *
 */
enum RotationMode {
	kRotationNormal = 0,
	kRotation90 = 90,
	kRotation180 = 180,
	kRotation270 = 270,
};

struct RotationModeDescription {
	const char *description;
	RotationMode id;
};

extern const RotationModeDescription g_rotationModes[];

extern RotationMode parseRotationMode(int val);

}

#endif
