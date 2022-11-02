/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_FRAME_WINDOW_H
#define BURIED_FRAME_WINDOW_H

#include "buried/window.h"

namespace Buried {

class BuriedEngine;
struct GlobalFlags;

class FrameWindow : public Window {
public:
	FrameWindow(BuriedEngine *vm);
	~FrameWindow();

	bool showTitleSequence();
	bool showMainMenu();
	bool returnToMainMenu();
	bool playMovie(const Common::String &background, const Common::String &movie, int movieLeft, int movieTop);
	bool showClosingScreen();
	bool showFeaturesScreen();
	bool startNewGame(bool walkthrough = false, bool introMovie = false);
	bool showDeathScene(int deathSceneIndex, GlobalFlags &globalFlags, Common::Array<int> itemArray);
	bool showCompletionScene(GlobalFlags &globalFlags);
	bool showCredits();
	bool showOverview();
	bool setTimerPause(bool pause);

	bool onEraseBackground();
	void onKeyDown(const Common::KeyState &key, uint flags);
	void onKeyUp(const Common::KeyState &key, uint flags);
	void onTimer(uint timer);
	void onKillFocus(Window *newWindow);
	void onSetFocus(Window *oldWindow);

	int getTransitionSpeed() const { return _transitionSpeed; }
	void setTransitionSpeed(int newSpeed);
	bool isFrameCyclingDefault() const { return _cycleDefault; }
	bool isFrameCachingAllowed() const { return _cacheFrames; }

	bool _reviewerMode;
	bool _controlDown;

	bool isGameInProgress() const { return _gameInProgress; }
	Window *getMainChildWindow() const { return _mainChildWindow; }
	void loadFromState(const Location &location, GlobalFlags &flags, Common::Array<int> inventoryItems);

private:
	Window *_mainChildWindow;

	bool _cacheFrames;
	bool _cycleDefault;
	int _transitionSpeed;
	bool _gameInProgress;
	bool _atMainMenu;
};

} // End of namespace Buried

#endif
