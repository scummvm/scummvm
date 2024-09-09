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

#include "ags/engine/ac/inv_window.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character_extras.h"
#include "ags/shared/ac/character_info.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game_state.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_room.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/gui/gui_dialog.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/engine/ac/dynobj/cc_character.h"
#include "ags/engine/ac/dynobj/cc_inventory.h"
#include "ags/shared/util/math.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/ac/timer.h"
#include "ags/shared/util/wgt2_allg.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

// *** INV WINDOW FUNCTIONS

void InvWindow_SetCharacterToUse(GUIInvWindow *guii, CharacterInfo *chaa) {
	if (chaa == nullptr)
		guii->CharId = -1;
	else
		guii->CharId = chaa->index_id;
	// reset to top of list
	guii->TopItem = 0;

	guii->MarkChanged();
}

CharacterInfo *InvWindow_GetCharacterToUse(GUIInvWindow *guii) {
	if (guii->CharId < 0)
		return nullptr;

	return &_GP(game).chars[guii->CharId];
}

void InvWindow_SetItemWidth(GUIInvWindow *guii, int newwidth) {
	guii->ItemWidth = newwidth;
	guii->OnResized();
}

int InvWindow_GetItemWidth(GUIInvWindow *guii) {
	return guii->ItemWidth;
}

void InvWindow_SetItemHeight(GUIInvWindow *guii, int newhit) {
	guii->ItemHeight = newhit;
	guii->OnResized();
}

int InvWindow_GetItemHeight(GUIInvWindow *guii) {
	return guii->ItemHeight;
}

void InvWindow_SetTopItem(GUIInvWindow *guii, int topitem) {
	if (guii->TopItem != topitem) {
		guii->TopItem = topitem;
		guii->MarkChanged();
	}
}

int InvWindow_GetTopItem(GUIInvWindow *guii) {
	return guii->TopItem;
}

int InvWindow_GetItemsPerRow(GUIInvWindow *guii) {
	return guii->ColCount;
}

int InvWindow_GetItemCount(GUIInvWindow *guii) {
	return _GP(charextra)[guii->GetCharacterId()].invorder_count;
}

int InvWindow_GetRowCount(GUIInvWindow *guii) {
	return guii->RowCount;
}

void InvWindow_ScrollDown(GUIInvWindow *guii) {
	if ((_GP(charextra)[guii->GetCharacterId()].invorder_count) >
	        (guii->TopItem + (guii->ColCount * guii->RowCount))) {
		guii->TopItem += guii->ColCount;
		guii->MarkChanged();
	}
}

void InvWindow_ScrollUp(GUIInvWindow *guii) {
	if (guii->TopItem > 0) {
		guii->TopItem -= guii->ColCount;
		if (guii->TopItem < 0)
			guii->TopItem = 0;

		guii->MarkChanged();
	}
}

ScriptInvItem *InvWindow_GetItemAtIndex(GUIInvWindow *guii, int index) {
	if ((index < 0) || (index >= _GP(charextra)[guii->GetCharacterId()].invorder_count))
		return nullptr;
	return &_G(scrInv)[_GP(charextra)[guii->GetCharacterId()].invorder[index]];
}

//=============================================================================

int offset_over_inv(GUIInvWindow *inv) {
	if (inv->ItemWidth <= 0 || inv->ItemHeight <= 0)
		return -1;
	int mover = _G(mouse_ifacebut_xoffs) / data_to_game_coord(inv->ItemWidth);
	// if it's off the edge of the visible items, ignore
	if (mover >= inv->ColCount)
		return -1;
	mover += (_G(mouse_ifacebut_yoffs) / data_to_game_coord(inv->ItemHeight)) * inv->ColCount;
	if (mover >= inv->ColCount * inv->RowCount)
		return -1;

	mover += inv->TopItem;
	if ((mover < 0) || (mover >= _GP(charextra)[inv->GetCharacterId()].invorder_count))
		return -1;

	return _GP(charextra)[inv->GetCharacterId()].invorder[mover];
}

