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

//=============================================================================
//
// Video playback interface.
//
// TODO: good future changes:
//  - do not render to the screen right inside the VideoPlayer class,
//    instead write the frame into the bitmap or texture, and expose
//    current frame in the interface to let the engine decide what to do
//    with it.
//
//=============================================================================

#ifndef AGS_ENGINE_MEDIA_VIDEO_VIDEO_H
#define AGS_ENGINE_MEDIA_VIDEO_VIDEO_H

namespace AGS3 {

enum VideoFlags {
	kVideo_EnableVideo = 0x0001,
	kVideo_Stretch = 0x0002,
	kVideo_ClearScreen = 0x0004,
	kVideo_LegacyFrameSize = 0x0008,
	kVideo_EnableAudio = 0x0010,
};

enum VideoSkipType {
	VideoSkipNone = 0,
	VideoSkipEscape = 1,
	VideoSkipAnyKey = 2,
	VideoSkipKeyOrMouse = 3
};

extern bool play_avi_video(const char *name, int flags, VideoSkipType skip, bool showError);
extern bool play_mpeg_video(const char *name, int flags, VideoSkipType skip, bool showError);
extern bool play_theora_video(const char *name, int flags, VideoSkipType skip, bool showError);
extern bool play_flc_video(int numb, int flags, VideoSkipType skip);

// Pause the active video
extern void video_pause();
// Resume the active video
extern void video_resume();

} // namespace AGS3

#endif
