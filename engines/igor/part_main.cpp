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
#include "igor/igor.h"

namespace Igor {

void IgorEngine::PART_MAIN() {
	while (!g_engine->shouldQuit() && !_eventQuitGame) {
		switch (_currentPart) {

        case 50:
        case 51:
        case 52:
            PART_05();
            break;
		case 61:
			PART_05(); // SpringRock
			break;

		case 850: // Intro cutscene
			PART_85();
			break;
		case 900: // Logo slideshow
		case 901:
		case 902:
		case 903:
		case 904:
			PART_90();
			break;

		default:
			error("Unknown or unhandled room part %d", _currentPart);
			break;
		}
	}
}

} // End of namespace Igor