//
// NOTE: This is an old default inventory screen implementation,
// which became completely obsolete after AGS 2.72.
//

#define ICONSPERLINE 4

struct DisplayInvItem {
	int num;
	int sprnum;
};

struct InventoryScreen {
	static const int ARROWBUTTONWID = 11;

	int BUTTONAREAHEIGHT;
	int cmode;
	int toret;
	int top_item;
	int num_visible_items;
	int MAX_ITEMAREA_HEIGHT;
	int wasonitem;
	int bartop;
	int barxp;
	int numitems;
	int widest;
	int highest;
	int windowwid;
	int windowhit;
	int windowxp;
	int windowyp;
	int buttonyp;
	DisplayInvItem dii[MAX_INV];
	int btn_look_sprite;
	int btn_select_sprite;
	int btn_ok_sprite;

	int break_code;

	void Prepare();
	int  Redraw();
	void Draw(Bitmap *ds);
	void RedrawOverItem(Bitmap *ds, int isonitem);
	bool Run();
	void Close();
};

InventoryScreen InvScr;

void InventoryScreen::Prepare() {
	BUTTONAREAHEIGHT = get_fixed_pixel_size(30);
	cmode = CURS_ARROW;
	toret = -1;
	top_item = 0;
	num_visible_items = 0;
	MAX_ITEMAREA_HEIGHT = ((_GP(play).GetUIViewport().GetHeight() - BUTTONAREAHEIGHT) - get_fixed_pixel_size(20));
	_G(in_inv_screen)++;
	_G(inv_screen_newroom) = -1;

	// Sprites 2041, 2042 and 2043 were hardcoded in the older versions of
	// the engine to be used in the built-in inventory window.
	// If they did not exist engine first fell back to sprites 0, 1, 2 instead.
	// Fun fact: this fallback does not seem to be intentional, and was a
	// coincidental result of SpriteCache incorrectly remembering "last seek'd
	// sprite" as 2041/2042/2043 while in fact stream was after sprite 0.
	if (!_GP(spriteset).DoesSpriteExist(2041) || !_GP(spriteset).DoesSpriteExist(2042) || !_GP(spriteset).DoesSpriteExist(2043)) {
		debug_script_warn("InventoryScreen: one or more of the inventory screen graphics (sprites 2041, 2042, 2043) does not exist, fallback to sprites 0, 1, 2 instead");
	}
	btn_look_sprite = _GP(spriteset).DoesSpriteExist(2041) ? 2041 : 0;
	btn_select_sprite = _GP(spriteset).DoesSpriteExist(2042) ? 2042 : (_GP(spriteset).DoesSpriteExist(1) ? 1 : 0);
	btn_ok_sprite = _GP(spriteset).DoesSpriteExist(2043) ? 2043 : (_GP(spriteset).DoesSpriteExist(2) ? 2 : 0);

	break_code = 0;
}

