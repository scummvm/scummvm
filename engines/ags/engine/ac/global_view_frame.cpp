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

#include "ags/engine/ac/global_view_frame.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/view.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/globals.h"

namespace AGS3 {

void SetFrameSound(int vii, int loop, int frame, int sound) {
	vii--; // convert to 0-based
	AssertFrame("SetFrameSound", vii, loop, frame);

	if (sound < 1) {
		_GP(views)[vii].loops[loop].frames[frame].sound = -1;
	} else {
		ScriptAudioClip *clip = GetAudioClipForOldStyleNumber(_GP(game), false, sound);
		if (clip == nullptr)
			quitprintf("!SetFrameSound: audio clip aSound%d not found", sound);

		_GP(views)[vii].loops[loop].frames[frame].sound =
			_GP(game).IsLegacyAudioSystem() ? sound : clip->id;
		_GP(views)[vii].loops[loop].frames[frame].audioclip = clip->id;
	}
}

} // namespace AGS3
