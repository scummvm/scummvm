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
#include "qdengine/system/app_core.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/sound/ds_snd_dispatcher.h"
#include "qdengine/qdcore/util/plaympp_api.h"

namespace QDEngine {

static bool operator == (const dsSound &snd0, const sndSound &snd1) {
	return snd0.sound() == snd1.sound();
}

static bool operator == (const dsSound &snd, const sndHandle &h) {
	return snd.handle() == &h;
}

ds_sndDispatcher::ds_sndDispatcher() : _sound_device(NULL) {
	warning("STUB: ds_sndDispatcher::ds_sndDispatcher()");
#if 0
	HRESULT res = DirectSoundCreate(NULL, &_sound_device, NULL);
	if (FAILED(res) || _sound_device == NULL)
		return;

	HWND hWnd = static_cast<HWND>(appGetHandle());
	grDispatcher *gp = grDispatcher::instance();

	if (gp && gp->is_in_fullscreen_mode()) {
		res = _sound_device->SetCooperativeLevel(hWnd, DSSCL_EXCLUSIVE);
		if (FAILED(res))
			_sound_device->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
	} else {
		_sound_device->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
	}
#endif
}

ds_sndDispatcher::~ds_sndDispatcher() {
	_sounds.clear();

	warning("STUB: ds_sndDispatcher::~ds_sndDispatcher()");
#if 0
	if (_sound_device)
		_sound_device-> Release();
#endif
}

void ds_sndDispatcher::quant() {
	_sounds.remove_if(std::mem_fn(&dsSound::is_stopped));
}

bool ds_sndDispatcher::play_sound(const sndSound *snd, bool loop, float start_position, int vol) {
	if (!_sound_device) return false;

	if (is_enabled()) {
		_sounds.push_back(dsSound(*snd, _sound_device));
		dsSound &p = _sounds.back();

		if (loop)
			p.toggle_looping();

		int snd_volume = (vol == 255) ? volume_dB() : convert_volume_to_dB((volume() * vol) >> 8);

		p.create_sound_buffer();
		p.set_volume(snd_volume);
		p.change_frequency(frequency_coeff());
		p.set_position(start_position);

		if (!is_paused()) {
			if (!p.play()) return false;
		} else
			p.pause();
	}

	return true;
}

bool ds_sndDispatcher::stop_sound(const sndSound *snd) {
	if (!_sound_device) return false;

	sound_list_t::iterator it = std::find(_sounds.begin(), _sounds.end(), *snd);

	if (it != _sounds.end()) {
		it->stop();
		_sounds.erase(it);

		return true;
	}

	return false;
}

bool ds_sndDispatcher::stop_sound(const sndHandle *handle) {
	if (!_sound_device) return false;

	sound_list_t::iterator it = std::find(_sounds.begin(), _sounds.end(), *handle);

	if (it != _sounds.end()) {
		it->stop();
		_sounds.erase(it);

		return true;
	}

	return false;
}

sndSound::status_t ds_sndDispatcher::sound_status(const sndHandle *handle) const {
	sound_list_t::const_iterator it = std::find(_sounds.begin(), _sounds.end(), *handle);

	if (it != _sounds.end()) {
		if (is_paused())
			return sndSound::SOUND_PAUSED;

		return sndSound::SOUND_PLAYING;
	}

	return sndSound::SOUND_STOPPED;
}

sndSound::status_t ds_sndDispatcher::sound_status(const sndSound *snd) const {
	sound_list_t::const_iterator it = std::find(_sounds.begin(), _sounds.end(), *snd);

	if (it != _sounds.end())
		return it->status();

	return sndSound::SOUND_STOPPED;
}

bool ds_sndDispatcher::update_volume() {
	for (sound_list_t::iterator it = _sounds.begin(); it != _sounds.end(); ++it)
		it->set_volume(volume_dB());

	return true;
}

bool ds_sndDispatcher::update_frequency() {
	for (sound_list_t::iterator it = _sounds.begin(); it != _sounds.end(); ++it)
		it->change_frequency(frequency_coeff());

	return true;
}

void ds_sndDispatcher::stop_sounds() {
	for (sound_list_t::iterator it = _sounds.begin(); it != _sounds.end(); ++it)
		it->stop();

	_sounds.clear();
}

bool ds_sndDispatcher::set_sound_frequency(const sndHandle *snd, float coeff) {
	sound_list_t::iterator it = std::find(_sounds.begin(), _sounds.end(), *snd);

	if (it != _sounds.end()) {
		it->change_frequency(frequency_coeff() * coeff);
		return true;
	}

	return false;
}

float ds_sndDispatcher::sound_position(const sndHandle *snd) const {
	sound_list_t::const_iterator it = std::find(_sounds.begin(), _sounds.end(), *snd);

	if (it != _sounds.end())
		return it->position();

	return 0.0f;
}

void ds_sndDispatcher::pause_sounds() {
	for (sound_list_t::iterator it = _sounds.begin(); it != _sounds.end(); ++it)
		it->pause();
}

void ds_sndDispatcher::resume_sounds() {
	for (sound_list_t::iterator it = _sounds.begin(); it != _sounds.end(); ++it) {
		if (it->is_paused())
			it->resume();
	}
}
} // namespace QDEngine
