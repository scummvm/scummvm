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

#ifndef STARK_UI_MENU_SAVELOAD_MENU_H
#define STARK_UI_MENU_SAVELOAD_MENU_H

#include "engines/stark/ui/menu/locationscreen.h"
#include "engines/stark/visual/text.h"

#include "common/error.h"

namespace Stark {

namespace Gfx {
class Bitmap;
class SurfaceRenderer;
}

class SaveDataWidget;

/**
 * The base class of the save and load menu of the game
 */
class SaveLoadMenuScreen : public StaticLocationScreen {
public:
	static const int _slotPerRow = 3;
	static const int _slotPerColumn = 3;
	static const int _slotPerPage = 9;

	SaveLoadMenuScreen(Gfx::Driver *gfx, Cursor *cursor, Screen::Name screenName);
	virtual ~SaveLoadMenuScreen();

	// StaticLocationScreen API
	void open() override;
	void close() override;

	/** Called when a SaveDataWidget is selected */
	virtual void onWidgetSelected(SaveDataWidget *widget) = 0;

	/** Check whether the menu is used for saving */
	virtual bool isSaveMenu() = 0;

protected:
	static void checkError(Common::Error error);

	enum WidgetIndex {
		kWidgetSaveText = 3,
		kWidgetLoadText = 4,
		kWidgetBack = 5,
		kWidgetNext = 6
	};

private:
	// Start from zero
	int _page;
	int _maxPage;

	void backHandler();

	void prevPageHandler() { changePage(_page - 1); }
	void nextPageHandler() { changePage(_page + 1); }

	void removeSaveDataWidgets();
	void loadSaveData(int page);
	void changePage(int page);
	int computeMaxPage();
};

/**
 * The save menu of the game
 */
class SaveMenuScreen : public SaveLoadMenuScreen {
public:
	SaveMenuScreen(Gfx::Driver *gfx, Cursor *cursor) :
			SaveLoadMenuScreen(gfx, cursor, Screen::kScreenSaveMenu),
			_slotToSaveAfterConfirm(nullptr) {}
	virtual ~SaveMenuScreen() {}

	// SaveLoadMenuScreen API
	void open() override;

	void onWidgetSelected(SaveDataWidget *widget) override;

	bool isSaveMenu() override { return true; }

private:
	void saveGameToSlot(SaveDataWidget *widget);
	void saveConfirmSlot();

	SaveDataWidget *_slotToSaveAfterConfirm;
};

/**
 * The load menu of the game
 */
class LoadMenuScreen : public SaveLoadMenuScreen {
public:
	LoadMenuScreen(Gfx::Driver *gfx, Cursor *cursor) :
			SaveLoadMenuScreen(gfx, cursor, Screen::kScreenLoadMenu),
			_slotToLoadAfterConfirm(-1) {}
	~LoadMenuScreen() override {}

	// SaveLoadMenuScreen API
	void open() override;

	void onWidgetSelected(SaveDataWidget *widget) override;

	bool isSaveMenu() override { return false; }

private:
	void loadConfirmSlot();

	int _slotToLoadAfterConfirm;
};

/**
 * The widget of save data
 */
class SaveDataWidget : public StaticLocationWidget {
public:
	SaveDataWidget(int slot, Gfx::Driver *gfx, SaveLoadMenuScreen *screen);
	~SaveDataWidget();

	// StaticLocationWidget API
	void render() override;
	bool isMouseInside(const Common::Point &mousePos) const override;
	void onClick() override;
	void onMouseMove(const Common::Point &mousePos) override;
	void onScreenChanged() override;

	int getSlot() { return _slot; }
	Common::String getName() { return _name; }

	/** Load the thumbnail and info from the save data */
	void loadSaveDataElements();

	/** Check whether the save slot has data */
	bool hasSave() { return _hasSave; }

private:
	const Gfx::Color _outlineColor = Gfx::Color(0x1E, 0x1E, 0x96);
	const Gfx::Color _textColor = Gfx::Color(0x5C, 0x48, 0x3D);

	int _slot;
	SaveLoadMenuScreen *_screen;

	Common::Point _thumbPos, _textDescPos, _textTimePos;
	int _thumbWidth, _thumbHeight;

	Gfx::Bitmap *_bitmap;
	Gfx::Bitmap *_outline;
	Gfx::SurfaceRenderer *_surfaceRenderer;

	VisualText _textDesc, _textTime;

	bool _isMouseHovered;
	bool _hasSave;

	Common::String _name;
};

} // End of namespace Stark

#endif // STARK_UI_MENU_SETTING_MENU_H
