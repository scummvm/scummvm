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

#ifndef AGS_ENGINE_MEDIA_AUDIO_CLIP_MYMIDI_H
#define AGS_ENGINE_MEDIA_AUDIO_CLIP_MYMIDI_H

#include "ags/engine/media/audio/sound_clip.h"
#include "ags/engine/media/audio/audio_defines.h"

namespace AGS3 {

// MIDI
struct MYMIDI : public SOUNDCLIP {
	Audio::Mixer *_mixer;
	Common::SeekableReadStream *_data;
	int lengthInSeconds;
	SoundClipState _state;

	MYMIDI(Common::SeekableReadStream *data, bool repeat);
	~MYMIDI() override;

	void poll() override;

	void seek(int pos) override;

	int get_pos() override;

	int get_pos_ms() override;

	int get_length_ms() override;

	void pause() override;

	void resume() override;

	int get_sound_type() const override {
		return MUS_MIDI;
	}

	int play() override;
	int play_from(int position) override;
	bool is_playing() override;
	bool is_paused() override;
	void set_panning(int newPanning) override;
	void set_speed(int new_speed) override;
	void adjust_volume() override;
};

} // namespace AGS3

#endif
