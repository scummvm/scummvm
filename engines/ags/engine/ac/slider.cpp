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

#include "ags/engine/ac/slider.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/util/math.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

void Slider_SetMax(GUISlider *guisl, int valn) {

	if (valn != guisl->MaxValue) {
		guisl->MaxValue = valn;

		if (guisl->Value > guisl->MaxValue)
			guisl->Value = guisl->MaxValue;
		if (guisl->MinValue > guisl->MaxValue)
			quit("!Slider.Max: minimum cannot be greater than maximum");

		guisl->MarkChanged();
	}

}

int Slider_GetMax(GUISlider *guisl) {
	return guisl->MaxValue;
}

void Slider_SetMin(GUISlider *guisl, int valn) {

	if (valn != guisl->MinValue) {
		guisl->MinValue = valn;

		if (guisl->Value < guisl->MinValue)
			guisl->Value = guisl->MinValue;
		if (guisl->MinValue > guisl->MaxValue)
			quit("!Slider.Min: minimum cannot be greater than maximum");

		guisl->MarkChanged();
	}

}

int Slider_GetMin(GUISlider *guisl) {
	return guisl->MinValue;
}

void Slider_SetValue(GUISlider *guisl, int valn) {
	valn = Math::Clamp<int>(valn, guisl->MinValue, guisl->MaxValue);

	if (valn != guisl->Value) {
		guisl->Value = valn;
		guisl->MarkChanged();
	}
}

int Slider_GetValue(GUISlider *guisl) {
	return guisl->Value;
}

int Slider_GetBackgroundGraphic(GUISlider *guisl) {
	return (guisl->BgImage > 0) ? guisl->BgImage : 0;
}

void Slider_SetBackgroundGraphic(GUISlider *guisl, int newImage) {
	if (newImage != guisl->BgImage) {
		guisl->BgImage = newImage;
		guisl->MarkChanged();
	}
}

int Slider_GetHandleGraphic(GUISlider *guisl) {
	return (guisl->HandleImage > 0) ? guisl->HandleImage : 0;
}

void Slider_SetHandleGraphic(GUISlider *guisl, int newImage) {
	if (newImage != guisl->HandleImage) {
		guisl->HandleImage = newImage;
		guisl->MarkChanged();
	}
}

int Slider_GetHandleOffset(GUISlider *guisl) {
	return guisl->HandleOffset;
}

void Slider_SetHandleOffset(GUISlider *guisl, int newOffset) {
	if (newOffset != guisl->HandleOffset) {
		guisl->HandleOffset = newOffset;
		guisl->MarkChanged();
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// int (GUISlider *guisl)
RuntimeScriptValue Sc_Slider_GetBackgroundGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUISlider, Slider_GetBackgroundGraphic);
}

// void (GUISlider *guisl, int newImage)
RuntimeScriptValue Sc_Slider_SetBackgroundGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUISlider, Slider_SetBackgroundGraphic);
}

// int (GUISlider *guisl)
RuntimeScriptValue Sc_Slider_GetHandleGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUISlider, Slider_GetHandleGraphic);
}

// void (GUISlider *guisl, int newImage)
RuntimeScriptValue Sc_Slider_SetHandleGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUISlider, Slider_SetHandleGraphic);
}

// int (GUISlider *guisl)
RuntimeScriptValue Sc_Slider_GetHandleOffset(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUISlider, Slider_GetHandleOffset);
}

// void (GUISlider *guisl, int newOffset)
RuntimeScriptValue Sc_Slider_SetHandleOffset(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUISlider, Slider_SetHandleOffset);
}

// int (GUISlider *guisl)
RuntimeScriptValue Sc_Slider_GetMax(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUISlider, Slider_GetMax);
}

// void (GUISlider *guisl, int valn)
RuntimeScriptValue Sc_Slider_SetMax(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUISlider, Slider_SetMax);
}

// int (GUISlider *guisl)
RuntimeScriptValue Sc_Slider_GetMin(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUISlider, Slider_GetMin);
}

// void (GUISlider *guisl, int valn)
RuntimeScriptValue Sc_Slider_SetMin(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUISlider, Slider_SetMin);
}

// int (GUISlider *guisl)
RuntimeScriptValue Sc_Slider_GetValue(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUISlider, Slider_GetValue);
}

// void Slider_SetValue(GUISlider *guisl, int valn)
RuntimeScriptValue Sc_Slider_SetValue(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUISlider, Slider_SetValue);
}


void RegisterSliderAPI() {
	ScFnRegister slider_api[] = {
		{"Slider::get_BackgroundGraphic", API_FN_PAIR(Slider_GetBackgroundGraphic)},
		{"Slider::set_BackgroundGraphic", API_FN_PAIR(Slider_SetBackgroundGraphic)},
		{"Slider::get_HandleGraphic", API_FN_PAIR(Slider_GetHandleGraphic)},
		{"Slider::set_HandleGraphic", API_FN_PAIR(Slider_SetHandleGraphic)},
		{"Slider::get_HandleOffset", API_FN_PAIR(Slider_GetHandleOffset)},
		{"Slider::set_HandleOffset", API_FN_PAIR(Slider_SetHandleOffset)},
		{"Slider::get_Max", API_FN_PAIR(Slider_GetMax)},
		{"Slider::set_Max", API_FN_PAIR(Slider_SetMax)},
		{"Slider::get_Min", API_FN_PAIR(Slider_GetMin)},
		{"Slider::set_Min", API_FN_PAIR(Slider_SetMin)},
		{"Slider::get_Value", API_FN_PAIR(Slider_GetValue)},
		{"Slider::set_Value", API_FN_PAIR(Slider_SetValue)},
	};

	ccAddExternalFunctions361(slider_api);
}

} // namespace AGS3
