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

#include "video/theora_decoder.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/core/types.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/ac/asset_helper.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/draw.h"
#include "ags/shared/ac/game_version.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/system.h"
#include "ags/shared/core/assetmanager.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/ddb.h"
#include "ags/engine/gfx/graphicsdriver.h"
#include "ags/engine/main/game_run.h"
#include "ags/shared/util/stream.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/ags.h"
#include "ags/events.h"
#include "ags/engine/media/video/video.h"

namespace AGS3 {

using AGS::Shared::AssetManager;

void play_theora_video(const char *name, int skip, int flags) {
#if !defined (USE_THEORADEC)
	Display("This games uses Theora videos but ScummVM has been compiled without Theora support");
#else
	std::unique_ptr<Stream> video_stream(AssetManager::OpenAsset(name));
	if (!video_stream) {
		Display("Unable to load theora video '%s'", name);
		return;
	}

	update_polled_stuff_if_runtime();

	Graphics::Screen &scr = *::AGS::g_vm->_rawScreen;
	bool stretchVideo = (flags % 10) != 0;
	int canAbort = skip;
	bool ignoreAudio = (flags >= 10);

	if (!ignoreAudio) {
		stop_all_sound_and_music();
	}

	Video::TheoraDecoder decoder;
	AGS::Shared::ScummVMReadStream *stream = new AGS::Shared::ScummVMReadStream(video_stream.get(), DisposeAfterUse::NO);

	if (!decoder.loadStream(stream)) {
		delete stream;
		Display("Unable to decode theora video '%s'", name);
		return;
	}

	update_polled_stuff_if_runtime();

	decoder.start();
	while (!SHOULD_QUIT && !decoder.endOfVideo()) {
		if (decoder.needsUpdate()) {
			// Get the next video frame and draw onto the screen
			const Graphics::Surface *frame = decoder.decodeNextFrame();

			if (stretchVideo && frame->w == scr.w && frame->h == scr.h)
				// Don't need to stretch video after all
				stretchVideo = false;

			if (stretchVideo) {
				scr.transBlitFrom(*frame, Common::Rect(0, 0, frame->w, frame->h),
					Common::Rect(0, 0, scr.w, scr.h));
			} else {
				scr.blitFrom(*frame, Common::Point((scr.w - frame->w) / 2,
					(scr.h - frame->h) / 2));
			}

			scr.update();
		}

		g_system->delayMillis(10);
		::AGS::g_events->pollEvents();

		if (canAbort) {
			// Check for whether user aborted video
			int key, mbut, mwheelz;
			if (run_service_key_controls(key)) {
				if (key == 27 && canAbort)
					return;
				if (canAbort >= 2)
					return;  // skip on any key
			}
			if (run_service_mb_controls(mbut, mwheelz) && mbut >= 0 && canAbort == 3) {
				return; // skip on mouse click
			}
		}
	}

	invalidate_screen();
#endif
}

void play_flc_file(int numb, int playflags) {
	warning("TODO: play_flc_file");
}

void video_on_gfxmode_changed() {
	warning("TODO: video_on_gfxmode_changed");
}

} // namespace AGS3
