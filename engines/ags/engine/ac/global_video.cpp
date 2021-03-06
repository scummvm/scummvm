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

#include "ags/lib/allegro.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_video.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/media/video/video.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/shared/util/string_compat.h"
#include "ags/globals.h"

namespace AGS3 {

void scrPlayVideo(const char *name, int skip, int flags) {
	EndSkippingUntilCharStops();

	if (_GP(play).fast_forward)
		return;
	if (_G(debug_flags) & DBG_NOVIDEO)
		return;

	if ((flags < 10) && (_GP(usetup).audio_backend == 0)) {
		// if game audio is disabled in Setup, then don't
		// play any sound on the video either
		flags += 10;
	}

	pause_sound_if_necessary_and_play_video(name, skip, flags);
}

void pause_sound_if_necessary_and_play_video(const char *name, int skip, int flags) {
	int musplaying = _GP(play).cur_music_number, i;
	int ambientWas[MAX_SOUND_CHANNELS];
	for (i = 1; i < MAX_SOUND_CHANNELS; i++)
		ambientWas[i] = _GP(ambient)[i].channel;

	if ((strlen(name) > 3) && (ags_stricmp(&name[strlen(name) - 3], "ogv") == 0)) {
		play_theora_video(name, skip, flags, true);
	} else if ((strlen(name) > 3) && (ags_stricmp(&name[strlen(name) - 3], "mpg") == 0)) {
		play_mpeg_video(name, skip, flags, true);
	} else if ((strlen(name) > 3) && (ags_stricmp(&name[strlen(name) - 3], "avi") == 0)) {
		play_avi_video(name, skip, flags, true);
	} else {
		// Unsure what the video type is, so try each in turn
		if (!play_avi_video(name, skip, flags, false) &&
				!play_mpeg_video(name, skip, flags, false) &&
				!play_theora_video(name, skip, flags, false))
			Display("Unsupported video '%s'", name);
	}

	if (flags < 10) {
		update_music_volume();
		// restart the music
		if (musplaying >= 0)
			newmusic(musplaying);
		for (i = 1; i < MAX_SOUND_CHANNELS; i++) {
			if (ambientWas[i] > 0)
				PlayAmbientSound(ambientWas[i], _GP(ambient)[i].num, _GP(ambient)[i].vol, _GP(ambient)[i].x, _GP(ambient)[i].y);
		}
	}
}

} // namespace AGS3