int InventoryScreen::Redraw() {
	numitems = 0;
	widest = 0;
	highest = 0;
	if (_GP(charextra)[_GP(game).playercharacter].invorder_count < 0)
		update_invorder();
	if (_GP(charextra)[_GP(game).playercharacter].invorder_count == 0) {
		DisplayMessage(996);
		_G(in_inv_screen)--;
		return -1;
	}

	if (_G(inv_screen_newroom) >= 0) {
		_G(in_inv_screen)--;
		NewRoom(_G(inv_screen_newroom));
		return -1;
	}

	for (int i = 0; i < _GP(charextra)[_GP(game).playercharacter].invorder_count; ++i) {
		if (_GP(game).invinfo[_GP(charextra)[_GP(game).playercharacter].invorder[i]].name[0] != 0) {
			dii[numitems].num = _GP(charextra)[_GP(game).playercharacter].invorder[i];
			dii[numitems].sprnum = _GP(game).invinfo[_GP(charextra)[_GP(game).playercharacter].invorder[i]].pic;
			int snn = dii[numitems].sprnum;
			if (_GP(game).SpriteInfos[snn].Width > widest) widest = _GP(game).SpriteInfos[snn].Width;
			if (_GP(game).SpriteInfos[snn].Height > highest) highest = _GP(game).SpriteInfos[snn].Height;
			numitems++;
		}
	}
	if (numitems != _GP(charextra)[_GP(game).playercharacter].invorder_count)
		quit("inconsistent inventory calculations");

	widest += get_fixed_pixel_size(4);
	highest += get_fixed_pixel_size(4);
	num_visible_items = (MAX_ITEMAREA_HEIGHT / highest) * ICONSPERLINE;

	windowhit = highest * (numitems / ICONSPERLINE) + get_fixed_pixel_size(4);
	if ((numitems % ICONSPERLINE) != 0) windowhit += highest;
	if (windowhit > MAX_ITEMAREA_HEIGHT) {
		windowhit = (MAX_ITEMAREA_HEIGHT / highest) * highest + get_fixed_pixel_size(4);
	}
	windowhit += BUTTONAREAHEIGHT;

	windowwid = widest * ICONSPERLINE + get_fixed_pixel_size(4);
	if (windowwid < get_fixed_pixel_size(105)) windowwid = get_fixed_pixel_size(105);
	windowxp = _GP(play).GetUIViewport().GetWidth() / 2 - windowwid / 2;
	windowyp = _GP(play).GetUIViewport().GetHeight() / 2 - windowhit / 2;
	buttonyp = windowhit - BUTTONAREAHEIGHT;
	bartop = get_fixed_pixel_size(2);
	barxp = get_fixed_pixel_size(2);

	Bitmap *ds = prepare_gui_screen(windowxp, windowyp, windowwid, windowhit, true);
	Draw(ds);
	set_mouse_cursor(cmode);
	wasonitem = -1;
	return 0;
}

void InventoryScreen::Draw(Bitmap *ds) {
	color_t draw_color = ds->GetCompatibleColor(_GP(play).sierra_inv_color);
	ds->FillRect(Rect(0, 0, windowwid, windowhit), draw_color);
	draw_color = ds->GetCompatibleColor(0);
	ds->FillRect(Rect(barxp, bartop, windowwid - get_fixed_pixel_size(2), buttonyp - 1), draw_color);
	for (int i = top_item; i < numitems; ++i) {
		if (i >= top_item + num_visible_items)
			break;
		Bitmap *spof = _GP(spriteset)[dii[i].sprnum];
		wputblock(ds, barxp + 1 + ((i - top_item) % 4) * widest + widest / 2 - spof->GetWidth() / 2,
		          bartop + 1 + ((i - top_item) / 4) * highest + highest / 2 - spof->GetHeight() / 2, spof, 1);
	}
#define BUTTONWID MAX(1, _GP(game).SpriteInfos[btn_select_sprite].Width)
	// Draw select, look and OK buttons
	wputblock(ds, 2, buttonyp + get_fixed_pixel_size(2), _GP(spriteset)[btn_look_sprite], 1);
	wputblock(ds, 3 + BUTTONWID, buttonyp + get_fixed_pixel_size(2), _GP(spriteset)[btn_select_sprite], 1);
	wputblock(ds, 4 + BUTTONWID * 2, buttonyp + get_fixed_pixel_size(2), _GP(spriteset)[btn_ok_sprite], 1);

	// Draw Up and Down buttons if required
	Bitmap *arrowblock = BitmapHelper::CreateTransparentBitmap(ARROWBUTTONWID, ARROWBUTTONWID);
	draw_color = arrowblock->GetCompatibleColor(0);
	if (_GP(play).sierra_inv_color == 0)
		draw_color = ds->GetCompatibleColor(14);

	arrowblock->DrawLine(Line(ARROWBUTTONWID / 2, 2, ARROWBUTTONWID - 2, 9), draw_color);
	arrowblock->DrawLine(Line(ARROWBUTTONWID / 2, 2, 2, 9), draw_color);
	arrowblock->DrawLine(Line(2, 9, ARROWBUTTONWID - 2, 9), draw_color);
	arrowblock->FloodFill(ARROWBUTTONWID / 2, 4, draw_color);

	if (top_item > 0)
		wputblock(ds, windowwid - ARROWBUTTONWID, buttonyp + get_fixed_pixel_size(2), arrowblock, 1);
	if (top_item + num_visible_items < numitems)
		arrowblock->FlipBlt(arrowblock, windowwid - ARROWBUTTONWID, buttonyp + get_fixed_pixel_size(4) + ARROWBUTTONWID, Shared::kFlip_Vertical);
	delete arrowblock;
}

