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
#include "ags/engine/ac/game_setup.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_video.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/media/video/video.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/shared/util/string_compat.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

void pause_sound_if_necessary_and_play_video(const char *name, int flags, VideoSkipType skip);

void PlayFlic(int numb, int scr_flags) {
	EndSkippingUntilCharStops();
	if (_GP(play).fast_forward)
		return;
	if (_G(debug_flags) & DBG_NOVIDEO)
		return;

	// AGS 2.x: If the screen is faded out, fade in again when playing a movie.
	if (_G(loaded_game_file_version) <= kGameVersion_272)
		_GP(play).screen_is_faded_out = 0;

	// Convert PlayFlic flags to common video flags
	/* NOTE: historically using decimal "flags"
	default (0): the video will be played stretched to screen;
		player cannot skip animation; screen will be cleared
	1: player can press ESC to skip animation
	2: player can press any key or click mouse to skip animation
	10: play the video at original size
	100: do not clear the screen before starting playback
	*/
	int flags = kVideo_EnableVideo;
	VideoSkipType skip = VideoSkipNone;
	// skip type
	switch (scr_flags % 10) {
	case 1: skip = VideoSkipEscape; break;
	case 2: skip = VideoSkipKeyOrMouse; break;
	default: skip = VideoSkipNone; break;
	}
	// video size
	switch ((scr_flags % 100) / 10) {
	case 1: /* play original size, no flag */ break;
	default: flags |= kVideo_Stretch;
	}
	// clear screen
	switch ((scr_flags % 1000) / 100) {
	case 1: /* don't clear screen, no flag */ break;
	default: flags |= kVideo_ClearScreen;
	}

	play_flc_video(numb, flags, skip);
}

void PlayVideo(const char *name, int skip, int scr_flags) {
	EndSkippingUntilCharStops();

	if (_GP(play).fast_forward)
		return;
	if (_G(debug_flags) & DBG_NOVIDEO)
		return;

	// WORKAROUND: This video uses an unsupported codec and
	// the decoder current implementation doesn't allow to
	// continue gracefully
	if (ConfMan.get("gameid") == "donnaavengerofblood" &&
		!strcmp(name, "terminus")) {
		warning("Skipped unsupported \'terminus\' video");
		return;
	}

	// Convert PlayVideo flags to common video flags
	/* NOTE: historically using decimal "flags"
	default (0): the video will be played at original size,
		video's own audio is playing, game sounds muted;
	1: the video will be stretched to full screen;
	10: keep game audio only, video's own audio muted;
	-- since 3.6.0:
	20: play both game audio and video's own audio
	*/
	int flags = kVideo_EnableVideo;
	// video size
	switch (scr_flags % 10) {
	case 1: flags |= kVideo_Stretch; break;
	default: break;
	}
	// audio option
	switch ((scr_flags % 100) / 10) {
	case 1: flags |= kVideo_KeepGameAudio; break;
	case 2: flags |= kVideo_EnableAudio | kVideo_KeepGameAudio; break;
	default: flags |= kVideo_EnableAudio; break;
	}

	// if game audio is disabled, then don't play any sound on the video either
	if (!_GP(usetup).audio_enabled)
		flags &= ~kVideo_EnableAudio;

	if (_G(loaded_game_file_version) < kGameVersion_360_16)
		flags |= kVideo_LegacyFrameSize;

	pause_sound_if_necessary_and_play_video(name, flags, static_cast<VideoSkipType>(skip));
}

#ifndef AGS_NO_VIDEO_PLAYER

void pause_sound_if_necessary_and_play_video(const char *name, int flags, VideoSkipType skip) {
	// Save the game audio parameters, in case we stop these
	int musplaying = _GP(play).cur_music_number, i;
	int ambientWas[MAX_GAME_CHANNELS]{ 0 };
	for (i = NUM_SPEECH_CHANS; i < _GP(game).numGameChannels; i++)
		ambientWas[i] = _GP(ambient)[i].channel;

	// Optionally stop the game audio
	if ((flags & kVideo_KeepGameAudio) == 0) {
		stop_all_sound_and_music();
	}

	if ((strlen(name) > 3) && (ags_stricmp(&name[strlen(name) - 3], "ogv") == 0)) {
		play_theora_video(name, flags, skip, true);
	} else if ((strlen(name) > 3) && (ags_stricmp(&name[strlen(name) - 3], "mpg") == 0)) {
		play_mpeg_video(name, flags, skip, true);
	} else if ((strlen(name) > 3) && (ags_stricmp(&name[strlen(name) - 3], "avi") == 0)) {
		play_avi_video(name, flags, skip, true);
	} else {
		// Unsure what the video type is, so try each in turn
		if (!play_avi_video(name, flags, skip, false) &&
		        !play_mpeg_video(name, flags, skip, false) &&
		        !play_theora_video(name, flags, skip, false))
			Display("Unsupported video '%s'", name);
	}

	// Restore the game audio if we stopped them before the video playback
	if ((flags & kVideo_KeepGameAudio) == 0) {
		update_music_volume();
		if (musplaying >= 0)
			newmusic(musplaying);
		for (i = NUM_SPEECH_CHANS; i < _GP(game).numGameChannels; i++) {
			if (ambientWas[i] > 0)
				PlayAmbientSound(ambientWas[i], _GP(ambient)[i].num,
					_GP(ambient)[i].vol, _GP(ambient)[i].x, _GP(ambient)[i].y);
		}
	}
}

#else

void pause_sound_if_necessary_and_play_video(const char *name, int flags, VideoSkipType skip) {
}

#endif

} // namespace AGS3
