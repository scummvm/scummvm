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

#include "efh/efh.h"

namespace Efh {

void EfhEngine::generateSound1(int arg0, int arg2, int duration) {
	warning("STUB: generateSound1 %d %d %d", arg0, arg2, duration);
}

void EfhEngine::generateSound2(int startFreq, int endFreq, int arg4) {
	warning("STUB: generateSound2 %d %d %d", startFreq, endFreq, arg4);

	// Arg4 doesn't seem to be used.
}

void EfhEngine::generateSound3() {
	warning("STUB: generateSound3");
}

void EfhEngine::generateSound4(int arg0) {
	warning("STUB: generateSound4 %d", arg0);
}

void EfhEngine::generateSound5(int arg0) {
	warning("STUB: generateSound5 %d", arg0);
}

void EfhEngine::generateSound(int16 soundType) {
	switch (soundType) {
	case 5:
		generateSound3();
		break;
	case 9:
		generateSound1(20, 888, 3000);
		generateSound1(20, 888, 3000);
		break;
	case 10:
		generateSound5(1);
		break;
	case 13:
		generateSound2(256, 4096, 18);
		break;
	case 14:
		generateSound2(20, 400, 100);
		break;
	case 15:
		generateSound2(100, 888, 88);
		break;
	case 16:
		generateSound1(2000, 6096, 1500);
		break;
	case 17:
		generateSound4(1);
		break;
	default:
		// Not implemented because not used by the engine
		break;
	}
}

void EfhEngine::genericGenerateSound(int16 soundType, int16 repeatCount) {
	if (repeatCount <= 0)
		return;

	switch (soundType) {
	case 0:
	case 1:
	case 2:
		generateSound(5);
		break;
	case 3:
	case 4:
	case 6:
		generateSound(9);
		break;
	case 5:
	case 7:
		generateSound(13);
		break;
	case 8:
	case 9:
	case 10:
		generateSound(10);
		generateSound(9);
		break;
	case 14:
		generateSound(14);
		break;
	case 11:
	case 12:
	case 13:
		for (int counter = 0; counter < repeatCount; ++counter) {
			generateSound(17);
		}
		break;
	case 15:
		generateSound(16);
	default:
		break;
	}
}

} // End of namespace Efh

