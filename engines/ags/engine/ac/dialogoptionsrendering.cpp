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

#include "ac/dialog.h"
#include "ac/dialogtopic.h"
#include "ac/dialogoptionsrendering.h"
#include "ac/gamestructdefines.h"
#include "debug/debug_log.h"
#include "script/runtimescriptvalue.h"
#include "ac/dynobj/cc_dialog.h"

extern DialogTopic *dialog;
extern CCDialog ccDynamicDialog;

// ** SCRIPT DIALOGOPTIONSRENDERING OBJECT

void DialogOptionsRendering_Update(ScriptDialogOptionsRendering *dlgOptRender)
{
    dlgOptRender->needRepaint = true;
}

bool DialogOptionsRendering_RunActiveOption(ScriptDialogOptionsRendering *dlgOptRender)
{
    dlgOptRender->chosenOptionID = dlgOptRender->activeOptionID;
    return dlgOptRender->chosenOptionID >= 0;
}

int DialogOptionsRendering_GetX(ScriptDialogOptionsRendering *dlgOptRender)
{
    return dlgOptRender->x;
}

void DialogOptionsRendering_SetX(ScriptDialogOptionsRendering *dlgOptRender, int newX)
{
    dlgOptRender->x = newX;
}

int DialogOptionsRendering_GetY(ScriptDialogOptionsRendering *dlgOptRender)
{
    return dlgOptRender->y;
}

void DialogOptionsRendering_SetY(ScriptDialogOptionsRendering *dlgOptRender, int newY)
{
    dlgOptRender->y = newY;
}

int DialogOptionsRendering_GetWidth(ScriptDialogOptionsRendering *dlgOptRender)
{
    return dlgOptRender->width;
}

void DialogOptionsRendering_SetWidth(ScriptDialogOptionsRendering *dlgOptRender, int newWidth)
{
    dlgOptRender->width = newWidth;
}

int DialogOptionsRendering_GetHeight(ScriptDialogOptionsRendering *dlgOptRender)
{
    return dlgOptRender->height;
}

void DialogOptionsRendering_SetHeight(ScriptDialogOptionsRendering *dlgOptRender, int newHeight)
{
    dlgOptRender->height = newHeight;
}

int DialogOptionsRendering_GetHasAlphaChannel(ScriptDialogOptionsRendering *dlgOptRender)
{
    return dlgOptRender->hasAlphaChannel;
}

void DialogOptionsRendering_SetHasAlphaChannel(ScriptDialogOptionsRendering *dlgOptRender, bool hasAlphaChannel)
{
    dlgOptRender->hasAlphaChannel = hasAlphaChannel;
}

int DialogOptionsRendering_GetParserTextboxX(ScriptDialogOptionsRendering *dlgOptRender)
{
    return dlgOptRender->parserTextboxX;
}

void DialogOptionsRendering_SetParserTextboxX(ScriptDialogOptionsRendering *dlgOptRender, int newX)
{
    dlgOptRender->parserTextboxX = newX;
}

int DialogOptionsRendering_GetParserTextboxY(ScriptDialogOptionsRendering *dlgOptRender)
{
    return dlgOptRender->parserTextboxY;
}

void DialogOptionsRendering_SetParserTextboxY(ScriptDialogOptionsRendering *dlgOptRender, int newY)
{
    dlgOptRender->parserTextboxY = newY;
}

int DialogOptionsRendering_GetParserTextboxWidth(ScriptDialogOptionsRendering *dlgOptRender)
{
    return dlgOptRender->parserTextboxWidth;
}

void DialogOptionsRendering_SetParserTextboxWidth(ScriptDialogOptionsRendering *dlgOptRender, int newWidth)
{
    dlgOptRender->parserTextboxWidth = newWidth;
}

ScriptDialog* DialogOptionsRendering_GetDialogToRender(ScriptDialogOptionsRendering *dlgOptRender)
{
    return &scrDialog[dlgOptRender->dialogID];
}

ScriptDrawingSurface* DialogOptionsRendering_GetSurface(ScriptDialogOptionsRendering *dlgOptRender)
{
    dlgOptRender->surfaceAccessed = true;
    return dlgOptRender->surfaceToRenderTo;
}

int DialogOptionsRendering_GetActiveOptionID(ScriptDialogOptionsRendering *dlgOptRender)
{
    return dlgOptRender->activeOptionID + 1;
}

