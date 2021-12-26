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

#include "ags/plugins/core/view_frame.h"
#include "ags/engine/ac/view_frame.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void ViewFrame::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(ViewFrame::get_Flipped, ViewFrame::GetFlipped);
	SCRIPT_METHOD(ViewFrame::get_Frame, ViewFrame::GetFrame);
	SCRIPT_METHOD(ViewFrame::get_Graphic, ViewFrame::GetGraphic);
	SCRIPT_METHOD(ViewFrame::set_Graphic, ViewFrame::SetGraphic);
	SCRIPT_METHOD(ViewFrame::get_LinkedAudio, ViewFrame::GetLinkedAudio);
	SCRIPT_METHOD(ViewFrame::set_LinkedAudio, ViewFrame::SetLinkedAudio);
	SCRIPT_METHOD(ViewFrame::get_Loop, ViewFrame::GetLoop);
	SCRIPT_METHOD(ViewFrame::get_Sound, ViewFrame::GetSound);
	SCRIPT_METHOD(ViewFrame::set_Sound, ViewFrame::SetSound);
	SCRIPT_METHOD(ViewFrame::get_Speed, ViewFrame::GetSpeed);
	SCRIPT_METHOD(ViewFrame::get_View, ViewFrame::GetView);
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
