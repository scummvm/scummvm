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

#include "ags/engine/ac/global_viewframe.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/view.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/media/audio/audio_system.h"

namespace AGS3 {

extern GameSetupStruct game;
extern ViewStruct *views;


void SetFrameSound(int vii, int loop, int frame, int sound) {
	if ((vii < 1) || (vii > game.numviews))
		quit("!SetFrameSound: invalid view number");
	vii--;

	if (loop >= views[vii].numLoops)
		quit("!SetFrameSound: invalid loop number");

	if (frame >= views[vii].loops[loop].numFrames)
		quit("!SetFrameSound: invalid frame number");

	if (sound < 1) {
		views[vii].loops[loop].frames[frame].sound = -1;
	} else {
		ScriptAudioClip *clip = GetAudioClipForOldStyleNumber(game, false, sound);
		if (clip == nullptr)
			quitprintf("!SetFrameSound: audio clip aSound%d not found", sound);

		views[vii].loops[loop].frames[frame].sound = clip->id + (game.IsLegacyAudioSystem() ? 0x10000000 : 0);
	}
}

} // namespace AGS3
