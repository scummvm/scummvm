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

#ifndef COMMON_H
#define COMMON_H

namespace Alcachofa {

enum class CursorType {
	Normal,
	LookAt,
	Use,
	GoTo,
	LeaveUp,
	LeaveRight,
	LeaveDown,
	LeaveLeft
};

enum class Direction {
	Up,
	Down,
	Left,
	Right
};

constexpr const int32 kDirectionCount = 4;
constexpr const int8 kOrderCount = 70;
constexpr const int8 kForegroundOrderCount = 10;

struct Color {
	uint8 r, g, b, a;
};
static constexpr const Color kWhite = { 255, 255, 255, 255 };
static constexpr const Color kBlack = { 0, 0, 0, 255 };
static constexpr const Color kClear = { 0, 0, 0, 0 };
static constexpr const Color kDebugRed = { 250, 0, 0, 70 };
static constexpr const Color kDebugGreen = { 0, 255, 0, 85 };
static constexpr const Color kDebugBlue = { 0, 0, 255, 110 };

}

#endif // ALCACHOFA_COMMON_H
