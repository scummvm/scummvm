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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/riddle/rooms/section4/room495.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room495::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room495::init() {
	updateFlags1();
	updateFlags2();
	interface_hide();
	player_set_commands_allowed(false);
	digi_preload("950_s46");

	_G(player).disable_hyperwalk = true;
	_G(player).walker_in_this_scene = false;
	_val1 = 255;

	series_show("495 peiping", 0xf00);
	series_show("495 new york", 0xf00);

	if (_G(flags)[V056])
		series_show("495 danzig", 0xf00);
	if (_G(flags)[V110]) {
		series_show("495 sikkim", 0xf00);
		series_show("495 lima", 0xf00);
	}

	if (player_been_here(501))
		series_show("495 easter island", 0xf00);
	digi_play("950_s46", 3);

	switch (_G(game).previous_room) {
	case 201:
		_G(flags)[V092] = 1;

		switch (_G(flags)[V129]) {
		case 1:
			play("495 peiping to ny", "495pg03s", 93, 3);
			break;
		case 2:
			play("495 peiping to danzig", "495pg02s", 38, 7);
			break;
		case 3:
			play("495 peiping to sikkim", "495pg01s", 17, 14);
			break;
		case 4:
			play("495 peiping to lima", "495pg04s", 11, 3);
			break;
		default:
			break;
		}
		break;

	case 301:
		switch (_G(flags)[V129]) {
		case 0:
			play("495 ny to peiping", "495ny03s", 108, 3);
			break;
		case 2:
			play("495 ny to danzig", "495ny02s", 47, 4);
			break;
		case 3:
			play("495 ny to sikkim", "495ny04s", 96, 3);
			break;
		case 4:
			play("495 ny to lima", "495ny01s", 28, 5);
			break;
		default:
			break;
		}
		break;

	case 401:
		switch (_G(flags)[V129]) {
		case 0:
			play("495 danzig to peiping", "495dg04s", 51, 4);
			break;
		case 1:
			play("495 danzig to ny", "495dg02s", 43, 4);
			break;
		case 3:
			play("495 danzig to sikkim", "495dg03s", 38, 4);
			break;
		case 4:
			play("495 danzig to lima", "495dg01s", 65, 4);
			break;
		default:
			break;
		}
		break;

	case 501:
		switch (_G(flags)[V129]) {
		case 0:
			play("495 lima to peiping", "495lm05s", 115, 3);
			break;
		case 1:
			play("495 lima to ny", "495lm02s", 18, 10);
			break;
		case 2:
			play("495 lima to danzig", "495m03s", 70, 4);
			break;
		case 3:
			play("495 lima to sikkim", "495lm04s", 100, 3);
			break;
		case 5:
			play("495 lima to easter isle", "495lm01s", 16, 12);
			break;
		default:
			break;
		}
		break;

	case 603:
	case 615:
		if (_G(flags)[V129] == 4)
			play("495 easter isle to lima", "495eis", 16, 12);
		break;

	case 701:
		switch (_G(flags)[V129]) {
		case 0:
			play("495 sikkim to peiping", "495sm02s", 17, 10);
			break;
		case 1:
			play("495 sikkim to ny", "495sm04s", 93, 3);
			break;
		case 2:
			play("495 sikkim to danzig", "495sm03s", 38, 5);
			break;
		case 4:
			play("495 sikkim to lima", "495sm01s", 100, 3);
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}

void Room495::daemon() {
	switch (_G(kernel).trigger) {
	case 500:
		kernel_timing_trigger(1, 502);
		disable_player_commands_and_fade_init(-1);

		series_play(_seriesName1, 0, 16, -1, 5, 0, 100, 0, 0,
			_lastFrame + 1, -1);
		series_play(_seriesName2, 0xf00, 16, -1, 5, 0, 100, 0, 0,
			_lastFrame + 1, -1);
		break;

	case 501:
		interface_show();

		switch (_G(flags)[V129]) {
		case 0:
			_G(flags)[V092] = 0;
			_G(game).setRoom(201);
			break;
		case 1:
			_G(game).setRoom(301);
			break;
		case 2:
			_G(flags)[V092] = 0;
			_G(game).setRoom(401);
			break;
		case 3:
			_G(flags)[V092] = 0;
			_G(game).setRoom(701);
			break;
		case 4:
			_G(flags)[V092] = 0;
			_G(game).setRoom(501);
			break;
		case 5:
			_G(flags)[V092] = 0;
			_G(game).setRoom(603);
			break;
		case 6:
			_G(flags)[V092] = 0;
			_G(game).setRoom(504);
			break;
		default:
			break;
		}
		break;

	case 502:
		_val1 -= 5;
		digi_change_panning(3, _val1);
		kernel_timing_trigger(1, 502);

		if (_val1 <= 0) {
			digi_stop(3);
			kernel_timing_trigger(1, 501);
		}
		break;

	default:
		break;
	}
}

void Room495::pre_parser() {
	_G(player).resetWalk();
}

void Room495::updateFlags1() {
	int numPlaced = getNumKeyItemsPlaced();

	if (numPlaced < 3)
		_G(flags)[V088] = 1;
	else if (numPlaced < 6)
		_G(flags)[V088] = 3;
	else if (numPlaced < 9)
		_G(flags)[V088] = 4;
	else
		_G(flags)[V088] = 5;
}

void Room495::updateFlags2() {
	if (_G(flags)[V045]) {
		_G(flags)[V045] = 0;
		if (++_G(flags)[V090] > 3)
			_G(flags)[V090] = 3;

		switch (_G(flags)[V090]) {
		case 1:
			_G(flags)[V367] = 1;
			break;
		case 2:
			_G(flags)[V368] = 1;
			break;
		case 3:
			_G(flags)[V369] = 1;
			break;
		default:
			break;
		}
	}
}

void Room495::play(const char *seriesName1, const char *seriesName2,
		int lastFrame, int frameRate) {
	_lastFrame = lastFrame;
	_seriesName1 = seriesName1;
	_seriesName2 = seriesName2;

	series_play(_seriesName1, 0, 0, 500, frameRate, 0, 100, 0, 0, 0, lastFrame);
	series_play(_seriesName2, 0xf00, 0, -1, frameRate, 0, 100, 0, 0, 0, lastFrame);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
