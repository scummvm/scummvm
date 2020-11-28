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

#include "ags/shared/util/wgt2allg.h"
#include "ags/engine/media/audio/audio.h"
#include "ags/engine/media/audio/audiodefines.h"
#include "ags/engine/media/audio/soundclip.h"
#include "ags/engine/media/audio/audiointernaldefs.h"

namespace AGS3 {

int SOUNDCLIP::play_from(int position) {
	int retVal = play();
	if ((retVal != 0) && (position > 0)) {
		seek(position);
	}
	return retVal;
}

void SOUNDCLIP::set_panning(int newPanning) {
	if (!is_playing()) {
		return;
	}

	int voice = get_voice();
	if (voice >= 0) {
		voice_set_pan(voice, newPanning);
		panning = newPanning;
	}
}

void SOUNDCLIP::pause() {
	if (state_ != SoundClipPlaying) {
		return;
	}

	int voice = get_voice();
	if (voice >= 0) {
		voice_stop(voice);
		state_ = SoundClipPaused;
	}
}

void SOUNDCLIP::resume() {
	if (state_ != SoundClipPaused) {
		return;
	}

	int voice = get_voice();
	if (voice >= 0) {
		voice_start(voice);
		state_ = SoundClipPlaying;
	}
}

SOUNDCLIP::SOUNDCLIP() {
	state_ = SoundClipInitial;
	priority = 50;
	panning = 128;
	panningAsPercentage = 0;
	speed = 1000;
	sourceClipType = 0;
	sourceClip = nullptr;
	vol = 0;
	volAsPercentage = 0;
	volModifier = 0;
	muted = false;
	repeat = false;
	xSource = -1;
	ySource = -1;
	maximumPossibleDistanceAway = 0;
	directionalVolModifier = 0;
}

SOUNDCLIP::~SOUNDCLIP() = default;

} // namespace AGS3
