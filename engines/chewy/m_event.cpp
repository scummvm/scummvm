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
#include "chewy/global.h"

namespace Chewy {

#define NORMAL_PLAY 0
#define SEQUENCE_PLAY 1
#define PATTERN_PLAY 2

void load_room_music(int16 room_nr) {
	const int16 seq_start = 0;
	const int16 seq_end = 0;
	const int16 pattern = 0;
	int16 ttp_index = -1;
	int16 volume = _G(spieler).MusicVol;
	const int16 lp_mode = 1;
	const int16 play_mode = NORMAL_PLAY;
	if (_G(spieler).MusicSwitch && (_G(music_handle))) {
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
			_G(sndPlayer)->stopMod();
			break;
		}
		if (ttp_index != -1) {
			if (volume < 0)
				volume = 0;
			else if (volume > _G(spieler).MusicVol)
				volume = _G(spieler).MusicVol;
			_G(sndPlayer)->setMusicMasterVol(volume);
			_G(sndPlayer)->setLoopMode(lp_mode);
			if (ttp_index != _G(currentSong)) {
				_G(sndPlayer)->stopMod();
				while (_G(sndPlayer)->musicPlaying());
				memset(_G(Ci).MusicSlot, 0, MUSIC_SLOT_SIZE);
				_G(mem)->file->select_pool_item(_G(music_handle), _G(EndOfPool) - ttp_index);
				_G(mem)->file->load_tmf(_G(music_handle), (tmf_header *)_G(Ci).MusicSlot);
				_G(currentSong) = ttp_index;
				if (!_G(modul)) {
					if (play_mode == NORMAL_PLAY)
						_G(sndPlayer)->playMod((tmf_header *)_G(Ci).MusicSlot);
					else {
						_G(sndPlayer)->playMod((tmf_header *)_G(Ci).MusicSlot);
						_G(sndPlayer)->stopMod();
						if (play_mode == SEQUENCE_PLAY)
							_G(sndPlayer)->playSequence(seq_start, seq_end);
						else if (play_mode == PATTERN_PLAY)
							_G(sndPlayer)->playPattern(pattern);
					}
				}
			}
		}
	}
}

} // namespace Chewy
