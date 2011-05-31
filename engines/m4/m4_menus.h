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
 */

#ifndef M4_M4_MENUS_H
#define M4_M4_MENUS_H

#include "common/list.h"
#include "common/ptr.h"

#include "m4/viewmgr.h"
#include "m4/m4.h"
#include "m4/gui.h"
#include "m4/saveload.h"

namespace M4 {

#define M4_DIALOG_FADE_STEPS 5
#define M4_DIALOG_FADE_DELAY 30

typedef Common::List<MenuObject *> MenuObjectList;

class OrionMenuView : public DialogView {
	typedef MenuObjectList::iterator MenuObjectsIterator;
private:
	MenuType _menuType;
	SpriteAsset *_sprites;
	MenuObjectList _menuObjects;
	MenuObject *_currentItem;
	typedef void (*Callback)(OrionMenuView *view);
	OrionMenuView::Callback _escapeHandler, _returnHandler;
	bool _closeFlag;
	bool _calledFromMainMenu;
	bool _interfaceWasVisible;
	int _firstSlotIndex;

	bool loadSprites(const char *seriesName);
	M4Surface *createThumbnail();
	void destroyView();
public:
	OrionMenuView(MadsM4Engine *vm, int x, int y, MenuType menuType, bool calledFromMainMenu,
		bool loadSaveFromHotkey);
	~OrionMenuView();
	MenuType getMenuType() { return _menuType; }
	SpriteAsset *sprites() { return _sprites; }
	MenuObjectList &items() { return _menuObjects; }
	MenuObject *getItem(int objectId);
	void setTopSaveSlot(int slotNumber);
	void refresh(const Common::Rect &areaRect);
	void close() { _closeFlag = true; }

	bool onEvent(M4EventType eventType, int32 param, int x, int y, bool &captureEvents);

	int _originalMidiVolume;
	SaveGameList *_saveNames;
	bool _loadSaveFromHotkey;
};

class OrionCallbacks {
public:
	static void closeMenuFn(DialogView *view, MenuObject *item);
	static void closeMenuFn(OrionMenuView *view);
	static void gameOptionsMenuFn(DialogView *view, MenuObject *item);
	static void gameSaveGameFn(DialogView *view, MenuObject *item);
	static void gameLoadGameFn(DialogView *view, MenuObject *item);
	static void gameExitFn(DialogView *view, MenuObject *item);
	static void optionsDigiSliderFn(DialogView *view, MenuObject *item);
	static void optionsMidiSliderFn(DialogView *view, MenuObject *item);
	static void optionsScrollingFn(DialogView *view, MenuObject *item);
	static void optionsCancelFn(DialogView *view, MenuObject *item);
	static void optionsDoneFn(DialogView *view, MenuObject *item);
	static void optionsReturnFn(OrionMenuView *view);
	static void optionsEscapeFn(OrionMenuView *view);
	static void saveLoadSaveFn(DialogView *view, MenuObject *item);
	static void saveLoadLoadFn(DialogView *view, MenuObject *item);
	static void saveLoadSlotFn(DialogView *view, MenuObject *item);
	static void saveLoadCancelFn(DialogView *view, MenuObject *item);
	static void saveLoadSliderFn(DialogView *view, MenuObject *item);
	static void saveLoadEscapeFn(OrionMenuView *view);
	static void saveLoadReturnFn(OrionMenuView *view);
};

}

#endif
