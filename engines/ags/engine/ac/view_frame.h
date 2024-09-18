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

#ifndef AGS_ENGINE_AC_VIEW_FRAME_H
#define AGS_ENGINE_AC_VIEW_FRAME_H

#include "ags/engine/ac/runtime_defines.h"
#include "ags/shared/ac/view.h"
#include "ags/shared/ac/dynobj/script_audio_clip.h"
#include "ags/engine/ac/dynobj/script_view_frame.h"
#include "ags/shared/gfx/bitmap.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Graphics;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

int  ViewFrame_GetFlipped(ScriptViewFrame *svf);
int  ViewFrame_GetGraphic(ScriptViewFrame *svf);
void ViewFrame_SetGraphic(ScriptViewFrame *svf, int newPic);
ScriptAudioClip *ViewFrame_GetLinkedAudio(ScriptViewFrame *svf);
void ViewFrame_SetLinkedAudio(ScriptViewFrame *svf, ScriptAudioClip *clip);
int  ViewFrame_GetSound(ScriptViewFrame *svf);
void ViewFrame_SetSound(ScriptViewFrame *svf, int newSound);
int  ViewFrame_GetSpeed(ScriptViewFrame *svf);
int  ViewFrame_GetView(ScriptViewFrame *svf);
int  ViewFrame_GetLoop(ScriptViewFrame *svf);
int  ViewFrame_GetFrame(ScriptViewFrame *svf);

// Calculate the frame sound volume from different factors;
// pass scale as 100 if volume scaling is disabled
// NOTE: historically scales only in 0-100 range :/
int CalcFrameSoundVolume(int obj_vol, int anim_vol, int scale = 100);
// Handle the new animation frame (play linked sounds, etc);
// sound_volume is an optional *relative* factor, 100 is default (unchanged)
void CheckViewFrame(int view, int loop, int frame, int sound_volume = 100);
// draws a view frame, flipped if appropriate
void DrawViewFrame(Shared::Bitmap *ds, const ViewFrame *vframe, int x, int y, bool alpha_blend = false);

} // namespace AGS3

#endif
