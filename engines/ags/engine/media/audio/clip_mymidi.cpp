/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/engine/media/audio/audiodefines.h"
#include "ags/shared/util/wgt2allg.h"
#include "ags/engine/media/audio/clip_mymidi.h"
#include "ags/engine/media/audio/audiointernaldefs.h"

namespace AGS3 {

MYMIDI::MYMIDI(Common::SeekableReadStream *data, bool repeat) :
		_data(data), _repeat(repeat), lengthInSeconds(0) {
}

void MYMIDI::destroy() {
	stop();
	delete _data;
	_data = nullptr;
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

void MYMIDI::stop() {
	::AGS::g_music->stop();
}

bool MYMIDI::is_playing() const {
	return ::AGS::g_music->isPlaying();
}

int MYMIDI::get_volume() const {
	return _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
}

void MYMIDI::set_volume(int volume) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
}

void MYMIDI::set_panning(int newPanning) {
	// No implementation for MIDI
}

/*
int MYMIDI::get_voice() {
	// voice is N/A for midi
	return -1;
}
*/

} // namespace AGS3
