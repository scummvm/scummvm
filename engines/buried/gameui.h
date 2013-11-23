/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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

#ifndef BURIED_GAMEUI_H
#define BURIED_GAMEUI_H

#include "buried/navdata.h"
#include "buried/window.h"

namespace Graphics {
struct Surface;
}

namespace Buried {

class BioChipRightWindow;
class InventoryWindow;
class LiveTextWindow;
class NavArrowWindow;
class SceneViewWindow;

class GameUIWindow : public Window {
public:
	GameUIWindow(BuriedEngine *vm, Window *parent);
	~GameUIWindow();

	bool startNewGame(bool walkthrough = false);
	bool startNewGameIntro(bool walkthrough = false);
	bool startNewGame(const Location &startingLocation);
	bool startNewGame(const Common::String &fileName);
	// startNewGame(continue data, location struct);
	bool loadGame();
	bool loadGame(const Common::String &fileName);
	bool saveGame();
	bool changeCurrentDate(int timeZoneID);
	bool flashWarningLight();
	bool setWarningState(bool newState);

	void onPaint();
	void onEnable(bool enable);
	void onKeyUp(const Common::KeyState &key, uint flags);

	NavArrowWindow *_navArrowWindow;
	LiveTextWindow *_liveTextWindow;
	SceneViewWindow *_sceneViewWindow;
	InventoryWindow *_inventoryWindow;
	BioChipRightWindow *_bioChipRightWindow;

private:
	int _currentDateDisplay;
	bool _warningLightDisplayed;
	bool _doNotDraw;
};

} // End of namespace Buried

#endif
