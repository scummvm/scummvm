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

#include "ags/engine/ac/invwindow.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/characterextras.h"
#include "ags/shared/ac/characterinfo.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_room.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/gui/guidialog.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/engine/script/runtimescriptvalue.h"
#include "ags/engine/ac/dynobj/cc_character.h"
#include "ags/engine/ac/dynobj/cc_inventory.h"
#include "ags/shared/util/math.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/ac/timer.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

extern GameSetupStruct game;
extern GameState play;
extern CharacterExtras *charextra;
extern ScriptInvItem scrInv[MAX_INV];
extern int mouse_ifacebut_xoffs, mouse_ifacebut_yoffs;
extern SpriteCache spriteset;
extern int evblocknum;
extern CharacterInfo *playerchar;
extern AGSPlatformDriver *platform;
extern CCCharacter ccDynamicCharacter;
extern CCInventory ccDynamicInv;

int in_inv_screen = 0, inv_screen_newroom = -1;

// *** INV WINDOW FUNCTIONS

void InvWindow_SetCharacterToUse(GUIInvWindow *guii, CharacterInfo *chaa) {
	if (chaa == nullptr)
		guii->CharId = -1;
	else
		guii->CharId = chaa->index_id;
	// reset to top of list
	guii->TopItem = 0;

	guis_need_update = 1;
}

CharacterInfo *InvWindow_GetCharacterToUse(GUIInvWindow *guii) {
	if (guii->CharId < 0)
		return nullptr;

	return &game.chars[guii->CharId];
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
		guis_need_update = 1;
	}
}

int InvWindow_GetTopItem(GUIInvWindow *guii) {
	return guii->TopItem;
}

int InvWindow_GetItemsPerRow(GUIInvWindow *guii) {
	return guii->ColCount;
}

int InvWindow_GetItemCount(GUIInvWindow *guii) {
	return charextra[guii->GetCharacterId()].invorder_count;
}

int InvWindow_GetRowCount(GUIInvWindow *guii) {
	return guii->RowCount;
}

void InvWindow_ScrollDown(GUIInvWindow *guii) {
	if ((charextra[guii->GetCharacterId()].invorder_count) >
		(guii->TopItem + (guii->ColCount * guii->RowCount))) {
		guii->TopItem += guii->ColCount;
		guis_need_update = 1;
	}
}

void InvWindow_ScrollUp(GUIInvWindow *guii) {
	if (guii->TopItem > 0) {
		guii->TopItem -= guii->ColCount;
		if (guii->TopItem < 0)
			guii->TopItem = 0;

		guis_need_update = 1;
	}
}

ScriptInvItem *InvWindow_GetItemAtIndex(GUIInvWindow *guii, int index) {
	if ((index < 0) || (index >= charextra[guii->GetCharacterId()].invorder_count))
		return nullptr;
	return &scrInv[charextra[guii->GetCharacterId()].invorder[index]];
}

//=============================================================================

int offset_over_inv(GUIInvWindow *inv) {
	if (inv->ItemWidth <= 0 || inv->ItemHeight <= 0)
		return -1;
	int mover = mouse_ifacebut_xoffs / data_to_game_coord(inv->ItemWidth);
	// if it's off the edge of the visible items, ignore
	if (mover >= inv->ColCount)
		return -1;
	mover += (mouse_ifacebut_yoffs / data_to_game_coord(inv->ItemHeight)) * inv->ColCount;
	if (mover >= inv->ColCount * inv->RowCount)
		return -1;

	mover += inv->TopItem;
	if ((mover < 0) || (mover >= charextra[inv->GetCharacterId()].invorder_count))
		return -1;

	return charextra[inv->GetCharacterId()].invorder[mover];
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
	MAX_ITEMAREA_HEIGHT = ((play.GetUIViewport().GetHeight() - BUTTONAREAHEIGHT) - get_fixed_pixel_size(20));
	in_inv_screen++;
	inv_screen_newroom = -1;

	// Sprites 2041, 2042 and 2043 were hardcoded in the older versions of
	// the engine to be used in the built-in inventory window.
	// If they did not exist engine first fell back to sprites 0, 1, 2 instead.
	// Fun fact: this fallback does not seem to be intentional, and was a
	// coincidental result of SpriteCache incorrectly remembering "last seeked
	// sprite" as 2041/2042/2043 while in fact stream was after sprite 0.
	if (spriteset[2041] == nullptr || spriteset[2042] == nullptr || spriteset[2043] == nullptr)
		debug_script_warn("InventoryScreen: one or more of the inventory screen graphics (sprites 2041, 2042, 2043) does not exist, fallback to sprites 0, 1, 2 instead");
	btn_look_sprite = spriteset[2041] != nullptr ? 2041 : 0;
	btn_select_sprite = spriteset[2042] != nullptr ? 2042 : (spriteset[1] != nullptr ? 1 : 0);
	btn_ok_sprite = spriteset[2043] != nullptr ? 2043 : (spriteset[2] != nullptr ? 2 : 0);

	break_code = 0;
}

