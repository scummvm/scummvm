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

#ifndef AGS_ENGINE_MEDIA_AUDIO_CLIP_MYDUMBMOD_H
#define AGS_ENGINE_MEDIA_AUDIO_CLIP_MYDUMBMOD_H

#include "ags/lib/audio/aldumb.h"
#include "ags/engine/media/audio/soundclip.h"

namespace AGS3 {

#define VOLUME_TO_DUMB_VOL(vol) ((float)vol) / 256.0

void al_duh_set_loop(AL_DUH_PLAYER *dp, int loop);

// MOD/XM (DUMB)
struct MYMOD : public SOUNDCLIP {
	DUH *tune;
	AL_DUH_PLAYER *duhPlayer;

	void poll() override;

	void set_volume(int newvol) override;

	void destroy() override;

	void seek(int patnum) override;

	// NOTE: this implementation of the virtual function returns a MOD/XM
	// "order" index, not actual playing position;
	// this does not make much sense in the context of the interface itself,
	// and, as it seems, was implemented so solely for the purpose of emulating
	// deprecated "GetMODPattern" script function.
	// (see Game_GetMODPattern(), and documentation for AudioChannel.Position property)
	// TODO: find a way to redesign this behavior
	int get_pos() override;

	int get_pos_ms() override;

	int get_length_ms() override;

	void pause() override;

	void resume() override;

	int get_sound_type() override;

	int play() override;

	MYMOD();

protected:
	int get_voice() override;
	void adjust_volume() override;
	// Returns real MOD/XM playing position
	int get_real_mod_pos();
};

} // namespace AGS3

#endif
