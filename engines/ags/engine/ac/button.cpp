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

#include "ags/engine/ac/button.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/gui.h"
#include "ags/shared/ac/view.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/view_frame.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/gui/animating_gui_button.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

// *** BUTTON FUNCTIONS

void Button_Animate(GUIButton *butt, int view, int loop, int speed, int repeat) {
	int guin = butt->ParentId;
	int objn = butt->Id;

	if ((view < 1) || (view > _GP(game).numviews))
		quit("!AnimateButton: invalid view specified");
	view--;

	if ((loop < 0) || (loop >= _GP(views)[view].numLoops))
		quit("!AnimateButton: invalid loop specified for view");

	// if it's already animating, stop it
	FindAndRemoveButtonAnimation(guin, objn);

	int buttonId = _GP(guis)[guin].GetControlID(objn);

	_GP(guibuts)[buttonId].PushedImage = 0;
	_GP(guibuts)[buttonId].MouseOverImage = 0;

	AnimatingGUIButton abtn;
	abtn.ongui = guin;
	abtn.onguibut = objn;
	abtn.buttonid = buttonId;
	abtn.view = view;
	abtn.loop = loop;
	abtn.speed = speed;
	abtn.repeat = repeat;
	abtn.frame = -1;
	abtn.wait = 0;
	_GP(animbuts).push_back(abtn);
	// launch into the first frame
	if (UpdateAnimatingButton(_GP(animbuts).size() - 1)) {
		debug_script_warn("AnimateButton: no frames to animate (button: %s, view: %d, loop: %d)",
			butt->GetScriptName().GetCStr(), view, loop);
		StopButtonAnimation(_GP(animbuts).size() - 1);
	}
}

const char *Button_GetText_New(GUIButton *butt) {
	return CreateNewScriptString(butt->GetText().GetCStr());
}

void Button_GetText(GUIButton *butt, char *buffer) {
	strcpy(buffer, butt->GetText().GetCStr());
}

void Button_SetText(GUIButton *butt, const char *newtx) {
	newtx = get_translation(newtx);

	if (butt->GetText() != newtx) {
		butt->SetText(newtx);
	}
}

void Button_SetFont(GUIButton *butt, int newFont) {
	if ((newFont < 0) || (newFont >= _GP(game).numfonts))
		quit("!Button.Font: invalid font number.");

	if (butt->Font != newFont) {
		butt->Font = newFont;
		butt->NotifyParentChanged();
	}
}

int Button_GetFont(GUIButton *butt) {
	return butt->Font;
}

int Button_GetClipImage(GUIButton *butt) {
	return butt->IsClippingImage() ? 1 : 0;
}

void Button_SetClipImage(GUIButton *butt, int newval) {
	if (butt->IsClippingImage() != (newval != 0)) {
		butt->SetClipImage(newval != 0);
	}
}

int Button_GetGraphic(GUIButton *butt) {
	// return currently displayed pic
	if (butt->CurrentImage < 0)
		return butt->Image;
	return butt->CurrentImage;
}

int Button_GetMouseOverGraphic(GUIButton *butt) {
	return butt->MouseOverImage;
}

void Button_SetMouseOverGraphic(GUIButton *guil, int slotn) {
	debug_script_log("GUI %d Button %d mouseover set to slot %d", guil->ParentId, guil->Id, slotn);

	if ((guil->IsMouseOver != 0) && (guil->IsPushed == 0))
		guil->CurrentImage = slotn;
	guil->MouseOverImage = slotn;

	guil->NotifyParentChanged();
	FindAndRemoveButtonAnimation(guil->ParentId, guil->Id);
}

int Button_GetNormalGraphic(GUIButton *butt) {
	return butt->Image;
}

