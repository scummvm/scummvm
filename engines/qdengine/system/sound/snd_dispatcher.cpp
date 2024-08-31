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

#include "common/config-manager.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/xmath.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/sound/snd_dispatcher.h"
#include "qdengine/qdcore/util/plaympp_api.h"

namespace QDEngine {

sndDispatcher *sndDispatcher::_dispatcher_ptr;

static bool operator == (const sndSound &snd0, const sndSound &snd1) {
	return snd0.sound() == snd1.sound();
}

static bool operator == (const sndSound &snd, const sndHandle &h) {
	return snd.handle() == &h;
}

sndDispatcher::sndDispatcher() : _is_enabled(true),
		_is_paused(false),
		_volume(255),
		_volume_dB(0),
		_frequency_coeff(1.0f) {

	if (!_dispatcher_ptr)
		_dispatcher_ptr = this;
}

sndDispatcher::~sndDispatcher() {
	_sounds.clear();

	if (_dispatcher_ptr == this)
		_dispatcher_ptr = NULL;
}

void sndDispatcher::set_volume(uint32 vol) {
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

void sndDispatcher::quant() {
	for (auto it = _sounds.begin(); it != _sounds.end(); ) {
			if (it->is_stopped())
				it = _sounds.erase(it);
			else
				++it;
	}
}

bool sndDispatcher::play_sound(const sndSound *snd, bool loop, int vol) {
	if (is_enabled()) {
		_sounds.push_back(sndSound(*snd));
		sndSound &p = _sounds.back();

		if (loop)
			p.toggle_looping();

		int snd_volume = (vol == 255) ? volume_dB() : convert_volume_to_dB((volume() * vol) >> 8);

		p.create_sound_buffer();
		p.set_volume(snd_volume);
		p.change_frequency(frequency_coeff());

		if (!is_paused()) {
			if (!p.play()) return false;
		} else
			p.pause();
	}

	return true;
}

bool sndDispatcher::stop_sound(const sndSound *snd) {
	sound_list_t::iterator it = Common::find(_sounds.begin(), _sounds.end(), *snd);

	if (it != _sounds.end()) {
		it->stop();
		_sounds.erase(it);

		return true;
	}

	return false;
}

bool sndDispatcher::stop_sound(const sndHandle *handle) {
	sound_list_t::iterator it = Common::find(_sounds.begin(), _sounds.end(), *handle);

	if (it != _sounds.end()) {
		it->stop();
		_sounds.erase(it);

		return true;
	}

	return false;
}

sndSound::status_t sndDispatcher::sound_status(const sndHandle *handle) const {
	sound_list_t::const_iterator it = Common::find(_sounds.begin(), _sounds.end(), *handle);

	if (it != _sounds.end()) {
		if (is_paused())
			return sndSound::SOUND_PAUSED;

		return sndSound::SOUND_PLAYING;
	}

	return sndSound::SOUND_STOPPED;
}

sndSound::status_t sndDispatcher::sound_status(const sndSound *snd) const {
	sound_list_t::const_iterator it = Common::find(_sounds.begin(), _sounds.end(), *snd);

	if (it != _sounds.end())
		return it->status();

	return sndSound::SOUND_STOPPED;
}

bool sndDispatcher::update_volume() {
	for (sound_list_t::iterator it = _sounds.begin(); it != _sounds.end(); ++it)
		it->set_volume(volume_dB());

	return true;
}

bool sndDispatcher::update_frequency() {
	for (sound_list_t::iterator it = _sounds.begin(); it != _sounds.end(); ++it)
		it->change_frequency(frequency_coeff());

	return true;
}

void sndDispatcher::stop_sounds() {
	for (sound_list_t::iterator it = _sounds.begin(); it != _sounds.end(); ++it)
		it->stop();

	_sounds.clear();
}

bool sndDispatcher::set_sound_frequency(const sndHandle *snd, float coeff) {
	sound_list_t::iterator it = Common::find(_sounds.begin(), _sounds.end(), *snd);

	if (it != _sounds.end()) {
		it->change_frequency(frequency_coeff() * coeff);
		return true;
	}

	return false;
}

void sndDispatcher::pause_sounds() {
	for (sound_list_t::iterator it = _sounds.begin(); it != _sounds.end(); ++it)
		it->pause();
}

void sndDispatcher::resume_sounds() {
	for (sound_list_t::iterator it = _sounds.begin(); it != _sounds.end(); ++it) {
		if (it->is_paused())
			it->resume();
	}
}

void sndDispatcher::syncSoundSettings() {
	set_volume(ConfMan.getInt("sound_volume"));

	if (ConfMan.getBool("enable_sound"))
		enable();
	else
		disable();
}

} // namespace QDEngine
