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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://www.switchlink.se/svn/picture/resource.h $
 * $Id: resource.h 2 2008-08-04 12:18:15Z johndoe $
 *
 */

#ifndef PICTURE_MENU_H
#define PICTURE_MENU_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiocd.h"

#include "engines/engine.h"

namespace Picture {

const byte kFontColorMenuDefault	= 229;
const byte kFontColorMenuActive		= 255;
const uint kFontResourceIndex		= 13;

enum MenuID {
	kMenuIdNone,
	kMenuIdMain,
	kMenuIdSave,
	kMenuIdLoad,
	kMenuIdVolumes
};

enum ItemID {
	kItemIdNone,
	// Main menu
	kItemIdSave,
	kItemIdLoad,
	kItemIdToggleText,
	kItemIdToggleVoices,
	kItemIdVolumesMenu,
	kItemIdPlay,
	kItemIdQuit,
	// Volumes menu
	// TODO: Up/down buttons
	kItemIdDone,
	kItemIdCancel,
	// Save/load menu
	// TODO
	kMenuIdDummy
};

class MenuSystem {

public:
	MenuSystem(PictureEngine *vm);
	~MenuSystem();

	void update();
	void handleEvents();
	
protected:

	struct Item {
		Common::Rect rect;
		ItemID id;
		//const byte *caption;
		Common::String caption;
		byte defaultColor, activeColor;
		int x, y, w;
	};

	PictureEngine *_vm;
	Graphics::Surface *_background;

	MenuID _currMenuID, _newMenuID;
	ItemID _currItemID;

	Common::Array<Item> _items;
	
	bool _cfgText, _cfgVoices;

	void addClickTextItem(ItemID id, int x, int y, int w, const byte *caption, byte defaultColor, byte activeColor);

	void drawItem(ItemID itemID, bool active);
	void handleMouseMove(int x, int y);
	void handleMouseClick(int x, int y);
	
	ItemID findItemAt(int x, int y);
	Item *getItem(ItemID id);
	void setItemCaption(Item *item, const byte *caption);

	void initMenu(MenuID menuID);
	
	void enterItem(ItemID id);
	void leaveItem(ItemID id);
	void clickItem(ItemID id);

	void saveBackground();
	void restoreBackground();
	void drawString(int16 x, int16 y, int w, byte color, byte *text);
	
	void setCfgText(bool value, bool active);
	void setCfgVoices(bool value, bool active);

};

} // End of namespace Picture

#endif /* PICTURE_MENU_H */