void InventoryScreen::RedrawOverItem(Bitmap *ds, int isonitem) {
	int rectxp = barxp + 1 + (wasonitem % 4) * widest;
	int rectyp = bartop + 1 + ((wasonitem - top_item) / 4) * highest;
	if (wasonitem >= 0) {
		color_t draw_color = ds->GetCompatibleColor(0);
		ds->DrawRect(Rect(rectxp, rectyp, rectxp + widest - 1, rectyp + highest - 1), draw_color);
	}
	if (isonitem >= 0) {
		color_t draw_color = ds->GetCompatibleColor(14);//opts.invrectcol);
		rectxp = barxp + 1 + (isonitem % 4) * widest;
		rectyp = bartop + 1 + ((isonitem - top_item) / 4) * highest;
		ds->DrawRect(Rect(rectxp, rectyp, rectxp + widest - 1, rectyp + highest - 1), draw_color);
	}
}

bool InventoryScreen::Run() {
	// Run() can be called in a loop, so keep events going.
	sys_evt_process_pending();

	// Handle all the buffered key events
	bool do_break = false;
	while (ags_keyevent_ready()) {
		KeyInput ki;
		if (run_service_key_controls(ki) && !_GP(play).IsIgnoringInput() && !IsAGSServiceKey(ki.Key)) {
			ags_clear_input_buffer();
			do_break = true; // end inventory screen loop
		}
	}
	if (do_break)
		return false;

	update_audio_system_on_game_loop();
	refresh_gui_screen();

	// NOTE: this is because old code was working with full game screen
	const int mx = _G(mousex) - windowxp;
	const int my = _G(mousey) - windowyp;

	int isonitem = ((my - bartop) / highest) * ICONSPERLINE + (mx - barxp) / widest;
	if (my <= bartop) isonitem = -1;
	else if (isonitem >= 0) isonitem += top_item;
	if ((isonitem < 0) | (isonitem >= numitems) | (isonitem >= top_item + num_visible_items))
		isonitem = -1;

	eAGSMouseButton mbut;
	int mwheelz;
	if (!run_service_mb_controls(mbut, mwheelz) || _GP(play).IsIgnoringInput()) {
		mbut = kMouseNone;
	}

	if (mbut == kMouseLeft) {
		if ((my < 0) | (my > windowhit) | (mx < 0) | (mx > windowwid))
			return true; // continue inventory screen loop
		if (my < buttonyp) {
			int clickedon = isonitem;
			if (clickedon < 0) return true; // continue inventory screen loop
			_G(evblocknum) = dii[clickedon].num;
			_GP(play).used_inv_on = dii[clickedon].num;

			if (cmode == MODE_LOOK) {
				run_event_block_inv(dii[clickedon].num, 0);
				// in case the script did anything to the screen, redraw it
				UpdateGameOnce();

				break_code = Redraw();
				return break_code == 0;
			} else if (cmode == MODE_USE) {
				// use objects on each other
				_GP(play).usedinv = toret;

				// set the activeinv so the script can check it
				int activeinvwas = _G(playerchar)->activeinv;
				_G(playerchar)->activeinv = toret;

				run_event_block_inv(dii[clickedon].num, 3);

				// if the script didn't change it, then put it back
				if (_G(playerchar)->activeinv == toret)
					_G(playerchar)->activeinv = activeinvwas;

				// in case the script did anything to the screen, redraw it
				UpdateGameOnce();

				// They used the active item and lost it
				if (_G(playerchar)->inv[toret] < 1) {
					cmode = CURS_ARROW;
					set_mouse_cursor(cmode);
					toret = -1;
				}

				break_code = Redraw();
				return break_code == 0;
			}
			toret = dii[clickedon].num;
			//        int plusng=_GP(play).using; _GP(play).using=toret;
			update_inv_cursor(toret);
			set_mouse_cursor(MODE_USE);
			cmode = MODE_USE;
			//        _GP(play).using=plusng;
			//        break;
			return true; // continue inventory screen loop
		} else {
			if (mx >= windowwid - ARROWBUTTONWID) {
				if (my < buttonyp + get_fixed_pixel_size(2) + ARROWBUTTONWID) {
					if (top_item > 0) {
						top_item -= ICONSPERLINE;

						break_code = Redraw();
						return break_code == 0;
					}
				} else if ((my < buttonyp + get_fixed_pixel_size(4) + ARROWBUTTONWID * 2) && (top_item + num_visible_items < numitems)) {
					top_item += ICONSPERLINE;

					break_code = Redraw();
					return break_code == 0;
				}
				return true; // continue inventory screen loop
			}

			int buton = mx - 2;
			if (buton < 0) return true; // continue inventory screen loop
			buton /= BUTTONWID;
			if (buton >= 3) return true; // continue inventory screen loop
			if (buton == 0) {
				toret = -1; cmode = MODE_LOOK;
			} else if (buton == 1) {
				cmode = CURS_ARROW; toret = -1;
			} else {
				return false; // end inventory screen loop
			}
			set_mouse_cursor(cmode);
		}
	} else if (mbut == kMouseRight) {
		if (cmode == CURS_ARROW)
			cmode = MODE_LOOK;
		else
			cmode = CURS_ARROW;
		toret = -1;
		set_mouse_cursor(cmode);
	} else if (isonitem != wasonitem) {
		RedrawOverItem(get_gui_screen(), isonitem);
	}
	wasonitem = isonitem;

	update_polled_stuff();

	WaitForNextFrame();

	return true; // continue inventory screen loop
}

