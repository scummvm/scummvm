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

#include "ags/engine/ac/textbox.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/globals.h"

namespace AGS3 {

// ** TEXT BOX FUNCTIONS

const char *TextBox_GetText_New(GUITextBox *texbox) {
	return CreateNewScriptString(texbox->Text.GetCStr());
}

void TextBox_GetText(GUITextBox *texbox, char *buffer) {
	snprintf(buffer, MAX_MAXSTRLEN, "%s", texbox->Text.GetCStr());
}

void TextBox_SetText(GUITextBox *texbox, const char *newtex) {
	if (texbox->Text != newtex) {
		texbox->Text = newtex;
		texbox->MarkChanged();
	}
}

int TextBox_GetTextColor(GUITextBox *guit) {
	return guit->TextColor;
}

void TextBox_SetTextColor(GUITextBox *guit, int colr) {
	if (guit->TextColor != colr) {
		guit->TextColor = colr;
		guit->MarkChanged();
	}
}

int TextBox_GetFont(GUITextBox *guit) {
	return guit->Font;
}

void TextBox_SetFont(GUITextBox *guit, int fontnum) {
	if ((fontnum < 0) || (fontnum >= _GP(game).numfonts))
		quit("!SetTextBoxFont: invalid font number.");

	if (guit->Font != fontnum) {
		guit->Font = fontnum;
		guit->MarkChanged();
	}
}

bool TextBox_GetShowBorder(GUITextBox *guit) {
	return guit->IsBorderShown();
}

void TextBox_SetShowBorder(GUITextBox *guit, bool on) {
	if (guit->IsBorderShown() != on) {
		guit->SetShowBorder(on);
		guit->MarkChanged();
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// void (GUITextBox *texbox, char *buffer)
RuntimeScriptValue Sc_TextBox_GetText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(GUITextBox, TextBox_GetText, char);
}

// void (GUITextBox *texbox, const char *newtex)
RuntimeScriptValue Sc_TextBox_SetText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(GUITextBox, TextBox_SetText, const char);
}

// int (GUITextBox *guit)
RuntimeScriptValue Sc_TextBox_GetFont(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUITextBox, TextBox_GetFont);
}

// void (GUITextBox *guit, int fontnum)
RuntimeScriptValue Sc_TextBox_SetFont(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUITextBox, TextBox_SetFont);
}

RuntimeScriptValue Sc_TextBox_GetShowBorder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(GUITextBox, TextBox_GetShowBorder);
}

// void (GUITextBox *guit, int fontnum)
RuntimeScriptValue Sc_TextBox_SetShowBorder(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PBOOL(GUITextBox, TextBox_SetShowBorder);
}

// const char* (GUITextBox *texbox)
RuntimeScriptValue Sc_TextBox_GetText_New(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(GUITextBox, const char, _GP(myScriptStringImpl), TextBox_GetText_New);
}

// int (GUITextBox *guit)
RuntimeScriptValue Sc_TextBox_GetTextColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUITextBox, TextBox_GetTextColor);
}

// void (GUITextBox *guit, int colr)
RuntimeScriptValue Sc_TextBox_SetTextColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUITextBox, TextBox_SetTextColor);
}


void RegisterTextBoxAPI() {
	ScFnRegister textbox_api[] = {
		{"TextBox::GetText^1", API_FN_PAIR(TextBox_GetText)},
		{"TextBox::SetText^1", API_FN_PAIR(TextBox_SetText)},
		{"TextBox::get_Font", API_FN_PAIR(TextBox_GetFont)},
		{"TextBox::set_Font", API_FN_PAIR(TextBox_SetFont)},
		{"TextBox::get_ShowBorder", API_FN_PAIR(TextBox_GetShowBorder)},
		{"TextBox::set_ShowBorder", API_FN_PAIR(TextBox_SetShowBorder)},
		{"TextBox::get_Text", API_FN_PAIR(TextBox_GetText_New)},
		{"TextBox::set_Text", API_FN_PAIR(TextBox_SetText)},
		{"TextBox::get_TextColor", API_FN_PAIR(TextBox_GetTextColor)},
		{"TextBox::set_TextColor", API_FN_PAIR(TextBox_SetTextColor)},
	};

	ccAddExternalFunctions361(textbox_api);
}

} // namespace AGS3
