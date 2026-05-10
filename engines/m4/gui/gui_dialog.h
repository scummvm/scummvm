
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

#ifndef M4_GUI_GUI_DIALOG_H
#define M4_GUI_GUI_DIALOG_H

#include "m4/m4_types.h"
#include "m4/graphics/gr_buff.h"
#include "m4/gui/gui.h"
#include "m4/gui/gui_item.h"
#include "m4/gui/gui_univ.h"

namespace M4 {

enum {
	TS_GIVEN = 0, TS_JUST_LEFT, TS_JUST_RIGHT, TS_CENTRE
};

struct Dialog {
	int32   w, h;
	int32   num_items;
	Item *itemList;
	Item *listBottom;
	Item *cancel_item, *return_item, *default_item;
	GrBuff *dlgBuffer;

	// General support methods
	void destroy();
	void configure(int32 defaultTag, int32 returnTag, int32 cancelTag);
	void show();

	// Add methods
	bool addMessage(int32 x, int32 y, const char *prompt, int32 tag);
	bool addButton(int32 x, int32 y, const char *prompt, M4CALLBACK cb, int32 tag);

	// Item Fields
	Item *getItem(int32 tag);

	// Text Fields
	bool addTextField(int32 x1, int32 y1, int32 x2, const char *defaultPrompt, M4CALLBACK cb, int32 tag, int32 fieldLength);
};

struct TextScrn {
	int32 w, h;
	int32 textColor;
	int32 textColor_alt1;
	int32 textColor_alt2;
	int32 hiliteColor;
	int32 hiliteColor_alt1;
	int32 hiliteColor_alt2;
	int32 luminance;
	Font *myFont;
	TextItem *myTextItems;
	TextItem *hiliteItem;
	GrBuff *textScrnBuffer;
};

struct Dialog_Globals {
	bool okButton = false;
	//event handler vars
	bool movingScreen = false;
	Item *clickItem = nullptr;
	Item *doubleClickItem = nullptr;
	char listboxSearchStr[80] = { 0 };
};

bool gui_dialog_init();
void gui_dialog_shutdown();

//GENERAL DIALOG SUPPORT
Dialog *DialogCreateAbsolute(int32 x1, int32 y1, int32 x2, int32 y2, uint32 scrnFlags);
void vmng_Dialog_Destroy(Dialog *d);     //used only by viewmgr.cpp **DO NOT USE
void DialogDestroy(Dialog *d, M4Rect *r = nullptr);
void Dialog_Refresh(Dialog *d);
void Dialog_Refresh_All();
void Dialog_Configure(Dialog *d, int32 defaultTag, int32 returnTag, int32 cancelTag);
void Dialog_SetDefault(Dialog *d, int32 tag);
bool Dialog_SetPressed(Dialog *d, int32 tag);

//MESSAGE TYPE SUPPORT
bool Dialog_Add_Message(Dialog *d, int32 x, int32 y, const char *prompt, int32 tag);

//BUTTON TYPE SUPPORT
bool Dialog_Add_Button(Dialog *d, int32 x, int32 y, const char *prompt, M4CALLBACK cb, int32 tag);

//LIST TYPE SUPPORT
void Dialog_GetPrevListItem(Dialog *d);
void Dialog_GetNextListItem(Dialog *d);

//TEXTFIELD TYPE SUPPORT
bool Dialog_Add_TextField(Dialog *d, int32 x1, int32 y1, int32 x2, const char *defaultPrompt, M4CALLBACK cb, int32 tag, int32 fieldLength);

//GENERAL ITEM SUPPORT
Item *Dialog_Get_Item(Dialog *d, int32 tag);
void Dialog_Change_Item_Prompt(Dialog *d, const char *newPrompt, Item *myItem, int32 tag);
void Dialog_Refresh_Item(Dialog *d, Item *myItem, int32 tag);
void Dialog_KeyMouseCollision();

bool sizeofGUIelement_border(int16 el_type, int32 *w, int32 *h);
bool sizeofGUIelement_interior(ButtonDrawRec *bdr, M4Rect *myRect);
bool drawGUIelement(ButtonDrawRec *bdr, M4Rect *myRect);

//----------------------------------------------------------------------------------------
//TEXTSCRN STUFF...
TextScrn *TextScrn_Create(int32 x1, int32 y1, int32 x2, int32 y2, int32 luminance, uint32 scrnFlags,
	int32 textColor, int32 hiliteColor,
	int32 textColor_alt1 = 0, int32 hiliteColor_alt1 = 0,
	int32 textColor_alt2 = 0, int32 hiliteColor_alt2 = 0);
void vmng_TextScrn_Destroy(TextScrn *myTextScrn);
void TextScrn_Destroy(TextScrn *myTextScrn);
void TextScrn_Activate(TextScrn *myTextScrn);
bool TextScrn_Add_TextItem(TextScrn *myTextScrn, int32 x, int32 y, int32 tag,
	int32 justification, const char *prompt, M4CALLBACK callback);
bool TextScrn_Add_Message(TextScrn *myTextScrn, int32 x, int32 y, int32 tag,
	int32 justification, const char *prompt);

} // End of namespace M4

#endif