void Button_SetNormalGraphic(GUIButton *guil, int slotn) {
	debug_script_log("GUI %d Button %d normal set to slot %d", guil->ParentId, guil->Id, slotn);
	// normal pic - update if mouse is not over, or if there's no MouseOverImage
	if (((guil->IsMouseOver == 0) || (guil->MouseOverImage < 1)) && (guil->IsPushed == 0))
		guil->CurrentImage = slotn;
	guil->Image = slotn;
	// update the clickable area to the same size as the graphic
	if (slotn < 0 || (size_t)slotn >= _GP(game).SpriteInfos.size()) {
		guil->Width = 0;
		guil->Height = 0;
	} else {
		guil->Width = _GP(game).SpriteInfos[slotn].Width;
		guil->Height = _GP(game).SpriteInfos[slotn].Height;
	}

	guil->NotifyParentChanged();
	FindAndRemoveButtonAnimation(guil->ParentId, guil->Id);
}

int Button_GetPushedGraphic(GUIButton *butt) {
	return butt->PushedImage;
}

void Button_SetPushedGraphic(GUIButton *guil, int slotn) {
	debug_script_log("GUI %d Button %d pushed set to slot %d", guil->ParentId, guil->Id, slotn);

	if (guil->IsPushed)
		guil->CurrentImage = slotn;
	guil->PushedImage = slotn;

	guil->NotifyParentChanged();
	FindAndRemoveButtonAnimation(guil->ParentId, guil->Id);
}

int Button_GetTextColor(GUIButton *butt) {
	return butt->TextColor;
}

void Button_SetTextColor(GUIButton *butt, int newcol) {
	if (butt->TextColor != newcol) {
		butt->TextColor = newcol;
		butt->NotifyParentChanged();
	}
}

// ** start animating buttons code

size_t GetAnimatingButtonCount() {
	return _GP(animbuts).size();
}

AnimatingGUIButton *GetAnimatingButtonByIndex(int idxn) {
	return idxn >= 0 && (size_t)idxn < _GP(animbuts).size() ?
		&_GP(animbuts)[idxn] : nullptr;
}

void AddButtonAnimation(const AnimatingGUIButton &abtn) {
	_GP(animbuts).push_back(abtn);
}

// returns 1 if animation finished
int UpdateAnimatingButton(int bu) {
	AnimatingGUIButton &abtn = _GP(animbuts)[bu];

	if (abtn.wait > 0) {
		abtn.wait--;
		return 0;
	}
	ViewStruct *tview = &_GP(views)[abtn.view];

	abtn.frame++;

	if (abtn.frame >= tview->loops[abtn.loop].numFrames) {
		if (tview->loops[abtn.loop].RunNextLoop()) {
			// go to next loop
			abtn.loop++;
			abtn.frame = 0;
		} else if (abtn.repeat) {
			abtn.frame = 0;
			// multi-loop anim, go back
			while ((abtn.loop > 0) &&
				(tview->loops[abtn.loop - 1].RunNextLoop()))
				abtn.loop--;
		} else
			return 1;
	}

	CheckViewFrame(abtn.view, abtn.loop, abtn.frame);

	// update the button's image
	_GP(guibuts)[abtn.buttonid].Image = tview->loops[abtn.loop].frames[abtn.frame].pic;
	_GP(guibuts)[abtn.buttonid].CurrentImage = _GP(guibuts)[abtn.buttonid].Image;
	_GP(guibuts)[abtn.buttonid].PushedImage = 0;
	_GP(guibuts)[abtn.buttonid].MouseOverImage = 0;
	_GP(guibuts)[abtn.buttonid].NotifyParentChanged();

	abtn.wait = abtn.speed + tview->loops[abtn.loop].frames[abtn.frame].speed;
	return 0;
}

void StopButtonAnimation(int idxn) {
	_GP(animbuts).erase(_GP(animbuts).begin() + idxn);
}

void RemoveAllButtonAnimations() {
	_GP(animbuts).clear();
}

