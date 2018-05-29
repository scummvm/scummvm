/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARK_UI_MENU_SAVELOAD_MENU_H
#define STARK_UI_MENU_SAVELOAD_MENU_H

#include "engines/stark/ui/menu/locationscreen.h"

#include "common/error.h"
#include "engines/engine.h"

namespace Stark {

namespace Gfx {
class Texture;
class SurfaceRenderer;	
}


/**
 * The base class of the save and load menu of the game
 */
class SaveLoadMenuScreen : public StaticLocationScreen {
public:
	SaveLoadMenuScreen(Gfx::Driver *gfx, Cursor *cursor, Screen::Name screenName);
	virtual ~SaveLoadMenuScreen();

	// StaticLocationScreen API
	void open() override;

	void clickHandler(int slot) {
		clickHandlerImpl(slot);
	}

protected:
	static void checkError(Common::Error error);

	enum WidgetIndex {
		kWidgetSaveText = 3,
		kWidgetLoadText = 4,
	};

	virtual void clickHandlerImpl(int slot) = 0;

private:
	static const uint32 _textColorBlack = 0xFF000000;

	void backHandler();
};

/**
 * The save menu of the game
 */
class SaveMenuScreen : public SaveLoadMenuScreen {
public:
	SaveMenuScreen(Gfx::Driver *gfx, Cursor *cursor) : 
			SaveLoadMenuScreen(gfx, cursor, Screen::kScreenSaveMenu) {}
	virtual ~SaveMenuScreen() {}

	// SaveLoadMenuScreen API
	void open() override;

protected:
	void clickHandlerImpl(int slot) override {
		checkError(g_engine->saveGameState(slot, "TestSave"));
	}
};

/**
 * The load menu of the game
 */
class LoadMenuScreen : public SaveLoadMenuScreen {
public:
	LoadMenuScreen(Gfx::Driver *gfx, Cursor *cursor) : 
			SaveLoadMenuScreen(gfx, cursor, Screen::kScreenLoadMenu) {}
	virtual ~LoadMenuScreen() {}

	// SaveLoadMenuScreen API
	void open() override;

protected:
	void clickHandlerImpl(int slot) override {
		checkError(g_engine->loadGameState(slot));
	}
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

private:
	int _slot;
	SaveLoadMenuScreen *_screen;
	Gfx::Texture *_texture;
	Gfx::SurfaceRenderer *_surfaceRenderer;
};

} // End of namespace Stark

#endif // STARK_UI_MENU_SETTING_MENU_H
