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

#ifndef CHEWY_ROOMS_ROOM0_H
#define CHEWY_ROOMS_ROOM0_H

namespace Chewy {
namespace Rooms {

/**
 * Prison Cell - Starting room
 */
class Room0 {
private:
	/**
	 * Do the basic eye animation.
	 */
	enum EyeMode { EYE_START = 0, EYE_END = 1 };
	static void eyeStart(EyeMode mode);

	/**
	 * Pause after the eye arrives
	 */
	static void eyeWait();

	/**
	 * Do the eye shooting animation
	 */
	static void eyeShoot();

	static void eyeSlimeBack();

	static void checkSlimeEye();

	static void feederStart(int16 mode);

	static void checkFeed();
	static void checkPillow();

	static void feederExtend();

	static void trapDoorOpen();
	static void trapDoorClose();

	static void calcEyeClick(int16 aniNr);

	static void calcPillowClick(int16 aniNr);
public:
	static void entry();
	static bool timer(int16 timerNr, int16 aniNr);

	static void eyeAnim();
	static void feederAni();
	static bool getPillow();
	static bool pullSlime();
};

} // namespace Rooms
} // namespace Chewy

#endif
