
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

struct Dialog {
	int32   w, h;
	int32   num_items;
	Item *itemList;
	Item *listBottom;
	Item *cancel_item, *return_item, *default_item;
	GrBuff *dlgBuffer;
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

extern bool gui_dialog_init();
extern void gui_dialog_shutdown();

//GENERAL DIALOG SUPPORT
extern Dialog *DialogCreateAbsolute(int32 x1, int32 y1, int32 x2, int32 y2, uint32 scrnFlags);
extern Dialog *DialogCreate(M4Rect *r, uint32 scrnFlags);
extern void vmng_Dialog_Destroy(Dialog *d);     //used only by viewmgr.cpp **DO NOT USE
extern void DialogDestroy(Dialog *d, M4Rect *r);
extern void Dialog_Refresh(Dialog *d);
extern void Dialog_Refresh_All();
extern void Dialog_Resize(Dialog *d, int32 newW, int32 newH);
extern bool GetDialogCoords(Dialog *d, M4Rect *r);
extern void Dialog_Configure(Dialog *d, int32 defaultTag, int32 returnTag, int32 cancelTag);
extern void Dialog_SetDefault(Dialog *d, int32 tag);
extern bool Dialog_SetPressed(Dialog *d, int32 tag);

//MESSAGE TYPE SUPPORT
extern bool Dialog_Add_Message(Dialog *d, int32 x, int32 y, char *prompt, int32 tag);

//PICTURE TYPE SUPPORT
extern bool Dialog_Add_Picture(Dialog *d, int32 x, int32 y, Buffer *myBuff, int32 tag);

//BUTTON TYPE SUPPORT
extern bool Dialog_Add_Button(Dialog *d, int32 x, int32 y, char *prompt, M4CALLBACK cb, int32 tag);
extern bool Dialog_Add_RepeatButton(Dialog *d, int32 x, int32 y, char *prompt, M4CALLBACK cb, int32 tag);

//LIST TYPE SUPPORT
extern bool Dialog_Add_List(Dialog *d, int32 x1, int32 y1, int32 x2, int32 y2, M4CALLBACK cb, int32 tag);
extern bool Dialog_Add_DirList(Dialog *d, int32 x1, int32 y1, int32 x2, int32 y2, M4CALLBACK cb, int32 tag, char *myDir, char *myTypes);
extern bool Dialog_Change_DirList(Dialog *d, Item *myItem, char *myDir, char *myTypes);
extern bool Dialog_Add_List_Item(Dialog *d, Item *myItem, char *prompt, int32 tag, int32 listTag, int32 addMode, bool refresh);
extern bool Dialog_Delete_List_Item(Dialog *d, Item *myItem, int32 tag, ListItem *myListItem, int32 listTag);
extern bool Dialog_Change_List_Item(Dialog *d, Item *myItem, int32 tag, ListItem *myListItem, int32 listTag, char *newPrompt, int32 newListTag, int32 changeMode, bool refresh);
extern void Dialog_EmptyListBox(Dialog *d, Item *i, int32 tag);
extern char *Dialog_GetListItemPrompt(Dialog *d, Item *i, int32 tag, int32 listTag);
extern ListItem *Dialog_GetCurrListItem(Dialog *d, Item *i, int32 tag);
extern char *Dialog_GetCurrListItemPrompt(Dialog *d, Item *i, int32 tag);
extern bool Dialog_GetCurrListItemTag(Dialog *d, Item *i, int32 tag, int32 *listTag);
extern bool Dialog_ListItemExists(Dialog *d, Item *myItem, int32 tag, char *prompt, int32 listTag);
extern bool Dialog_ListboxSearch(Dialog *d, Item *myItem, int32 tag, int32 searchMode, char *searchStr, int32 parm1);
extern void Dialog_GetPrevListItem(Dialog *d);
extern void Dialog_GetNextListItem(Dialog *d);

//TEXTFIELD TYPE SUPPORT
extern bool Dialog_Add_TextField(Dialog *d, int32 x1, int32 y1, int32 x2, char *defaultPrompt, M4CALLBACK cb, int32 tag, int32 fieldLength);
extern void Dialog_RegisterTextField(Dialog *d);

//HOTKEY SUPPORT
extern bool Dialog_Add_Key(Dialog *d, long myKey, HotkeyCB cb);
extern bool Dialog_Remove_Key(Dialog *d, long myKey);

//GENERAL ITEM SUPPORT
extern Item *Dialog_Get_Item(Dialog *d, int32 tag);
extern void Dialog_Change_Item_Prompt(Dialog *d, char *newPrompt, Item *myItem, int32 tag);
extern bool Dialog_Remove_Item(Dialog *d, Item *myItem, int32 tag);
extern void Dialog_Refresh_Item(Dialog *d, Item *myItem, int32 tag);
extern void Dialog_KeyMouseCollision();

extern void Dialog_SystemError(char *s);

extern bool sizeofGUIelement_border(int16 el_type, int32 *w, int32 *h);
extern bool sizeofGUIelement_interior(ButtonDrawRec *bdr, M4Rect *myRect);
extern bool drawGUIelement(ButtonDrawRec *bdr, M4Rect *myRect);

extern bool custom_drawGUIelement(ButtonDrawRec *bdr, M4Rect *myRect);
extern bool custom_sizeofGUIelement_border(int16 el_type, int32 *w, int32 *h);
extern bool custom_sizeofGUIelement_interior(ButtonDrawRec *bdr, M4Rect *myRect);

//----------------------------------------------------------------------------------------
//TEXTSCRN STUFF...
TextScrn *TextScrn_Create(int32 x1, int32 y1, int32 x2, int32 y2, int32 luminance, uint32 scrnFlags,
	int32 textColor, int32 hiliteColor,
	int32 textColor_alt1 = 0, int32 hiliteColor_alt1 = 0,
	int32 textColor_alt2 = 0, int32 hiliteColor_alt2 = 0);
void vmng_TextScrn_Destroy(TextScrn *myTextScrn);
void TextScrn_Destroy(TextScrn *myTextScrn);
void TextScrn_Activate(TextScrn *myTextScrn);
bool TextScrn_Add_Key(TextScrn *myTextScrn, long myKey, HotkeyCB cb);
bool TextScrn_Add_TextItem(TextScrn *myTextScrn, int32 x, int32 y, int32 tag,
	int32 justification, char *prompt, M4CALLBACK callback);
bool TextScrn_Add_Message(TextScrn *myTextScrn, int32 x, int32 y, int32 tag,
	int32 justification, char *prompt);
void TextScrn_Change_TextItem(TextScrn *myTextScrn, int32 tag, char *prompt, uint8 color);
void TextScrn_Delete_TextItem(TextScrn *myTextScrn, int32 tag);

} // End of namespace M4

#endif
