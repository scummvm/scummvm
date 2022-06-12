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

#include "chewy/defines.h"
#include "chewy/globals.h"
#include "chewy/sound.h"

namespace Chewy {

#define NORMAL_PLAY 0
#define SEQUENCE_PLAY 1
#define PATTERN_PLAY 2

void load_room_music(int16 room_nr) {
	int16 musicIndex = -1;
	if (!g_engine->_sound->musicEnabled())
		return;

	switch (room_nr) {
	case 0:
		musicIndex = 13;
		break;
	case 1:
	case 18:
	case 90:
		musicIndex = 17;
		break;
	case 2:
	case 88:
		musicIndex = 43;
		break;
	case 3:
	case 4:
		musicIndex = 0;
		break;
	case 5:
	case 8:
	case 12:
	case 86:
		musicIndex = 14;
		break;
	case 6:
		musicIndex = 1;
		break;
	case 7:
	case 97:
		musicIndex = 18;
		break;
	case 9:
	case 10:
	case 47:
	case 87:
		musicIndex = 20;
		break;
	case 11:
		musicIndex = 19;
		break;
	case 13:
		musicIndex = -1;
		break;
	case 14:
		musicIndex = 19;
		break;
	case 15:
	case 16:
	case 19:
	case 96:
		musicIndex = 16;
		break;
	case 21:
		musicIndex = 2;
		break;
	case 22:
		musicIndex = 48;
		break;
	case 25:
	case 26:
		musicIndex = 11;
		break;
	case 27:
	case 30:
	case 54:
	case 63:
		musicIndex = 33;
		break;
	case 28:
	case 29:
		musicIndex = 47;
		break;
	case 31:
	case 35:
		musicIndex = 9;
		break;
	case 32:
	case 40:
	case 71:
	case 89:
	case 92:
		musicIndex = 38;
		break;
	case 33:
		musicIndex = 35;
		break;
	case 37:
		musicIndex = 8;
		break;
	case 39:
		musicIndex = 9;
		break;
	case 41:
	case 77:
	case 78:
	case 83:
	case 93:
		musicIndex = -1;
		break;
	case 42:
		musicIndex = 41;
		break;
	case 45:
		musicIndex = 44;
		break;
	case 46:
	case 50:
	case 73:
	case 74:
		musicIndex = 21;
		break;
	case 48:
		musicIndex = 22;
		break;
	case 49:
		musicIndex = 3;
		break;
	case 51:
	case 52:
		musicIndex = 27;
		break;
	case 53:
		musicIndex = 26;
		break;
	case 55:
	case 57:
		musicIndex = 23;
		break;
	case 56:
		// TODO: Extra checks for two flags
		//if ( (spieler.flags32 & SpielerFlags32_10) != 0 && spieler.flags33 >= 0 ) {
		musicIndex = 52;
		//} else {
		//  musicIndex = 7;
		//}
		break;
	case 62:
		musicIndex = 25;
		break;
	case 64:
		musicIndex = 51;
		break;
	case 66:
	case 68:
		musicIndex = 34;
		break;
	case 67:
	case 69:
	case 70:
	case 75:
		musicIndex = 28;
		break;
	case 72:
		musicIndex = 31;
		break;
	case 76:
		musicIndex = 46;
		break;
	case 79:
		musicIndex = 6;
		break;
	case 80:
		musicIndex = 29;
		break;
	case 81:
		musicIndex = 45;
		break;
	case 82:
		musicIndex = 50;
		break;
	case 84:
		musicIndex = 24;
		break;
	case 85:
		musicIndex = 32;
		break;
	case 91:
		musicIndex = 36;
		break;
	case 94:
	case 95:
		musicIndex = 40;
		break;
	case 98:
		musicIndex = 4;
		break;
	case 255:
		musicIndex = 5;
		break;
	case 256:
		musicIndex = 10;
		break;
	case 257:
		musicIndex = 52;
		break;
	case 258:
		musicIndex = 53;
		break;
	case 259:
		musicIndex = 54;
		break;
	case 260:
		musicIndex = 24;
		break;
	default:
		musicIndex = -1;
		_G(currentSong) = -1;
		g_engine->_sound->stopMusic();
		break;
	}

	// The original changed the volume here,
	// but we try to maintain a stable volume
	if (musicIndex != _G(currentSong)) {
		g_engine->_sound->stopMusic();
		_G(currentSong) = musicIndex;
		g_engine->_sound->playMusic(musicIndex, true);
	}
}

} // namespace Chewy
