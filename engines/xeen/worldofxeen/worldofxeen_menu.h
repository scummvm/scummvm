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

#ifndef XEEN_WORLDOFXEEN_WORLDOFXEEN_MENU_H
#define XEEN_WORLDOFXEEN_WORLDOFXEEN_MENU_H

#include "xeen/xeen.h"
#include "xeen/dialogs.h"

namespace Xeen {
namespace WorldOfXeen {

class WorldOfXeenMenu : public SettingsBaseDialog {
private:
	void execute();
protected:
	WorldOfXeenMenu(XeenEngine *vm) : SettingsBaseDialog(vm) {}
protected:
	virtual void startup(Common::String &title1, Common::String &title2) = 0;

	virtual void setBackground(bool doFade) {}

	virtual void showTitles1(SpriteResource &sprites);

	virtual void showTitles2();

	virtual void setupButtons(SpriteResource *buttons);

	/**
	 * Opens the menu window
	 */
	virtual void openWindow() {}

	/**
	 * Closes the menu window
	 */
	virtual void closeWindow() {}
public:
	virtual ~WorldOfXeenMenu() {}

	static void show(XeenEngine *vm);
};

class CloudsOptionsMenu : public WorldOfXeenMenu {
protected:
	virtual void startup(Common::String &title1, Common::String &title2);
public:
	CloudsOptionsMenu(XeenEngine *vm) : WorldOfXeenMenu(vm) {}

	virtual ~CloudsOptionsMenu() {}
};

class DarkSideOptionsMenu : public WorldOfXeenMenu {
protected:
	virtual void startup(Common::String &title1, Common::String &title2);
public:
	DarkSideOptionsMenu(XeenEngine *vm) : WorldOfXeenMenu(vm) {}

	virtual ~DarkSideOptionsMenu() {}
};

class WorldOptionsMenu : public DarkSideOptionsMenu {
private:
	int _bgFrame;
protected:
	virtual void startup(Common::String &title1, Common::String &title2);

	virtual void setBackground(bool doFade);

	virtual void showTitles2() {}

	virtual void setupButtons(SpriteResource *buttons);

	/**
	 * Opens the menu window
	 */
	virtual void openWindow();

	/**
	 * Closes the menu window
	 */
	virtual void closeWindow();

	virtual void showContents(SpriteResource &title1, bool mode);
public:
	WorldOptionsMenu(XeenEngine *vm) : DarkSideOptionsMenu(vm), _bgFrame(0) {}

	virtual ~WorldOptionsMenu() {}
};

} // End of namespace WorldOfXeen
} // End of namespace Xeen

#endif /* XEEN_WORLDOFXEEN_WORLDOFXEEN_MENU_H */
