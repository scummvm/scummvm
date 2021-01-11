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

void MYMIDI::poll() {
	if (state_ != SoundClipPlaying) {
		return;
	}

	if (midi_pos < 0)
		state_ = SoundClipStopped;
}

void MYMIDI::adjust_volume() {
	if (!is_playing()) {
		return;
	}
	AGS3::set_volume(-1, get_final_volume());
}

void MYMIDI::set_volume(int newvol) {
	vol = newvol;
	adjust_volume();
}

void MYMIDI::destroy() {
	stop_midi();

	if (tune) {
		destroy_midi(tune);
	}
	tune = nullptr;

	state_ = SoundClipStopped;
}

void MYMIDI::seek(int pos) {
	if (!is_playing()) {
		return;
	}
	midi_seek(pos);
}

int MYMIDI::get_pos() {
	if (!is_playing()) {
		return -1;
	}
	return midi_pos;
}

int MYMIDI::get_pos_ms() {
	return 0;                   // we don't know ms with midi
}

int MYMIDI::get_length_ms() {
	return lengthInSeconds * 1000;
}

int MYMIDI::get_voice() {
	// voice is N/A for midi
	return -1;
}

void MYMIDI::pause() {
	if (state_ != SoundClipPlaying) {
		return;
	}
	midi_pause();
	state_ = SoundClipPaused;
}

void MYMIDI::resume() {
	if (state_ != SoundClipPaused) {
		return;
	}
	midi_resume();
	state_ = SoundClipPlaying;
}

int MYMIDI::get_sound_type() {
	return MUS_MIDI;
}

int MYMIDI::play() {
	if (tune == nullptr) {
		return 0;
	}

	lengthInSeconds = get_midi_length(tune);
	if (AGS3::play_midi(tune, repeat)) {
		lengthInSeconds = 0;
		return 0;
	}

	state_ = SoundClipPlaying;
	return 1;
}

MYMIDI::MYMIDI() : SOUNDCLIP() {
	tune = nullptr;
	lengthInSeconds = 0;
}

} // namespace AGS3
