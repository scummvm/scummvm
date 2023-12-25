
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
	void refresh();
	void resize(int32 newW, int32 newH);
	void configure(int32 defaultTag, int32 returnTag, int32 cancelTag);
	void setDefault(int32 tag);
	bool setPressed(int32 tag);
	void show();

	// Add methods
	bool addMessage(int32 x, int32 y, const char *prompt, int32 tag);
	bool addPicture(int32 x, int32 y, Buffer *myBuff, int32 tag);
	bool addButton(int32 x, int32 y, const char *prompt, M4CALLBACK cb, int32 tag);
	bool addRepeatButton(int32 x, int32 y, const char *prompt, M4CALLBACK cb, int32 tag);
	bool addList(int32 x1, int32 y1, int32 x2, int32 y2, M4CALLBACK cb, int32 tag);

	// Item Fields
	Item *getItem(int32 tag);
	void changeItemPrompt(const char *newPrompt, Item *myItem, int32 tag);
	bool removeItem(Item *myItem, int32 tag);
	void refreshItem(Item *myItem, int32 tag);

	// Text Fields
	bool addTextField(int32 x1, int32 y1, int32 x2, const char *defaultPrompt, M4CALLBACK cb, int32 tag, int32 fieldLength);
	void registerTextField();

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
Dialog *DialogCreate(M4Rect *r, uint32 scrnFlags);
void vmng_Dialog_Destroy(Dialog *d);     //used only by viewmgr.cpp **DO NOT USE
void DialogDestroy(Dialog *d, M4Rect *r = nullptr);
void Dialog_Refresh(Dialog *d);
void Dialog_Refresh_All();
void Dialog_Resize(Dialog *d, int32 newW, int32 newH);
bool GetDialogCoords(Dialog *d, M4Rect *r);
void Dialog_Configure(Dialog *d, int32 defaultTag, int32 returnTag, int32 cancelTag);
void Dialog_SetDefault(Dialog *d, int32 tag);
bool Dialog_SetPressed(Dialog *d, int32 tag);

//MESSAGE TYPE SUPPORT
bool Dialog_Add_Message(Dialog *d, int32 x, int32 y, const char *prompt, int32 tag);

//PICTURE TYPE SUPPORT
bool Dialog_Add_Picture(Dialog *d, int32 x, int32 y, Buffer *myBuff, int32 tag);

//BUTTON TYPE SUPPORT
bool Dialog_Add_Button(Dialog *d, int32 x, int32 y, const char *prompt, M4CALLBACK cb, int32 tag);
bool Dialog_Add_RepeatButton(Dialog *d, int32 x, int32 y, const char *prompt, M4CALLBACK cb, int32 tag);

//LIST TYPE SUPPORT
bool Dialog_Add_List(Dialog *d, int32 x1, int32 y1, int32 x2, int32 y2, M4CALLBACK cb, int32 tag);
bool Dialog_Add_DirList(Dialog *d, int32 x1, int32 y1, int32 x2, int32 y2, M4CALLBACK cb, int32 tag, char *myDir, char *myTypes);
bool Dialog_Change_DirList(Dialog *d, Item *myItem, const char *myDir, const char *myTypes);
bool Dialog_Add_List_Item(Dialog *d, Item *myItem, const char *prompt, int32 tag, int32 listTag, int32 addMode, bool refresh);
bool Dialog_Delete_List_Item(Dialog *d, Item *myItem, int32 tag, ListItem *myListItem, int32 listTag);
bool Dialog_Change_List_Item(Dialog *d, Item *myItem, int32 tag, ListItem *myListItem, int32 listTag, char *newPrompt, int32 newListTag, int32 changeMode, bool refresh);
void Dialog_EmptyListBox(Dialog *d, Item *i, int32 tag);
char *Dialog_GetListItemPrompt(Dialog *d, Item *i, int32 tag, int32 listTag);
ListItem *Dialog_GetCurrListItem(Dialog *d, Item *i, int32 tag);
char *Dialog_GetCurrListItemPrompt(Dialog *d, Item *i, int32 tag);
bool Dialog_GetCurrListItemTag(Dialog *d, Item *i, int32 tag, int32 *listTag);
bool Dialog_ListItemExists(Dialog *d, Item *myItem, int32 tag, char *prompt, int32 listTag);
bool Dialog_ListboxSearch(Dialog *d, Item *myItem, int32 tag, int32 searchMode, char *searchStr, int32 parm1);
void Dialog_GetPrevListItem(Dialog *d);
void Dialog_GetNextListItem(Dialog *d);

//TEXTFIELD TYPE SUPPORT
bool Dialog_Add_TextField(Dialog *d, int32 x1, int32 y1, int32 x2, const char *defaultPrompt, M4CALLBACK cb, int32 tag, int32 fieldLength);
void Dialog_RegisterTextField(Dialog *d);

//HOTKEY SUPPORT
bool Dialog_Add_Key(Dialog *d, long myKey, HotkeyCB cb);
bool Dialog_Remove_Key(Dialog *d, long myKey);

//GENERAL ITEM SUPPORT
Item *Dialog_Get_Item(Dialog *d, int32 tag);
void Dialog_Change_Item_Prompt(Dialog *d, const char *newPrompt, Item *myItem, int32 tag);
bool Dialog_Remove_Item(Dialog *d, Item *myItem, int32 tag);
void Dialog_Refresh_Item(Dialog *d, Item *myItem, int32 tag);
void Dialog_KeyMouseCollision();

void Dialog_SystemError(char *s);

bool sizeofGUIelement_border(int16 el_type, int32 *w, int32 *h);
bool sizeofGUIelement_interior(ButtonDrawRec *bdr, M4Rect *myRect);
bool drawGUIelement(ButtonDrawRec *bdr, M4Rect *myRect);

bool custom_drawGUIelement(ButtonDrawRec *bdr, M4Rect *myRect);
bool custom_sizeofGUIelement_border(int16 el_type, int32 *w, int32 *h);
bool custom_sizeofGUIelement_interior(ButtonDrawRec *bdr, M4Rect *myRect);

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
	int32 justification, const char *prompt, M4CALLBACK callback);
bool TextScrn_Add_Message(TextScrn *myTextScrn, int32 x, int32 y, int32 tag,
	int32 justification, const char *prompt);
void TextScrn_Change_TextItem(TextScrn *myTextScrn, int32 tag, char *prompt, uint8 color);
void TextScrn_Delete_TextItem(TextScrn *myTextScrn, int32 tag);

} // End of namespace M4

#endif
