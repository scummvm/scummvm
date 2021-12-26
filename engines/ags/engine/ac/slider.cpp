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
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/globals.h"

namespace AGS3 {

// *** SLIDER FUNCTIONS

void Slider_SetMax(GUISlider *guisl, int valn) {

	if (valn != guisl->MaxValue) {
		guisl->MaxValue = valn;

		if (guisl->Value > guisl->MaxValue)
			guisl->Value = guisl->MaxValue;
		if (guisl->MinValue > guisl->MaxValue)
			quit("!Slider.Max: minimum cannot be greater than maximum");

		guisl->NotifyParentChanged();
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

		guisl->NotifyParentChanged();
	}

}

int Slider_GetMin(GUISlider *guisl) {
	return guisl->MinValue;
}

void Slider_SetValue(GUISlider *guisl, int valn) {
	if (valn > guisl->MaxValue) valn = guisl->MaxValue;
	if (valn < guisl->MinValue) valn = guisl->MinValue;

	if (valn != guisl->Value) {
		guisl->Value = valn;
		guisl->NotifyParentChanged();
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
		guisl->NotifyParentChanged();
	}
}

int Slider_GetHandleGraphic(GUISlider *guisl) {
	return (guisl->HandleImage > 0) ? guisl->HandleImage : 0;
}

void Slider_SetHandleGraphic(GUISlider *guisl, int newImage) {
	if (newImage != guisl->HandleImage) {
		guisl->HandleImage = newImage;
		guisl->NotifyParentChanged();
	}
}

int Slider_GetHandleOffset(GUISlider *guisl) {
	return guisl->HandleOffset;
}

void Slider_SetHandleOffset(GUISlider *guisl, int newOffset) {
	if (newOffset != guisl->HandleOffset) {
		guisl->HandleOffset = newOffset;
		guisl->NotifyParentChanged();
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
	ccAddExternalObjectFunction("Slider::get_BackgroundGraphic", Sc_Slider_GetBackgroundGraphic);
	ccAddExternalObjectFunction("Slider::set_BackgroundGraphic", Sc_Slider_SetBackgroundGraphic);
	ccAddExternalObjectFunction("Slider::get_HandleGraphic", Sc_Slider_GetHandleGraphic);
	ccAddExternalObjectFunction("Slider::set_HandleGraphic", Sc_Slider_SetHandleGraphic);
	ccAddExternalObjectFunction("Slider::get_HandleOffset", Sc_Slider_GetHandleOffset);
	ccAddExternalObjectFunction("Slider::set_HandleOffset", Sc_Slider_SetHandleOffset);
	ccAddExternalObjectFunction("Slider::get_Max", Sc_Slider_GetMax);
	ccAddExternalObjectFunction("Slider::set_Max", Sc_Slider_SetMax);
	ccAddExternalObjectFunction("Slider::get_Min", Sc_Slider_GetMin);
	ccAddExternalObjectFunction("Slider::set_Min", Sc_Slider_SetMin);
	ccAddExternalObjectFunction("Slider::get_Value", Sc_Slider_GetValue);
	ccAddExternalObjectFunction("Slider::set_Value", Sc_Slider_SetValue);
}

} // namespace AGS3