// Returns the index of the AnimatingGUIButton object corresponding to the
// given button ID; returns -1 if no such animation exists
int FindAnimatedButton(int guin, int objn) {
	for (size_t i = 0; i < _GP(animbuts).size(); ++i) {
		if (_GP(animbuts)[i].ongui == guin && _GP(animbuts)[i].onguibut == objn)
			return i;
	}
	return -1;
}

void FindAndRemoveButtonAnimation(int guin, int objn) {
	int idx = FindAnimatedButton(guin, objn);
	if (idx >= 0)
		StopButtonAnimation(idx);
}

// ** end animating buttons code

void Button_Click(GUIButton *butt, int mbut) {
	process_interface_click(butt->ParentId, butt->Id, mbut);
}

bool Button_IsAnimating(GUIButton *butt) {
	return FindAnimatedButton(butt->ParentId, butt->Id) >= 0;
}

// NOTE: in correspondance to similar functions for Character & Object,
// GetView returns (view index + 1), while GetLoop and GetFrame return
// zero-based index and 0 in case of no animation.
int Button_GetAnimView(GUIButton *butt) {
	int idx = FindAnimatedButton(butt->ParentId, butt->Id);
	return idx >= 0 ? _GP(animbuts)[idx].view + 1 : 0;
}

int Button_GetAnimLoop(GUIButton *butt) {
	int idx = FindAnimatedButton(butt->ParentId, butt->Id);
	return idx >= 0 ? _GP(animbuts)[idx].loop : 0;
}

int Button_GetAnimFrame(GUIButton *butt) {
	int idx = FindAnimatedButton(butt->ParentId, butt->Id);
	return idx >= 0 ? _GP(animbuts)[idx].frame : 0;
}

int Button_GetTextAlignment(GUIButton *butt) {
	return butt->TextAlignment;
}

void Button_SetTextAlignment(GUIButton *butt, int align) {
	if (butt->TextAlignment != align) {
		butt->TextAlignment = (FrameAlignment)align;
		butt->NotifyParentChanged();
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// void | GUIButton *butt, int view, int loop, int speed, int repeat
RuntimeScriptValue Sc_Button_Animate(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT4(GUIButton, Button_Animate);
}

// const char* | GUIButton *butt
RuntimeScriptValue Sc_Button_GetText_New(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ(GUIButton, const char, _GP(myScriptStringImpl), Button_GetText_New);
}

// void | GUIButton *butt, char *buffer
RuntimeScriptValue Sc_Button_GetText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(GUIButton, Button_GetText, char);
}

// void | GUIButton *butt, const char *newtx
RuntimeScriptValue Sc_Button_SetText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(GUIButton, Button_SetText, const char);
}

// void | GUIButton *butt, int newFont
RuntimeScriptValue Sc_Button_SetFont(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIButton, Button_SetFont);
}

// int | GUIButton *butt
RuntimeScriptValue Sc_Button_GetFont(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIButton, Button_GetFont);
}

// int | GUIButton *butt
RuntimeScriptValue Sc_Button_GetClipImage(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIButton, Button_GetClipImage);
}

// void | GUIButton *butt, int newval
RuntimeScriptValue Sc_Button_SetClipImage(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIButton, Button_SetClipImage);
}

// int | GUIButton *butt
RuntimeScriptValue Sc_Button_GetGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIButton, Button_GetGraphic);
}

// int | GUIButton *butt
RuntimeScriptValue Sc_Button_GetMouseOverGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIButton, Button_GetMouseOverGraphic);
}

// void | GUIButton *guil, int slotn
RuntimeScriptValue Sc_Button_SetMouseOverGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIButton, Button_SetMouseOverGraphic);
}

// int | GUIButton *butt
RuntimeScriptValue Sc_Button_GetNormalGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIButton, Button_GetNormalGraphic);
}

// void | GUIButton *guil, int slotn
RuntimeScriptValue Sc_Button_SetNormalGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIButton, Button_SetNormalGraphic);
}

