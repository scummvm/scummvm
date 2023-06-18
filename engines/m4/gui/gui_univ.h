
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

#ifndef M4_GUI_GUI_UNIV_H
#define M4_GUI_GUI_UNIV_H

#include "m4/m4_types.h"
#include "m4/gui/gui.h"
#include "m4/graphics/gr_font.h"
#include "m4/graphics/gr_buff.h"

namespace M4 {

typedef void (*RefreshFunc)(void *myScreen, void *theRectList, void *destBuffer, int32 destX, int32 destY);
typedef void (*HotkeyCB)(void *myParam, void *myContent);
typedef bool (*EventHandler)(void *scrnContent, int32 eventType,
	int32 parm1, int32 parm2, int32 parm3, bool *currScreen);

struct Hotkey {
	Hotkey *next;
	int32 myKey;
	HotkeyCB callback;
};

/**
 * The main structure used by the view manager is the ScreenContext
 */
struct ScreenContext {
	ScreenContext *infront;
	ScreenContext *behind;
	int32			 		 x1, y1, x2, y2;
	int32					 scrnType;
	uint32					 scrnFlags;
	void *scrnContent;
	RefreshFunc				 redraw;
	EventHandler			 evtHandler;
	Hotkey *scrnHotkeys;
};

struct ButtonDrawRec {
	void *dialog;
	Buffer *scrBuf;
	M4sprite *sprite;
	int32 x1, y1, x2, y2;
	int16 el_type;
	bool fillMe, pressed;
};

enum ItemType {
	MESSAGE, PICTURE, BUTTON, LISTBOX, TEXTFIELD, REPEAT_BUTTON, DIALOGBOX
};

struct ListItem {
	char   prompt[80];
	int32  tag;
	struct ListItem *next;
	struct ListItem *prev;
};

struct Item {
	struct Item *next;
	struct Item *prev;
	M4CALLBACK     callback;
	enum ItemType  type;
	int32          status;
	Font *myFont;
	char *prompt;
	char *aux;
	char *aux2;
	int32          promptMax;
	int32          myListCount;
	int32          viewIndex;
	int32          thumbY;
	ListItem *myList;
	ListItem *currItem;
	ListItem *viewTop;
	ListItem *viewBottom;
	int32          listView;
	int32          x, y, w, h;
	int32          tag;
};

struct TextItem {
	struct TextItem *next;
	int32			justification;
	int32			x;
	int32			y;
	int32			w;
	int32			h;
	int32			tag;
	int32			type;
	char *prompt;
	M4CALLBACK	callback;
};

} // End of namespace M4

#endif