void DialogOptionsRendering_SetActiveOptionID(ScriptDialogOptionsRendering *dlgOptRender, int activeOptionID)
{
    int optionCount = dialog[scrDialog[dlgOptRender->dialogID].id].numoptions;
    if ((activeOptionID < 0) || (activeOptionID > optionCount))
        quitprintf("DialogOptionsRenderingInfo.ActiveOptionID: invalid ID specified for this dialog (specified %d, valid range: 1..%d)", activeOptionID, optionCount);

    if (dlgOptRender->activeOptionID != activeOptionID - 1)
    {
        dlgOptRender->activeOptionID = activeOptionID - 1;
        dlgOptRender->needRepaint = true;
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

RuntimeScriptValue Sc_DialogOptionsRendering_Update(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID(ScriptDialogOptionsRendering, DialogOptionsRendering_Update);
}

RuntimeScriptValue Sc_DialogOptionsRendering_RunActiveOption(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_BOOL(ScriptDialogOptionsRendering, DialogOptionsRendering_RunActiveOption);
}

// int (ScriptDialogOptionsRendering *dlgOptRender)
RuntimeScriptValue Sc_DialogOptionsRendering_GetActiveOptionID(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDialogOptionsRendering, DialogOptionsRendering_GetActiveOptionID);
}

// void (ScriptDialogOptionsRendering *dlgOptRender, int activeOptionID)
RuntimeScriptValue Sc_DialogOptionsRendering_SetActiveOptionID(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptDialogOptionsRendering, DialogOptionsRendering_SetActiveOptionID);
}

// ScriptDialog* (ScriptDialogOptionsRendering *dlgOptRender)
RuntimeScriptValue Sc_DialogOptionsRendering_GetDialogToRender(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_OBJ(ScriptDialogOptionsRendering, ScriptDialog, ccDynamicDialog, DialogOptionsRendering_GetDialogToRender);
}

// int (ScriptDialogOptionsRendering *dlgOptRender)
RuntimeScriptValue Sc_DialogOptionsRendering_GetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDialogOptionsRendering, DialogOptionsRendering_GetHeight);
}

// void (ScriptDialogOptionsRendering *dlgOptRender, int newHeight)
RuntimeScriptValue Sc_DialogOptionsRendering_SetHeight(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptDialogOptionsRendering, DialogOptionsRendering_SetHeight);
}

// int (ScriptDialogOptionsRendering *dlgOptRender)
RuntimeScriptValue Sc_DialogOptionsRendering_GetParserTextboxX(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDialogOptionsRendering, DialogOptionsRendering_GetParserTextboxX);
}

// void (ScriptDialogOptionsRendering *dlgOptRender, int newX)
RuntimeScriptValue Sc_DialogOptionsRendering_SetParserTextboxX(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptDialogOptionsRendering, DialogOptionsRendering_SetParserTextboxX);
}

// int (ScriptDialogOptionsRendering *dlgOptRender)
RuntimeScriptValue Sc_DialogOptionsRendering_GetParserTextboxY(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDialogOptionsRendering, DialogOptionsRendering_GetParserTextboxY);
}

// void (ScriptDialogOptionsRendering *dlgOptRender, int newY)
RuntimeScriptValue Sc_DialogOptionsRendering_SetParserTextboxY(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptDialogOptionsRendering, DialogOptionsRendering_SetParserTextboxY);
}

// int (ScriptDialogOptionsRendering *dlgOptRender)
RuntimeScriptValue Sc_DialogOptionsRendering_GetParserTextboxWidth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDialogOptionsRendering, DialogOptionsRendering_GetParserTextboxWidth);
}

// void (ScriptDialogOptionsRendering *dlgOptRender, int newWidth)
RuntimeScriptValue Sc_DialogOptionsRendering_SetParserTextboxWidth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptDialogOptionsRendering, DialogOptionsRendering_SetParserTextboxWidth);
}

// ScriptDrawingSurface* (ScriptDialogOptionsRendering *dlgOptRender)
RuntimeScriptValue Sc_DialogOptionsRendering_GetSurface(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_OBJAUTO(ScriptDialogOptionsRendering, ScriptDrawingSurface, DialogOptionsRendering_GetSurface);
}

// int (ScriptDialogOptionsRendering *dlgOptRender)
RuntimeScriptValue Sc_DialogOptionsRendering_GetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDialogOptionsRendering, DialogOptionsRendering_GetWidth);
}

// void (ScriptDialogOptionsRendering *dlgOptRender, int newWidth)
RuntimeScriptValue Sc_DialogOptionsRendering_SetWidth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptDialogOptionsRendering, DialogOptionsRendering_SetWidth);
}

// int (ScriptDialogOptionsRendering *dlgOptRender)
RuntimeScriptValue Sc_DialogOptionsRendering_GetX(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDialogOptionsRendering, DialogOptionsRendering_GetX);
}

// void (ScriptDialogOptionsRendering *dlgOptRender, int newX)
RuntimeScriptValue Sc_DialogOptionsRendering_SetX(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptDialogOptionsRendering, DialogOptionsRendering_SetX);
}