void InventoryScreen::Close() {
	clear_gui_screen();
	set_default_cursor();
	invalidate_screen();
	_G(in_inv_screen)--;
}

int __actual_invscreen() {
	InvScr.Prepare();
	InvScr.break_code = InvScr.Redraw();
	if (InvScr.break_code != 0) {
		return InvScr.break_code;
	}

	while (InvScr.Run());

	if (InvScr.break_code != 0) {
		return InvScr.break_code;
	}

	ags_clear_input_buffer();

	InvScr.Close();
	return InvScr.toret;
}

int invscreen() {
	int selt = __actual_invscreen();
	if (selt < 0) return -1;
	_G(playerchar)->activeinv = selt;
	GUI::MarkInventoryForUpdate(_G(playerchar)->index_id, true);
	set_cursor_mode(MODE_USE);
	return selt;
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// void (GUIInvWindow *guii)
RuntimeScriptValue Sc_InvWindow_ScrollDown(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(GUIInvWindow, InvWindow_ScrollDown);
}

// void (GUIInvWindow *guii)
RuntimeScriptValue Sc_InvWindow_ScrollUp(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(GUIInvWindow, InvWindow_ScrollUp);
}

// CharacterInfo* (GUIInvWindow *guii)
RuntimeScriptValue Sc_InvWindow_GetCharacterToUse(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(GUIInvWindow, CharacterInfo, _GP(ccDynamicCharacter), InvWindow_GetCharacterToUse);
}

// void (GUIInvWindow *guii, CharacterInfo *chaa)
RuntimeScriptValue Sc_InvWindow_SetCharacterToUse(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(GUIInvWindow, InvWindow_SetCharacterToUse, CharacterInfo);
}

// ScriptInvItem* (GUIInvWindow *guii, int index)
RuntimeScriptValue Sc_InvWindow_GetItemAtIndex(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_PINT(GUIInvWindow, ScriptInvItem, _GP(ccDynamicInv), InvWindow_GetItemAtIndex);
}

// int (GUIInvWindow *guii)
RuntimeScriptValue Sc_InvWindow_GetItemCount(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIInvWindow, InvWindow_GetItemCount);
}