int InventoryScreen::Redraw() {
	numitems = 0;
	widest = 0;
	highest = 0;
	if (charextra[game.playercharacter].invorder_count < 0)
		update_invorder();
	if (charextra[game.playercharacter].invorder_count == 0) {
		DisplayMessage(996);
		in_inv_screen--;
		return -1;
	}

	if (inv_screen_newroom >= 0) {
		in_inv_screen--;
		NewRoom(inv_screen_newroom);
		return -1;
	}

	for (int i = 0; i < charextra[game.playercharacter].invorder_count; ++i) {
		if (game.invinfo[charextra[game.playercharacter].invorder[i]].name[0] != 0) {
			dii[numitems].num = charextra[game.playercharacter].invorder[i];
			dii[numitems].sprnum = game.invinfo[charextra[game.playercharacter].invorder[i]].pic;
			int snn = dii[numitems].sprnum;
			if (game.SpriteInfos[snn].Width > widest) widest = game.SpriteInfos[snn].Width;
			if (game.SpriteInfos[snn].Height > highest) highest = game.SpriteInfos[snn].Height;
			numitems++;
		}
	}
	if (numitems != charextra[game.playercharacter].invorder_count)
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
	windowxp = play.GetUIViewport().GetWidth() / 2 - windowwid / 2;
	windowyp = play.GetUIViewport().GetHeight() / 2 - windowhit / 2;
	buttonyp = windowhit - BUTTONAREAHEIGHT;
	bartop = get_fixed_pixel_size(2);
	barxp = get_fixed_pixel_size(2);

	Bitmap *ds = prepare_gui_screen(windowxp, windowyp, windowwid, windowhit, true);
	Draw(ds);
	//ags_domouse(DOMOUSE_ENABLE);
	set_mouse_cursor(cmode);
	wasonitem = -1;
	return 0;
}

