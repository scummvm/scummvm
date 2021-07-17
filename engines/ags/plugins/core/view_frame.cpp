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

#include "ags/plugins/core/view_frame.h"
#include "ags/engine/ac/view_frame.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void ViewFrame::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(ViewFrame::get_Flipped, GetFlipped);
	SCRIPT_METHOD_EXT(ViewFrame::get_Frame, GetFrame);
	SCRIPT_METHOD_EXT(ViewFrame::get_Graphic, GetGraphic);
	SCRIPT_METHOD_EXT(ViewFrame::set_Graphic, SetGraphic);
	SCRIPT_METHOD_EXT(ViewFrame::get_LinkedAudio, GetLinkedAudio);
	SCRIPT_METHOD_EXT(ViewFrame::set_LinkedAudio, SetLinkedAudio);
	SCRIPT_METHOD_EXT(ViewFrame::get_Loop, GetLoop);
	SCRIPT_METHOD_EXT(ViewFrame::get_Sound, GetSound);
	SCRIPT_METHOD_EXT(ViewFrame::set_Sound, SetSound);
	SCRIPT_METHOD_EXT(ViewFrame::get_Speed, GetSpeed);
	SCRIPT_METHOD_EXT(ViewFrame::get_View, GetView);
}

void ViewFrame::GetFlipped(ScriptMethodParams &params) {
	PARAMS1(ScriptViewFrame *, svf);
	params._result = AGS3::ViewFrame_GetFlipped(svf);
}

void ViewFrame::GetFrame(ScriptMethodParams &params) {
	PARAMS1(ScriptViewFrame *, svf);
	params._result = AGS3::ViewFrame_GetFrame(svf);
}

void ViewFrame::GetGraphic(ScriptMethodParams &params) {
	PARAMS1(ScriptViewFrame *, svf);
	params._result = AGS3::ViewFrame_GetGraphic(svf);
}

void ViewFrame::SetGraphic(ScriptMethodParams &params) {
	PARAMS2(ScriptViewFrame *, svf, int, newPic);
	AGS3::ViewFrame_SetGraphic(svf, newPic);
}

void ViewFrame::GetLinkedAudio(ScriptMethodParams &params) {
	PARAMS1(ScriptViewFrame *, svf);
	params._result = AGS3::ViewFrame_GetLinkedAudio(svf);
}

void ViewFrame::SetLinkedAudio(ScriptMethodParams &params) {
	PARAMS2(ScriptViewFrame *, svf, ScriptAudioClip *, clip);
	AGS3::ViewFrame_SetLinkedAudio(svf, clip);
}

void ViewFrame::GetLoop(ScriptMethodParams &params) {
	PARAMS1(ScriptViewFrame *, svf);
	params._result = AGS3::ViewFrame_GetLoop(svf);
}

void ViewFrame::GetSound(ScriptMethodParams &params) {
	PARAMS1(ScriptViewFrame *, svf);
	params._result = AGS3::ViewFrame_GetSound(svf);
}

void ViewFrame::SetSound(ScriptMethodParams &params) {
	PARAMS2(ScriptViewFrame *, svf, int, newSound);
	AGS3::ViewFrame_SetSound(svf, newSound);
}

void ViewFrame::GetSpeed(ScriptMethodParams &params) {
	PARAMS1(ScriptViewFrame *, svf);
	params._result = AGS3::ViewFrame_GetSpeed(svf);
}

void ViewFrame::GetView(ScriptMethodParams &params) {
	PARAMS1(ScriptViewFrame *, svf);
	params._result = AGS3::ViewFrame_GetView(svf);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