// int (ScriptDialogOptionsRendering *dlgOptRender)
RuntimeScriptValue Sc_DialogOptionsRendering_GetY(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDialogOptionsRendering, DialogOptionsRendering_GetY);
}

// void (ScriptDialogOptionsRendering *dlgOptRender, int newY)
RuntimeScriptValue Sc_DialogOptionsRendering_SetY(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(ScriptDialogOptionsRendering, DialogOptionsRendering_SetY);
}

RuntimeScriptValue Sc_DialogOptionsRendering_GetHasAlphaChannel(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDialogOptionsRendering, DialogOptionsRendering_GetHasAlphaChannel);
}

RuntimeScriptValue Sc_DialogOptionsRendering_SetHasAlphaChannel(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PBOOL(ScriptDialogOptionsRendering, DialogOptionsRendering_SetHasAlphaChannel);
}


void RegisterDialogOptionsRenderingAPI()
{
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::Update^0",             Sc_DialogOptionsRendering_Update);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::RunActiveOption^0",    Sc_DialogOptionsRendering_RunActiveOption);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::get_ActiveOptionID",   Sc_DialogOptionsRendering_GetActiveOptionID);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::set_ActiveOptionID",   Sc_DialogOptionsRendering_SetActiveOptionID);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::get_DialogToRender",   Sc_DialogOptionsRendering_GetDialogToRender);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::get_Height",           Sc_DialogOptionsRendering_GetHeight);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::set_Height",           Sc_DialogOptionsRendering_SetHeight);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::get_ParserTextBoxX",   Sc_DialogOptionsRendering_GetParserTextboxX);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::set_ParserTextBoxX",   Sc_DialogOptionsRendering_SetParserTextboxX);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::get_ParserTextBoxY",   Sc_DialogOptionsRendering_GetParserTextboxY);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::set_ParserTextBoxY",   Sc_DialogOptionsRendering_SetParserTextboxY);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::get_ParserTextBoxWidth", Sc_DialogOptionsRendering_GetParserTextboxWidth);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::set_ParserTextBoxWidth", Sc_DialogOptionsRendering_SetParserTextboxWidth);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::get_Surface",          Sc_DialogOptionsRendering_GetSurface);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::get_Width",            Sc_DialogOptionsRendering_GetWidth);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::set_Width",            Sc_DialogOptionsRendering_SetWidth);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::get_X",                Sc_DialogOptionsRendering_GetX);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::set_X",                Sc_DialogOptionsRendering_SetX);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::get_Y",                Sc_DialogOptionsRendering_GetY);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::set_Y",                Sc_DialogOptionsRendering_SetY);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::get_HasAlphaChannel",  Sc_DialogOptionsRendering_GetHasAlphaChannel);
    ccAddExternalObjectFunction("DialogOptionsRenderingInfo::set_HasAlphaChannel",  Sc_DialogOptionsRendering_SetHasAlphaChannel);

    /* ----------------------- Registering unsafe exports for plugins -----------------------*/

    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::get_ActiveOptionID",   (void*)DialogOptionsRendering_GetActiveOptionID);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::set_ActiveOptionID",   (void*)DialogOptionsRendering_SetActiveOptionID);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::get_DialogToRender",   (void*)DialogOptionsRendering_GetDialogToRender);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::get_Height",           (void*)DialogOptionsRendering_GetHeight);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::set_Height",           (void*)DialogOptionsRendering_SetHeight);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::get_ParserTextBoxX",   (void*)DialogOptionsRendering_GetParserTextboxX);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::set_ParserTextBoxX",   (void*)DialogOptionsRendering_SetParserTextboxX);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::get_ParserTextBoxY",   (void*)DialogOptionsRendering_GetParserTextboxY);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::set_ParserTextBoxY",   (void*)DialogOptionsRendering_SetParserTextboxY);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::get_ParserTextBoxWidth", (void*)DialogOptionsRendering_GetParserTextboxWidth);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::set_ParserTextBoxWidth", (void*)DialogOptionsRendering_SetParserTextboxWidth);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::get_Surface",          (void*)DialogOptionsRendering_GetSurface);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::get_Width",            (void*)DialogOptionsRendering_GetWidth);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::set_Width",            (void*)DialogOptionsRendering_SetWidth);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::get_X",                (void*)DialogOptionsRendering_GetX);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::set_X",                (void*)DialogOptionsRendering_SetX);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::get_Y",                (void*)DialogOptionsRendering_GetY);
    ccAddExternalFunctionForPlugin("DialogOptionsRenderingInfo::set_Y",                (void*)DialogOptionsRendering_SetY);
}
