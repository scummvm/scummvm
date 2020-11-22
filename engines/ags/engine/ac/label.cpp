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

#include <string.h>
#include "ac/label.h"
#include "ac/common.h"
#include "ac/gamesetupstruct.h"
#include "ac/global_translation.h"
#include "ac/string.h"

namespace AGS3 {

extern GameSetupStruct game;

// ** LABEL FUNCTIONS

const char *Label_GetText_New(GUILabel *labl) {
	return CreateNewScriptString(labl->GetText());
}

void Label_GetText(GUILabel *labl, char *buffer) {
	strcpy(buffer, labl->GetText());
}

void Label_SetText(GUILabel *labl, const char *newtx) {
	newtx = get_translation(newtx);

	if (strcmp(labl->GetText(), newtx)) {
		guis_need_update = 1;
		labl->SetText(newtx);
	}
}

int Label_GetTextAlignment(GUILabel *labl) {
	return labl->TextAlignment;
}

void Label_SetTextAlignment(GUILabel *labl, int align) {
	if (labl->TextAlignment != align) {
		labl->TextAlignment = (HorAlignment)align;
		guis_need_update = 1;
	}
}

int Label_GetColor(GUILabel *labl) {
	return labl->TextColor;
}

void Label_SetColor(GUILabel *labl, int colr) {
	if (labl->TextColor != colr) {
		labl->TextColor = colr;
		guis_need_update = 1;
	}
}

int Label_GetFont(GUILabel *labl) {
	return labl->Font;
}

void Label_SetFont(GUILabel *guil, int fontnum) {
	if ((fontnum < 0) || (fontnum >= game.numfonts))
		quit("!SetLabelFont: invalid font number.");

	if (fontnum != guil->Font) {
		guil->Font = fontnum;
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
#include "ac/dynobj/scriptstring.h"

extern ScriptString myScriptStringImpl;

// void (GUILabel *labl, char *buffer)
RuntimeScriptValue Sc_Label_GetText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(GUILabel, Label_GetText, char);
}

// void (GUILabel *labl, const char *newtx)
RuntimeScriptValue Sc_Label_SetText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(GUILabel, Label_SetText, const char);
}

RuntimeScriptValue Sc_Label_GetTextAlignment(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUILabel, Label_GetTextAlignment);
}

RuntimeScriptValue Sc_Label_SetTextAlignment(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUILabel, Label_SetTextAlignment);
}


// int (GUILabel *labl)
RuntimeScriptValue Sc_Label_GetFont(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUILabel, Label_GetFont);
}

// void (GUILabel *guil, int fontnum)
RuntimeScriptValue Sc_Label_SetFont(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUILabel, Label_SetFont);
}

// const char* (GUILabel *labl)
RuntimeScriptValue Sc_Label_GetText_New(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(GUILabel, const char, myScriptStringImpl, Label_GetText_New);
}

// int (GUILabel *labl)
RuntimeScriptValue Sc_Label_GetColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUILabel, Label_GetColor);
}

// void (GUILabel *labl, int colr)
RuntimeScriptValue Sc_Label_SetColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUILabel, Label_SetColor);
}



void RegisterLabelAPI() {
	ccAddExternalObjectFunction("Label::GetText^1", Sc_Label_GetText);
	ccAddExternalObjectFunction("Label::SetText^1", Sc_Label_SetText);
	ccAddExternalObjectFunction("Label::get_TextAlignment", Sc_Label_GetTextAlignment);
	ccAddExternalObjectFunction("Label::set_TextAlignment", Sc_Label_SetTextAlignment);
	ccAddExternalObjectFunction("Label::get_Font", Sc_Label_GetFont);
	ccAddExternalObjectFunction("Label::set_Font", Sc_Label_SetFont);
	ccAddExternalObjectFunction("Label::get_Text", Sc_Label_GetText_New);
	ccAddExternalObjectFunction("Label::set_Text", Sc_Label_SetText);
	ccAddExternalObjectFunction("Label::get_TextColor", Sc_Label_GetColor);
	ccAddExternalObjectFunction("Label::set_TextColor", Sc_Label_SetColor);

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("Label::GetText^1", (void *)Label_GetText);
	ccAddExternalFunctionForPlugin("Label::SetText^1", (void *)Label_SetText);
	ccAddExternalFunctionForPlugin("Label::get_Font", (void *)Label_GetFont);
	ccAddExternalFunctionForPlugin("Label::set_Font", (void *)Label_SetFont);
	ccAddExternalFunctionForPlugin("Label::get_Text", (void *)Label_GetText_New);
	ccAddExternalFunctionForPlugin("Label::set_Text", (void *)Label_SetText);
	ccAddExternalFunctionForPlugin("Label::get_TextColor", (void *)Label_GetColor);
	ccAddExternalFunctionForPlugin("Label::set_TextColor", (void *)Label_SetColor);
}

} // namespace AGS3
