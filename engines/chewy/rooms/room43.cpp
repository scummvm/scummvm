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
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room43.h"

namespace Chewy {
namespace Rooms {

void Room43::night_small() {
	int16 i;
	hide_person();
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	switch_room(43);
	ailsnd->stop_mod();
	hide_cur();
	flags.NoScroll = true;
	_G(spieler).ScrollxStep = 1;
	start_aad_wait(188, -1);
	auto_scroll(320, 0);
	start_aad_wait(189, -1);
	auto_scroll(0, 0);
	start_aad_wait(190, -1);
	_G(spieler).ScrollxStep = 16;
	auto_scroll(192, 0);
	for (i = 0; i < 3; i++) {
		if (i)
			_G(spieler).ScrollxStep = 16 / i;
		auto_scroll(128 + i * 16, 0);
		auto_scroll(192, 0);
	}
	_G(spieler).scrollx = 194;
	start_aad_wait(191, -1);
	load_room_music(255);
	flic_cut(FCUT_058, FLC_MODE);
	flags.NoScroll = false;
	_G(spieler).ScrollxStep = 1;
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	show_cur();
	show_person();
}

void Room43::catch_pg() {
	hide_person();
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	switch_room(43);
	ailsnd->stop_mod();
	hide_cur();
	flags.NoScroll = true;
	_G(spieler).ScrollxStep = 1;
	start_aad_wait(234, -1);
	auto_scroll(194, 0);
	start_aad_wait(235, -1);
	load_room_music(255);
	_G(spieler).R43GetPgLady = true;
	flic_cut(FCUT_058, FLC_MODE);
	flags.NoScroll = false;
	_G(spieler).ScrollxStep = 1;
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	show_cur();
	show_person();
}

} // namespace Rooms
} // namespace Chewy
