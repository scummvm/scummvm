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

#include "ags/shared/util/wgt2_allg.h"
#include "ags/engine/media/audio/clip_my_midi.h"
#include "ags/ags.h"
#include "ags/music.h"

namespace AGS3 {

MYMIDI::MYMIDI(Common::SeekableReadStream *data, bool repeat) :
	_state(SoundClipInitial), _data(data), lengthInSeconds(0) {
	_mixer = ::AGS::g_vm->_mixer;
	_repeat = repeat;
}

MYMIDI::~MYMIDI() {
	::AGS::g_music->stop();
	delete _data;
	_data = nullptr;
}

void MYMIDI::poll() {
	bool playing = is_playing();
	if (playing)
		_state = SoundClipPlaying;
	else if (_state == SoundClipPlaying)
		_state = SoundClipStopped;
}

void MYMIDI::seek(int pos) {
	warning("TODO: MYMIDI::seek");
}

int MYMIDI::get_pos() {
	// We don't know ms with midi
	return 0;
}

int MYMIDI::get_pos_ms() {
	// We don't know ms with midi
	return 0;
}

int MYMIDI::get_length_ms() {
	warning("TODO: MYMIDI::get_length_ms");
	return lengthInSeconds * 1000;
}

void MYMIDI::pause() {
	::AGS::g_music->pause();
	_state = SoundClipPaused;
}

void MYMIDI::resume() {
	if (_state != SoundClipPaused)
		return;

	::AGS::g_music->resume();
	_state = SoundClipPlaying;
}

int MYMIDI::play() {
	::AGS::g_music->playMusic(_data, _repeat);
	_state =  SoundClipPlaying;
	return 1;
}

int MYMIDI::play_from(int position) {
	// TODO: Implement playing from arbitrary positions
	if (position == 0) {
		play();
		return 1;
	} else {
		return 0;
	}
}

bool MYMIDI::is_playing() {
	return ::AGS::g_music->isPlaying();
}

bool MYMIDI::is_paused() {
	return false;
}

void MYMIDI::set_panning(int newPanning) {
	// No implementation for MIDI
}

void MYMIDI::set_speed(int new_speed) {
	warning("TODO: MYMIDI::set_speed");
	_speed = new_speed;
}

void MYMIDI::adjust_volume() {
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, _vol255);
}

} // namespace AGS3
