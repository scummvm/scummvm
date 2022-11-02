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

#include "ags/engine/ac/label.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/globals.h"

namespace AGS3 {

// ** LABEL FUNCTIONS

const char *Label_GetText_New(GUILabel *labl) {
	return CreateNewScriptString(labl->GetText().GetCStr());
}

void Label_GetText(GUILabel *labl, char *buffer) {
	snprintf(buffer, MAX_MAXSTRLEN, "%s", labl->GetText().GetCStr());
}

void Label_SetText(GUILabel *labl, const char *newtx) {
	newtx = get_translation(newtx);

	if (labl->GetText() != newtx) {
		labl->SetText(newtx);
	}
}

int Label_GetTextAlignment(GUILabel *labl) {
	return labl->TextAlignment;
}

void Label_SetTextAlignment(GUILabel *labl, int align) {
	if (labl->TextAlignment != align) {
		labl->TextAlignment = (HorAlignment)align;
		labl->MarkChanged();
	}
}

int Label_GetColor(GUILabel *labl) {
	return labl->TextColor;
}

void Label_SetColor(GUILabel *labl, int colr) {
	if (labl->TextColor != colr) {
		labl->TextColor = colr;
		labl->MarkChanged();
	}
}

int Label_GetFont(GUILabel *labl) {
	return labl->Font;
}

void Label_SetFont(GUILabel *guil, int fontnum) {
	if ((fontnum < 0) || (fontnum >= _GP(game).numfonts))
		quit("!SetLabelFont: invalid font number.");

	if (fontnum != guil->Font) {
		guil->Font = fontnum;
		guil->MarkChanged();
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

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
	API_CONST_OBJCALL_OBJ(GUILabel, const char, _GP(myScriptStringImpl), Label_GetText_New);
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
}

} // namespace AGS3