void InventoryScreen::Draw(Bitmap *ds) {
	color_t draw_color = ds->GetCompatibleColor(play.sierra_inv_color);
	ds->FillRect(Rect(0, 0, windowwid, windowhit), draw_color);
	draw_color = ds->GetCompatibleColor(0);
	ds->FillRect(Rect(barxp, bartop, windowwid - get_fixed_pixel_size(2), buttonyp - 1), draw_color);
	for (int i = top_item; i < numitems; ++i) {
		if (i >= top_item + num_visible_items)
			break;
		Bitmap *spof = spriteset[dii[i].sprnum];
		wputblock(ds, barxp + 1 + ((i - top_item) % 4) * widest + widest / 2 - spof->GetWidth() / 2,
			bartop + 1 + ((i - top_item) / 4) * highest + highest / 2 - spof->GetHeight() / 2, spof, 1);
	}
#define BUTTONWID Math::Max(1, game.SpriteInfos[btn_select_sprite].Width)
	// Draw select, look and OK buttons
	wputblock(ds, 2, buttonyp + get_fixed_pixel_size(2), spriteset[btn_look_sprite], 1);
	wputblock(ds, 3 + BUTTONWID, buttonyp + get_fixed_pixel_size(2), spriteset[btn_select_sprite], 1);
	wputblock(ds, 4 + BUTTONWID * 2, buttonyp + get_fixed_pixel_size(2), spriteset[btn_ok_sprite], 1);

	// Draw Up and Down buttons if required
	Bitmap *arrowblock = BitmapHelper::CreateTransparentBitmap(ARROWBUTTONWID, ARROWBUTTONWID);
	draw_color = arrowblock->GetCompatibleColor(0);
	if (play.sierra_inv_color == 0)
		draw_color = ds->GetCompatibleColor(14);

	arrowblock->DrawLine(Line(ARROWBUTTONWID / 2, 2, ARROWBUTTONWID - 2, 9), draw_color);
	arrowblock->DrawLine(Line(ARROWBUTTONWID / 2, 2, 2, 9), draw_color);
	arrowblock->DrawLine(Line(2, 9, ARROWBUTTONWID - 2, 9), draw_color);
	arrowblock->FloodFill(ARROWBUTTONWID / 2, 4, draw_color);

	if (top_item > 0)
		wputblock(ds, windowwid - ARROWBUTTONWID, buttonyp + get_fixed_pixel_size(2), arrowblock, 1);
	if (top_item + num_visible_items < numitems)
		arrowblock->FlipBlt(arrowblock, windowwid - ARROWBUTTONWID, buttonyp + get_fixed_pixel_size(4) + ARROWBUTTONWID, Shared::kBitmap_VFlip);
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
	int kgn;
	if (run_service_key_controls(kgn) && !play.IsIgnoringInput()) {
		return false; // end inventory screen loop
	}

	update_audio_system_on_game_loop();
	refresh_gui_screen();

	// NOTE: this is because old code was working with full game screen
	const int mouseX = _G(mousex) - windowxp;
	const int mouseY = _G(mousey) - windowyp;

	int isonitem = ((mouseY - bartop) / highest) * ICONSPERLINE + (mouseX - barxp) / widest;
	if (mouseY <= bartop) isonitem = -1;
	else if (isonitem >= 0) isonitem += top_item;
	if ((isonitem < 0) | (isonitem >= numitems) | (isonitem >= top_item + num_visible_items))
		isonitem = -1;

	int mclick, mwheelz;
	if (!run_service_mb_controls(mclick, mwheelz) || play.IsIgnoringInput()) {
		mclick = NONE;
	}

	if (mclick == LEFT) {
		if ((mouseY < 0) | (mouseY > windowhit) | (mouseX < 0) | (mouseX > windowwid))
			return true; // continue inventory screen loop
		if (mouseY < buttonyp) {
			int clickedon = isonitem;
			if (clickedon < 0) return true; // continue inventory screen loop
			evblocknum = dii[clickedon].num;
			play.used_inv_on = dii[clickedon].num;

			if (cmode == MODE_LOOK) {
				//ags_domouse(DOMOUSE_DISABLE);
				run_event_block_inv(dii[clickedon].num, 0);
				// in case the script did anything to the screen, redraw it
				UpdateGameOnce();

				break_code = Redraw();
				return break_code == 0;
			} else if (cmode == MODE_USE) {
				// use objects on each other
				play.usedinv = toret;

				// set the activeinv so the script can check it
				int activeinvwas = playerchar->activeinv;
				playerchar->activeinv = toret;

				//ags_domouse(DOMOUSE_DISABLE);
				run_event_block_inv(dii[clickedon].num, 3);

				// if the script didn't change it, then put it back
				if (playerchar->activeinv == toret)
					playerchar->activeinv = activeinvwas;

				// in case the script did anything to the screen, redraw it
				UpdateGameOnce();

				// They used the active item and lost it
				if (playerchar->inv[toret] < 1) {
					cmode = CURS_ARROW;
					set_mouse_cursor(cmode);
					toret = -1;
				}

				break_code = Redraw();
				return break_code == 0;
			}
			toret = dii[clickedon].num;
			//        int plusng=play.using; play.using=toret;
			update_inv_cursor(toret);
			set_mouse_cursor(MODE_USE);
			cmode = MODE_USE;
			//        play.using=plusng;
			//        break;
			return true; // continue inventory screen loop
		} else {
			if (mouseX >= windowwid - ARROWBUTTONWID) {
				if (mouseY < buttonyp + get_fixed_pixel_size(2) + ARROWBUTTONWID) {
					if (top_item > 0) {
						top_item -= ICONSPERLINE;
						//ags_domouse(DOMOUSE_DISABLE);

						break_code = Redraw();
						return break_code == 0;
					}
				} else if ((mouseY < buttonyp + get_fixed_pixel_size(4) + ARROWBUTTONWID * 2) && (top_item + num_visible_items < numitems)) {
					top_item += ICONSPERLINE;
					//ags_domouse(DOMOUSE_DISABLE);

					break_code = Redraw();
					return break_code == 0;
				}
				return true; // continue inventory screen loop
			}

			int buton = mouseX - 2;
			if (buton < 0) return true; // continue inventory screen loop
			buton /= BUTTONWID;
			if (buton >= 3) return true; // continue inventory screen loop
			if (buton == 0) {
				toret = -1;
				cmode = MODE_LOOK;
			} else if (buton == 1) {
				cmode = CURS_ARROW;
				toret = -1;
			} else {
				return false; // end inventory screen loop
			}
			set_mouse_cursor(cmode);
		}
	} else if (mclick == RIGHT) {
		if (cmode == CURS_ARROW)
			cmode = MODE_LOOK;
		else
			cmode = CURS_ARROW;
		toret = -1;
		set_mouse_cursor(cmode);
	} else if (isonitem != wasonitem) {
		//ags_domouse(DOMOUSE_DISABLE);
		RedrawOverItem(get_gui_screen(), isonitem);
		//ags_domouse(DOMOUSE_ENABLE);
	}
	wasonitem = isonitem;

	update_polled_stuff_if_runtime();

	WaitForNextFrame();

	return true; // continue inventory screen loop
}