// int | GUIButton *butt
RuntimeScriptValue Sc_Button_GetPushedGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIButton, Button_GetPushedGraphic);
}

// void | GUIButton *guil, int slotn
RuntimeScriptValue Sc_Button_SetPushedGraphic(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIButton, Button_SetPushedGraphic);
}

// int | GUIButton *butt
RuntimeScriptValue Sc_Button_GetTextColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIButton, Button_GetTextColor);
}

// void | GUIButton *butt, int newcol
RuntimeScriptValue Sc_Button_SetTextColor(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIButton, Button_SetTextColor);
}

RuntimeScriptValue Sc_Button_Click(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIButton, Button_Click);
}

RuntimeScriptValue Sc_Button_GetAnimating(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(GUIButton, Button_IsAnimating);
}

RuntimeScriptValue Sc_Button_GetTextAlignment(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIButton, Button_GetTextAlignment);
}

RuntimeScriptValue Sc_Button_SetTextAlignment(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIButton, Button_SetTextAlignment);
}

RuntimeScriptValue Sc_Button_GetFrame(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIButton, Button_GetAnimFrame);
}

RuntimeScriptValue Sc_Button_GetLoop(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIButton, Button_GetAnimLoop);
}

RuntimeScriptValue Sc_Button_GetView(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIButton, Button_GetAnimView);
}

void RegisterButtonAPI() {
	ccAddExternalObjectFunction("Button::Animate^4", Sc_Button_Animate);
	ccAddExternalObjectFunction("Button::Click^1", Sc_Button_Click);
	ccAddExternalObjectFunction("Button::GetText^1", Sc_Button_GetText);
	ccAddExternalObjectFunction("Button::SetText^1", Sc_Button_SetText);
	ccAddExternalObjectFunction("Button::get_TextAlignment", Sc_Button_GetTextAlignment);
	ccAddExternalObjectFunction("Button::set_TextAlignment", Sc_Button_SetTextAlignment);
	ccAddExternalObjectFunction("Button::get_Animating", Sc_Button_GetAnimating);
	ccAddExternalObjectFunction("Button::get_ClipImage", Sc_Button_GetClipImage);
	ccAddExternalObjectFunction("Button::set_ClipImage", Sc_Button_SetClipImage);
	ccAddExternalObjectFunction("Button::get_Font", Sc_Button_GetFont);
	ccAddExternalObjectFunction("Button::set_Font", Sc_Button_SetFont);
	ccAddExternalObjectFunction("Button::get_Frame", Sc_Button_GetFrame);
	ccAddExternalObjectFunction("Button::get_Graphic", Sc_Button_GetGraphic);
	ccAddExternalObjectFunction("Button::get_Loop", Sc_Button_GetLoop);
	ccAddExternalObjectFunction("Button::get_MouseOverGraphic", Sc_Button_GetMouseOverGraphic);
	ccAddExternalObjectFunction("Button::set_MouseOverGraphic", Sc_Button_SetMouseOverGraphic);
	ccAddExternalObjectFunction("Button::get_NormalGraphic", Sc_Button_GetNormalGraphic);
	ccAddExternalObjectFunction("Button::set_NormalGraphic", Sc_Button_SetNormalGraphic);
	ccAddExternalObjectFunction("Button::get_PushedGraphic", Sc_Button_GetPushedGraphic);
	ccAddExternalObjectFunction("Button::set_PushedGraphic", Sc_Button_SetPushedGraphic);
	ccAddExternalObjectFunction("Button::get_Text", Sc_Button_GetText_New);
	ccAddExternalObjectFunction("Button::set_Text", Sc_Button_SetText);
	ccAddExternalObjectFunction("Button::get_TextColor", Sc_Button_GetTextColor);
	ccAddExternalObjectFunction("Button::set_TextColor", Sc_Button_SetTextColor);
	ccAddExternalObjectFunction("Button::get_View", Sc_Button_GetView);
}

} // namespace AGS3
