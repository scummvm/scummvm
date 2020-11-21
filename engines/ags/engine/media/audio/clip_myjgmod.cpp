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

#include "media/audio/audiodefines.h"

#ifdef JGMOD_MOD_PLAYER

#include "media/audio/clip_myjgmod.h"
#include "media/audio/audiointernaldefs.h"

int MYMOD::poll() {
	if (done)
		return done;

	if (is_mod_playing() == 0)
		done = 1;

	return done;
}

void MYMOD::set_volume(int newvol) {
	vol = newvol;
	if (!done)
		set_mod_volume(newvol);
}

void MYMOD::destroy() {
	stop_mod();
	destroy_mod(tune);
	tune = NULL;
}

void MYMOD::seek(int patnum) {
	if (is_mod_playing() != 0)
		goto_mod_track(patnum);
}

int MYMOD::get_pos() {
	if (!is_mod_playing())
		return -1;
	return mi.trk;
}

int MYMOD::get_pos_ms() {
	return 0;                   // we don't know ms offset
}

int MYMOD::get_length_ms() {
	// we don't know ms
	return 0;
}

int MYMOD::get_voice() {
	// MOD uses so many different voices it's not practical to keep track
	return -1;
}

int MYMOD::get_sound_type() {
	return MUS_MOD;
}

int MYMOD::play() {
	play_mod(tune, repeat);

	return 1;
}

MYMOD::MYMOD() : SOUNDCLIP() {
}

#endif // #ifdef JGMOD_MOD_PLAYER
