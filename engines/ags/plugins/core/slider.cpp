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

#include "ags/plugins/core/slider.h"
#include "ags/engine/ac/slider.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Slider::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Slider::get_BackgroundGraphic, GetBackgroundGraphic);
	SCRIPT_METHOD_EXT(Slider::set_BackgroundGraphic, SetBackgroundGraphic);
	SCRIPT_METHOD_EXT(Slider::get_HandleGraphic, GetHandleGraphic);
	SCRIPT_METHOD_EXT(Slider::set_HandleGraphic, SetHandleGraphic);
	SCRIPT_METHOD_EXT(Slider::get_HandleOffset, GetHandleOffset);
	SCRIPT_METHOD_EXT(Slider::set_HandleOffset, SetHandleOffset);
	SCRIPT_METHOD_EXT(Slider::get_Max, GetMax);
	SCRIPT_METHOD_EXT(Slider::set_Max, SetMax);
	SCRIPT_METHOD_EXT(Slider::get_Min, GetMin);
	SCRIPT_METHOD_EXT(Slider::set_Min, SetMin);
	SCRIPT_METHOD_EXT(Slider::get_Value, GetValue);
	SCRIPT_METHOD_EXT(Slider::set_Value, SetValue);
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
