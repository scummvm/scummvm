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

#include "video/avi_decoder.h"
#include "video/flic_decoder.h"
#include "video/mpegps_decoder.h"
#include "video/theora_decoder.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/core/types.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/ac/asset_helper.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/draw.h"
#include "ags/shared/ac/game_version.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/system.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/ddb.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/main/game_run.h"
#include "ags/shared/util/stream.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/media/video/video.h"
#include "ags/ags.h"
#include "ags/events.h"

namespace AGS3 {

using AGS::Shared::AssetManager;

static bool play_video(Video::VideoDecoder *decoder, const char *name, int flags, VideoSkipType skip, bool showError) {
	std::unique_ptr<Stream> video_stream(_GP(AssetMgr)->OpenAsset(name));
	if (!video_stream) {
		if (showError)
			Display("Unable to load video '%s'", name);
		return false;
	}

	AGS::Shared::ScummVMReadStream *stream = new AGS::Shared::ScummVMReadStream(video_stream.get(), DisposeAfterUse::NO);

	if (!decoder->loadStream(stream)) {
		warning("Unable to decode video '%s'", name);
		return false;
	}

	update_polled_stuff();  // TODO: probably unneeded

	Graphics::Screen scr;
	bool stretchVideo = (flags & kVideo_Stretch) != 0;
	bool enableVideo = (flags & kVideo_EnableVideo) != 0;
	bool enableAudio = (flags & kVideo_EnableAudio) != 0;
	bool clut8Screen = scr.format.isCLUT8();

	if (!enableAudio)
		decoder->setVolume(0);

	update_polled_stuff();

	decoder->start();
	while (!SHOULD_QUIT && !decoder->endOfVideo()) {
		if (decoder->needsUpdate()) {
			// Get the next video frame and draw onto the screen
			const Graphics::Surface *frame = decoder->decodeNextFrame();
			if (clut8Screen && decoder->hasDirtyPalette())
				scr.setPalette(decoder->getPalette(), 0, 256);

			if (frame && enableVideo) {
				Rect dstRect = PlaceInRect(RectWH(0, 0, scr.w, scr.h), RectWH(0, 0, frame->w, frame->h),
					(stretchVideo ? kPlaceStretchProportional : kPlaceCenter));

				if (stretchVideo && frame->w == dstRect.GetWidth() && frame->h == dstRect.GetHeight())
					// Don't need to stretch video after all
					stretchVideo = false;

				if (stretchVideo) {
					scr.transBlitFrom(*frame, Common::Rect(0, 0, frame->w, frame->h),
					                  Common::Rect(dstRect.Left, dstRect.Top, dstRect.Right + 1, dstRect.Bottom + 1));
				} else {
					scr.blitFrom(*frame, Common::Point(dstRect.Left, dstRect.Top));
				}
			}

			scr.update();
		}

		g_system->delayMillis(10);
		::AGS::g_events->pollEvents();

		if (skip != VideoSkipNone) {
			// Check for whether user aborted video
			KeyInput key;
			eAGSMouseButton mbut;
			int mwheelz;
			// Handle all the buffered key events
			bool do_break = false;
			while (ags_keyevent_ready()) {
				if (run_service_key_controls(key)) {
					if ((key.Key == eAGSKeyCodeEscape) && (skip == VideoSkipEscape))
						do_break = true;
					if (skip >= VideoSkipAnyKey)
						do_break = true;  // skip on any key
				}
			}
			if (do_break)
				return true; // skip on key press
			if (run_service_mb_controls(mbut, mwheelz) && mbut >= kMouseNone && skip == VideoSkipKeyOrMouse)
				return true; // skip on mouse click
		}
	}

	invalidate_screen();

	return true;
}

bool play_avi_video(const char *name, int flags, VideoSkipType skip, bool showError) {
	Video::AVIDecoder decoder;
	return play_video(&decoder, name, flags, skip, showError);
}

bool play_mpeg_video(const char *name, int flags, VideoSkipType skip, bool showError) {
	Video::MPEGPSDecoder decoder;
	return play_video(&decoder, name, flags, skip, showError);
}

bool play_theora_video(const char *name, int flags, VideoSkipType skip, bool showError) {
#if !defined (USE_THEORADEC)
	if (showError)
		Display("This games uses Theora videos but ScummVM has been compiled without Theora support");
	return false;
#else
	Video::TheoraDecoder decoder;
	return play_video(&decoder, name, flags, skip, showError);
#endif
}

bool play_flc_video(int numb, int flags, VideoSkipType skip) {
	Video::FlicDecoder decoder;
	String flicName = String::FromFormat("flic%d.flc", numb);

	return play_video(&decoder, flicName, flags, skip, false);
}

void video_pause() {
	// TODO
}

void video_resume() {
	// TODO
}

} // namespace AGS3
