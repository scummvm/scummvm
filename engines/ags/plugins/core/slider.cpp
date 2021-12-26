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

#include "ags/plugins/core/slider.h"
#include "ags/engine/ac/slider.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Slider::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Slider::get_BackgroundGraphic, Slider::GetBackgroundGraphic);
	SCRIPT_METHOD(Slider::set_BackgroundGraphic, Slider::SetBackgroundGraphic);
	SCRIPT_METHOD(Slider::get_HandleGraphic, Slider::GetHandleGraphic);
	SCRIPT_METHOD(Slider::set_HandleGraphic, Slider::SetHandleGraphic);
	SCRIPT_METHOD(Slider::get_HandleOffset, Slider::GetHandleOffset);
	SCRIPT_METHOD(Slider::set_HandleOffset, Slider::SetHandleOffset);
	SCRIPT_METHOD(Slider::get_Max, Slider::GetMax);
	SCRIPT_METHOD(Slider::set_Max, Slider::SetMax);
	SCRIPT_METHOD(Slider::get_Min, Slider::GetMin);
	SCRIPT_METHOD(Slider::set_Min, Slider::SetMin);
	SCRIPT_METHOD(Slider::get_Value, Slider::GetValue);
	SCRIPT_METHOD(Slider::set_Value, Slider::SetValue);
}

void Slider::GetBackgroundGraphic(ScriptMethodParams &params) {
	PARAMS1(GUISlider *, guisl);
	params._result = AGS3::Slider_GetBackgroundGraphic(guisl);
}

void Slider::SetBackgroundGraphic(ScriptMethodParams &params) {
	PARAMS2(GUISlider *, guisl, int, newImage);
	AGS3::Slider_SetBackgroundGraphic(guisl, newImage);
}

void Slider::GetHandleGraphic(ScriptMethodParams &params) {
	PARAMS1(GUISlider *, guisl);
	params._result = AGS3::Slider_GetHandleGraphic(guisl);
}

void Slider::SetHandleGraphic(ScriptMethodParams &params) {
	PARAMS2(GUISlider *, guisl, int, newImage);
	AGS3::Slider_SetHandleGraphic(guisl, newImage);
}

void Slider::GetHandleOffset(ScriptMethodParams &params) {
	PARAMS1(GUISlider *, guisl);
	params._result = AGS3::Slider_GetHandleOffset(guisl);
}

void Slider::SetHandleOffset(ScriptMethodParams &params) {
	PARAMS2(GUISlider *, guisl, int, newOffset);
	AGS3::Slider_SetHandleOffset(guisl, newOffset);
}

void Slider::GetMax(ScriptMethodParams &params) {
	PARAMS1(GUISlider *, guisl);
	params._result = AGS3::Slider_GetMax(guisl);
}

void Slider::SetMax(ScriptMethodParams &params) {
	PARAMS2(GUISlider *, guisl, int, valn);
	AGS3::Slider_SetMax(guisl, valn);
}

void Slider::GetMin(ScriptMethodParams &params) {
	PARAMS1(GUISlider *, guisl);
	params._result = AGS3::Slider_GetMin(guisl);
}

void Slider::SetMin(ScriptMethodParams &params) {
	PARAMS2(GUISlider *, guisl, int, valn);
	AGS3::Slider_SetMin(guisl, valn);
}

void Slider::GetValue(ScriptMethodParams &params) {
	PARAMS1(GUISlider *, guisl);
	params._result = AGS3::Slider_GetValue(guisl);
}

void Slider::SetValue(ScriptMethodParams &params) {
	PARAMS2(GUISlider *, guisl, int, valn);
	AGS3::Slider_SetValue(guisl, valn);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
