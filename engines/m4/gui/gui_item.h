
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

#ifndef M4_GUI_GUI_ITEM_H
#define M4_GUI_GUI_ITEM_H

#include "m4/m4_types.h"
#include "m4/graphics/gr_font.h"

namespace M4 {

#define ITEM_NORMAL     0x00000
#define ITEM_PRESSED    0x00001

#define ITEM_DEFAULT    0x00001
#define ITEM_RETURN     0x00002

#define SU_PRESSED      0x0100
#define SD_PRESSED      0x0200
#define PU_PRESSED      0x0400
#define PD_PRESSED      0x0800
#define THUMB_PRESSED   0x1000
#define BOX_PRESSED     0x2000
#define AREA_PRESSED    0x3f00

#define NO_SCROLL       0x0000
#define SCROLLABLE      0x0001
#define PAGEABLE        0x0002

#define FILL_INTERIOR 1	// a flag for use by DrawTile
#define BORDER 0

enum ItemType {
	MESSAGE, PICTURE, BUTTON, LISTBOX, TEXTFIELD, REPEAT_BUTTON, DIALOGBOX
};

struct ListItem {
	char   prompt[80];
	int32  tag;
	struct ListItem *next;
	struct ListItem *prev;
};

struct TextItem {
	TextItem *next;
	int32 justification;
	int32 x;
	int32 y;
	int32 w;
	int32 h;
	int32 tag;
	int32 type;
	char *prompt;
	M4CALLBACK	callback;
};

struct Item {
	Item *next;
	Item *prev;
	M4CALLBACK callback;
	ItemType type;
	int32 status;
	Font *myFont;
	char *prompt;
	char *aux;
	char *aux2;
	int32 promptMax;
	int32 myListCount;
	int32 viewIndex;
	int32 thumbY;
	ListItem *myList;
	ListItem *currItem;
	ListItem *viewTop;
	ListItem *viewBottom;
	int32 listView;
	int32 x, y, w, h;
	int32 tag;
};

struct Item_Globals {
	char *origPrompt = nullptr;
	char *undoPrompt = nullptr;
	char *undoAux = nullptr;
	char *undoAux2 = nullptr;
	Item *currTextField = nullptr;

	int32 buttonWidth = 0;
	int32 buttonHeight = 0;

	char clipBoard[100] = { 0 };
};

extern bool InitItems(void);
extern Item *Item_create(Item *parent, enum ItemType type, int32 tag, M4CALLBACK cb);
extern void Item_destroy(Item *myItem);
extern void Item_empty_list(Item *myItem);
extern Item *ItemAdd(Item *itemList, int32 x, int32 y, int32 w, int32 h, char *prompt, int32 tag,
	ItemType type, M4CALLBACK cb, int32 promptMax);
extern Item *ItemFind(Item *itemList, int32 tag);
extern bool Item_SetViewBottom(Item *i);
extern bool ListItemExists(Item *myItem, char *prompt, int32 listTag);
extern bool ListItemAdd(Item *myItem, char *prompt, int32 listTag, int32 addMode, ListItem *changedItem);
extern bool ListItemDelete(Item *myItem, ListItem *myListItem, int32 listTag);
extern bool ListItemChange(Item *myItem, ListItem *myListItem, int32 listTag,
	char *newPrompt, int32 newTag, int32 changeMode);
extern void ViewCurrListItem(Item *myItem);
extern ListItem *ListItemFind(Item *myItem, int32 searchMode, char *searchStr, int32 parm1);
extern bool ListItemSearch(Item *myItem, int32 searchMode, char *searchStr, int32 parm1);
extern bool DoubleClickOnListBox(Item *myItem, int32 xOffset, int32 yOffset);
extern bool ClickOnListBox(Item *myItem, int32 xOffset, int32 yOffset, int32 scrollType);
extern bool ResetDefaultListBox(Item *myItem);
extern bool Item_change_prompt(Item *myItem, char *newPrompt);
extern void Item_ClearOrigPrompt(void);
extern Item *Item_RestoreTextField(void);
extern Item *Item_CheckTextField(void);
extern void Item_SaveTextField(Item *myItem);
extern void SetTextBlockBegin(Item *myItem, int32 relXPos);
extern void SetTextBlockEnd(Item *myItem, int32 relXPos);
extern bool Item_TextEdit(Item *myItem, int32 parm1);
extern bool GetNextListItem(Item *myItem);
extern bool GetNextPageList(Item *myItem);
extern bool GetPrevListItem(Item *myItem);
extern bool GetPrevPageList(Item *myItem);
extern bool Item_show(Item *i, void *bdrDialog, Buffer *scrBuf, int32 itemType);
extern void Item_format(Item *i);
extern Item *Item_set_default(Item *itemList, Item *currDefault, int32 tag);
extern Item *Item_set_pressed(Item *itemList, Item *myItem, int32 tag);
extern Item *Item_set_unpressed(Item *itemList, Item *myItem, int32 tag);
extern Item *Item_set_cancel(Item *itemList, int32 tag);
extern Item *Item_set_next_default(Item *currDefault, Item *itemList);
extern Item *Item_set_prev_default(Item *currDefault, Item *listBottom);

} // End of namespace M4

#endif