void InventoryScreen::Close() {
	clear_gui_screen();
	set_default_cursor();
	invalidate_screen();
	in_inv_screen--;
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
	playerchar->activeinv = selt;
	guis_need_update = 1;
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
	API_OBJCALL_OBJ(GUIInvWindow, CharacterInfo, ccDynamicCharacter, InvWindow_GetCharacterToUse);
}

// void (GUIInvWindow *guii, CharacterInfo *chaa)
RuntimeScriptValue Sc_InvWindow_SetCharacterToUse(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(GUIInvWindow, InvWindow_SetCharacterToUse, CharacterInfo);
}

// ScriptInvItem* (GUIInvWindow *guii, int index)
RuntimeScriptValue Sc_InvWindow_GetItemAtIndex(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_PINT(GUIInvWindow, ScriptInvItem, ccDynamicInv, InvWindow_GetItemAtIndex);
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

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("InvWindow::ScrollDown^0", (void *)InvWindow_ScrollDown);
	ccAddExternalFunctionForPlugin("InvWindow::ScrollUp^0", (void *)InvWindow_ScrollUp);
	ccAddExternalFunctionForPlugin("InvWindow::get_CharacterToUse", (void *)InvWindow_GetCharacterToUse);
	ccAddExternalFunctionForPlugin("InvWindow::set_CharacterToUse", (void *)InvWindow_SetCharacterToUse);
	ccAddExternalFunctionForPlugin("InvWindow::geti_ItemAtIndex", (void *)InvWindow_GetItemAtIndex);
	ccAddExternalFunctionForPlugin("InvWindow::get_ItemCount", (void *)InvWindow_GetItemCount);
	ccAddExternalFunctionForPlugin("InvWindow::get_ItemHeight", (void *)InvWindow_GetItemHeight);
	ccAddExternalFunctionForPlugin("InvWindow::set_ItemHeight", (void *)InvWindow_SetItemHeight);
	ccAddExternalFunctionForPlugin("InvWindow::get_ItemWidth", (void *)InvWindow_GetItemWidth);
	ccAddExternalFunctionForPlugin("InvWindow::set_ItemWidth", (void *)InvWindow_SetItemWidth);
	ccAddExternalFunctionForPlugin("InvWindow::get_ItemsPerRow", (void *)InvWindow_GetItemsPerRow);
	ccAddExternalFunctionForPlugin("InvWindow::get_RowCount", (void *)InvWindow_GetRowCount);
	ccAddExternalFunctionForPlugin("InvWindow::get_TopItem", (void *)InvWindow_GetTopItem);
	ccAddExternalFunctionForPlugin("InvWindow::set_TopItem", (void *)InvWindow_SetTopItem);
}

} // namespace AGS3
