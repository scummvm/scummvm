//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include "ac/slider.h"
#include "ac/common.h"

// *** SLIDER FUNCTIONS

void Slider_SetMax(GUISlider *guisl, int valn) {

    if (valn != guisl->MaxValue) {
        guisl->MaxValue = valn;

        if (guisl->Value > guisl->MaxValue)
            guisl->Value = guisl->MaxValue;
        if (guisl->MinValue > guisl->MaxValue)
            quit("!Slider.Max: minimum cannot be greater than maximum");

        guis_need_update = 1;
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

        guis_need_update = 1;
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
        guis_need_update = 1;
    }
}

int Slider_GetValue(GUISlider *guisl) {
    return guisl->Value;
}

int Slider_GetBackgroundGraphic(GUISlider *guisl) {
    return (guisl->BgImage > 0) ? guisl->BgImage : 0;
}

void Slider_SetBackgroundGraphic(GUISlider *guisl, int newImage) 
{
    if (newImage != guisl->BgImage)
    {
        guisl->BgImage = newImage;
        guis_need_update = 1;
    }
}

int Slider_GetHandleGraphic(GUISlider *guisl) {
    return (guisl->HandleImage > 0) ? guisl->HandleImage : 0;
}

void Slider_SetHandleGraphic(GUISlider *guisl, int newImage) 
{
    if (newImage != guisl->HandleImage)
    {
        guisl->HandleImage = newImage;
        guis_need_update = 1;
    }
}

int Slider_GetHandleOffset(GUISlider *guisl) {
    return guisl->HandleOffset;
}

void Slider_SetHandleOffset(GUISlider *guisl, int newOffset) 
{
    if (newOffset != guisl->HandleOffset)
    {
        guisl->HandleOffset = newOffset;
        guis_need_update = 1;
    }
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

#include "debug/out.h"
#include "script/script_api.h"
#include "script/script_runtime.h"

// int (GUISlider *guisl)
RuntimeScriptValue Sc_Slider_GetBackgroundGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(GUISlider, Slider_GetBackgroundGraphic);
}

// void (GUISlider *guisl, int newImage)
RuntimeScriptValue Sc_Slider_SetBackgroundGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(GUISlider, Slider_SetBackgroundGraphic);
}

// int (GUISlider *guisl)
RuntimeScriptValue Sc_Slider_GetHandleGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(GUISlider, Slider_GetHandleGraphic);
}

// void (GUISlider *guisl, int newImage)
RuntimeScriptValue Sc_Slider_SetHandleGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(GUISlider, Slider_SetHandleGraphic);
}

// int (GUISlider *guisl)
RuntimeScriptValue Sc_Slider_GetHandleOffset(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(GUISlider, Slider_GetHandleOffset);
}

// void (GUISlider *guisl, int newOffset)
RuntimeScriptValue Sc_Slider_SetHandleOffset(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(GUISlider, Slider_SetHandleOffset);
}

// int (GUISlider *guisl)
RuntimeScriptValue Sc_Slider_GetMax(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(GUISlider, Slider_GetMax);
}

// void (GUISlider *guisl, int valn)
RuntimeScriptValue Sc_Slider_SetMax(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(GUISlider, Slider_SetMax);
}

// int (GUISlider *guisl)
RuntimeScriptValue Sc_Slider_GetMin(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(GUISlider, Slider_GetMin);
}

// void (GUISlider *guisl, int valn)
RuntimeScriptValue Sc_Slider_SetMin(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(GUISlider, Slider_SetMin);
}

// int (GUISlider *guisl)
RuntimeScriptValue Sc_Slider_GetValue(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(GUISlider, Slider_GetValue);
}

// void Slider_SetValue(GUISlider *guisl, int valn)
RuntimeScriptValue Sc_Slider_SetValue(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(GUISlider, Slider_SetValue);
}


void RegisterSliderAPI()
{
    ccAddExternalObjectFunction("Slider::get_BackgroundGraphic",    Sc_Slider_GetBackgroundGraphic);
    ccAddExternalObjectFunction("Slider::set_BackgroundGraphic",    Sc_Slider_SetBackgroundGraphic);
    ccAddExternalObjectFunction("Slider::get_HandleGraphic",        Sc_Slider_GetHandleGraphic);
    ccAddExternalObjectFunction("Slider::set_HandleGraphic",        Sc_Slider_SetHandleGraphic);
    ccAddExternalObjectFunction("Slider::get_HandleOffset",         Sc_Slider_GetHandleOffset);
    ccAddExternalObjectFunction("Slider::set_HandleOffset",         Sc_Slider_SetHandleOffset);
    ccAddExternalObjectFunction("Slider::get_Max",                  Sc_Slider_GetMax);
    ccAddExternalObjectFunction("Slider::set_Max",                  Sc_Slider_SetMax);
    ccAddExternalObjectFunction("Slider::get_Min",                  Sc_Slider_GetMin);
    ccAddExternalObjectFunction("Slider::set_Min",                  Sc_Slider_SetMin);
    ccAddExternalObjectFunction("Slider::get_Value",                Sc_Slider_GetValue);
    ccAddExternalObjectFunction("Slider::set_Value",                Sc_Slider_SetValue);

    /* ----------------------- Registering unsafe exports for plugins -----------------------*/

    ccAddExternalFunctionForPlugin("Slider::get_BackgroundGraphic",    (void*)Slider_GetBackgroundGraphic);
    ccAddExternalFunctionForPlugin("Slider::set_BackgroundGraphic",    (void*)Slider_SetBackgroundGraphic);
    ccAddExternalFunctionForPlugin("Slider::get_HandleGraphic",        (void*)Slider_GetHandleGraphic);
    ccAddExternalFunctionForPlugin("Slider::set_HandleGraphic",        (void*)Slider_SetHandleGraphic);
    ccAddExternalFunctionForPlugin("Slider::get_HandleOffset",         (void*)Slider_GetHandleOffset);
    ccAddExternalFunctionForPlugin("Slider::set_HandleOffset",         (void*)Slider_SetHandleOffset);
    ccAddExternalFunctionForPlugin("Slider::get_Max",                  (void*)Slider_GetMax);
    ccAddExternalFunctionForPlugin("Slider::set_Max",                  (void*)Slider_SetMax);
    ccAddExternalFunctionForPlugin("Slider::get_Min",                  (void*)Slider_GetMin);
    ccAddExternalFunctionForPlugin("Slider::set_Min",                  (void*)Slider_SetMin);
    ccAddExternalFunctionForPlugin("Slider::get_Value",                (void*)Slider_GetValue);
    ccAddExternalFunctionForPlugin("Slider::set_Value",                (void*)Slider_SetValue);
}
