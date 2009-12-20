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
 * $URL$
 * $Id$
 *
 */

#ifndef M4_MADS_MENUS_H
#define M4_MADS_MENUS_H

#include "m4/viewmgr.h"

namespace M4 {

#define MADS_MENU_ANIM_DELAY 70

enum MadsGameAction {START_GAME, RESUME_GAME, SHOW_INTRO, CREDITS, QUOTES, EXIT};

class RexMainMenuView: public View {
private:
	M4Surface *_bgSurface;
	RGBList *_bgPalData;
	int _menuItemIndex;
	int _frameIndex;
	bool _skipFlag;
	SpriteAsset *_menuItem;
	Common::Array<RGBList *> _itemPalData;
	uint32 _delayTimeout;
	int _highlightedIndex;

	int getHighlightedItem(int x, int y);
	void handleAction(MadsGameAction action);
public:
	RexMainMenuView(M4Engine *vm);
	~RexMainMenuView();

	bool onEvent(M4EventType eventType, int32 param, int x, int y, bool &captureEvents);
	void updateState();
};

class DragonMainMenuView: public View {
private:
	//M4Surface *_bgSurface;
	RGBList *_bgPalData;
	int _menuItemIndex;
	int _frameIndex;
	bool _skipFlag;
	SpriteAsset *_menuItem;
	Common::Array<RGBList *> _itemPalData;
	uint32 _delayTimeout;
	int _highlightedIndex;

	int getHighlightedItem(int x, int y);
	void handleAction(MadsGameAction action);
public:
	DragonMainMenuView(M4Engine *vm);
	~DragonMainMenuView();

	bool onEvent(M4EventType eventType, int32 param, int x, int y, bool &captureEvents);
	void updateState();
};

class MadsMainMenuView: public View {
public:
	MadsMainMenuView(M4Engine *vm);

	bool onEvent(M4EventType eventType, int32 param, int x, int y, bool &captureEvents);
	void updateState();
};

}

#endif