// int (GUIInvWindow *guii)
RuntimeScriptValue Sc_InvWindow_GetItemHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIInvWindow, InvWindow_GetItemHeight);
}

// void (GUIInvWindow *guii, int newhit)
RuntimeScriptValue Sc_InvWindow_SetItemHeight(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIInvWindow, InvWindow_SetItemHeight);
}

// int (GUIInvWindow *guii)
RuntimeScriptValue Sc_InvWindow_GetItemWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIInvWindow, InvWindow_GetItemWidth);
}

// void (GUIInvWindow *guii, int newwidth)
RuntimeScriptValue Sc_InvWindow_SetItemWidth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIInvWindow, InvWindow_SetItemWidth);
}

// int (GUIInvWindow *guii)
RuntimeScriptValue Sc_InvWindow_GetItemsPerRow(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIInvWindow, InvWindow_GetItemsPerRow);
}

// int (GUIInvWindow *guii)
RuntimeScriptValue Sc_InvWindow_GetRowCount(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIInvWindow, InvWindow_GetRowCount);
}

// int (GUIInvWindow *guii)
RuntimeScriptValue Sc_InvWindow_GetTopItem(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(GUIInvWindow, InvWindow_GetTopItem);
}

// void (GUIInvWindow *guii, int topitem)
RuntimeScriptValue Sc_InvWindow_SetTopItem(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(GUIInvWindow, InvWindow_SetTopItem);
}



void RegisterInventoryWindowAPI() {
	ccAddExternalObjectFunction("InvWindow::ScrollDown^0", Sc_InvWindow_ScrollDown);
	ccAddExternalObjectFunction("InvWindow::ScrollUp^0", Sc_InvWindow_ScrollUp);
	ccAddExternalObjectFunction("InvWindow::get_CharacterToUse", Sc_InvWindow_GetCharacterToUse);
	ccAddExternalObjectFunction("InvWindow::set_CharacterToUse", Sc_InvWindow_SetCharacterToUse);
	ccAddExternalObjectFunction("InvWindow::geti_ItemAtIndex", Sc_InvWindow_GetItemAtIndex);
	ccAddExternalObjectFunction("InvWindow::get_ItemCount", Sc_InvWindow_GetItemCount);
	ccAddExternalObjectFunction("InvWindow::get_ItemHeight", Sc_InvWindow_GetItemHeight);
	ccAddExternalObjectFunction("InvWindow::set_ItemHeight", Sc_InvWindow_SetItemHeight);
	ccAddExternalObjectFunction("InvWindow::get_ItemWidth", Sc_InvWindow_GetItemWidth);
	ccAddExternalObjectFunction("InvWindow::set_ItemWidth", Sc_InvWindow_SetItemWidth);
	ccAddExternalObjectFunction("InvWindow::get_ItemsPerRow", Sc_InvWindow_GetItemsPerRow);
	ccAddExternalObjectFunction("InvWindow::get_RowCount", Sc_InvWindow_GetRowCount);
	ccAddExternalObjectFunction("InvWindow::get_TopItem", Sc_InvWindow_GetTopItem);
	ccAddExternalObjectFunction("InvWindow::set_TopItem", Sc_InvWindow_SetTopItem);
}

} // namespace AGS3
