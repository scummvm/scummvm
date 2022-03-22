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
	int16 ttp_index = -1;
	int16 volume = _G(gameState).MusicVol;
	if (g_engine->_sound->musicEnabled()) {
		switch (room_nr) {
		case 0:
			ttp_index = 0;
			break;

		case 1:
			ttp_index = 0;
			volume -= 4;
			break;

		case 2:
			ttp_index = 0;
			volume -= 3;
			break;

		case 3:
			ttp_index = 0;
			volume -= 6;
			break;

		case 4:
			ttp_index = 0;
			volume -= 7;
			break;

		case 5:
			ttp_index = 0;
			volume -= 2;
			break;

		case 6:
			ttp_index = 1;
			volume -= 5;
			break;

		case 7:
			ttp_index = 0;
			volume -= 1;
			break;

		case 8:
			ttp_index = 0;
			break;

		case 9:
			ttp_index = 0;
			break;

		case 10:
			ttp_index = 0;
			break;

		case 11:
			ttp_index = 1;
			break;

		case 12:
			ttp_index = 0;
			break;

		case 13:
			ttp_index = 0;
			break;

		case 14:
			ttp_index = 0;
			break;

		case 15:
			ttp_index = 0;
			break;

		case 16:
			ttp_index = 1;
			break;
		case 18:
			ttp_index = 3;
			break;

		case 21:
			ttp_index = 2;
			break;

		case 22:
			ttp_index = 0;
			break;

		case 25:
		case 26:
			ttp_index = 13;
			break;

		case 32:
			ttp_index = 8;
			volume -= 20;
			break;

		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 33:
		case 35:
		case 39:
			ttp_index = 11;
			volume -= 10;
			break;

		case 37:
			ttp_index = 10;
			volume -= 10;
			break;

		case 40:
			ttp_index = 14;
			volume -= 10;
			break;

		case 41:
			ttp_index = 9;
			volume -= 10;
			break;

		case 42:
			ttp_index = 8;
			volume -= 10;
			break;

		case 255:
			ttp_index = 7;
			break;

		case 256:
			ttp_index = 12;
			break;

		default:
			ttp_index = -1;
			_G(currentSong) = -1;
			g_engine->_sound->stopMusic();
			break;
		}
		if (ttp_index != -1) {
			if (volume < 0)
				volume = 0;
			else if (volume > _G(gameState).MusicVol)
				volume = _G(gameState).MusicVol;
			g_engine->_sound->setMusicVolume(volume * Audio::Mixer::kMaxChannelVolume / 120);
			if (ttp_index != _G(currentSong)) {
				g_engine->_sound->stopMusic();
				_G(currentSong) = ttp_index;
				g_engine->_sound->playMusic(ttp_index, true);
			}
		}
	}
}

} // namespace Chewy
