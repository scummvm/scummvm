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

#include "qdengine/qd_precomp.h"
#include "qdengine/system/sound/snd_dispatcher.h"

namespace QDEngine {

sndDispatcher *sndDispatcher::_dispatcher_ptr;

sndDispatcher::sndDispatcher() : _is_enabled(true),
	_is_paused(false),
	_volume(255),
	_volume_dB(0),
	_frequency_coeff(1.0f) {
	if (!_dispatcher_ptr)
		_dispatcher_ptr = this;
}

sndDispatcher::~sndDispatcher() {
	if (_dispatcher_ptr == this)
		_dispatcher_ptr = NULL;
}

void sndDispatcher::set_volume(unsigned int vol) {
	_volume = vol & 0xFF;

	_volume_dB = convert_volume_to_dB(_volume);

	update_volume();
}

int sndDispatcher::convert_volume_to_dB(int vol) {
	if (vol > 255) vol = 255;
	if (vol < 0) vol = 0;

	if (vol != 255) {
		const int DB_MIN = -10000;
		const int DB_MAX = 0;
		const int DB_SIZE = DB_MAX - DB_MIN;

		return (DB_MIN + round(log10(9.0 * log(double(vol + 1)) / (log(2.0) * 8) + 1.0) * DB_SIZE));
	} else
		return 0;
}

} // namespace QDEngine
