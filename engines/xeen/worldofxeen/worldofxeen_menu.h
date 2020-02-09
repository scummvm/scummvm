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
#include "xeen/dialogs/dialogs.h"
#include "common/array.h"

namespace Xeen {
namespace WorldOfXeen {

class MenuContainerDialog;

class MainMenuContainer {
private:
	uint _animateCtr;
	uint _frameCount;
	Common::Array<SpriteResource> _backgroundSprites;
	MenuContainerDialog *_dialog;
protected:
	/**
	 * Draws the main menu background
	 */
	void draw();

	/**
	 * Called when the menu screen is first shown
	 */
	virtual void display() = 0;

	/**
	 * Shows the main menu dialog
	 */
	virtual void showMenuDialog() = 0;
public:
	/**
	 * Show the main menu for the correct game
	 */
	static void show();
public:
	/**
	 * Constructor
	 */
	MainMenuContainer(const char *spritesName1, const char *spritesName2 = nullptr, const char *spritesName3 = nullptr);

	/**
	 * Destructor
	 */
	virtual ~MainMenuContainer();

	/**
	 * Execute the menu
	 */
	void execute();

	/**
	 * Sets the dialog being displayed in the menu
	 */
	void setOwner(MenuContainerDialog *dlalog) {
		_dialog = dlalog;
	}
};

class CloudsMainMenuContainer : public MainMenuContainer {
protected:
	/**
	 * Called when the menu screen is first shown
	 */
	void display() override;

	/**
	 * Shows the main menu dialog
	 */
	void showMenuDialog() override;
public:
	CloudsMainMenuContainer();
};

class DarkSideMainMenuContainer : public MainMenuContainer {
private:
	SpriteResource _background;
protected:
	/**
	 * Called when the menu screen is first shown
	 */
	void display() override;

	/**
	* Shows the main menu dialog
	*/
	void showMenuDialog() override;
public:
	DarkSideMainMenuContainer();
};

class WorldOfXeenMainMenuContainer : public MainMenuContainer {
protected:
	/**
	 * Called when the menu screen is first shown
	 */
	void display() override;

	/**
	* Shows the main menu dialog
	*/
	void showMenuDialog() override;
public:
	WorldOfXeenMainMenuContainer();
};

class WorldOfXeenCDMainMenuContainer : public MainMenuContainer {
protected:
	/**
	 * Called when the menu screen is first shown
	 */
	void display() override;

	/**
	* Shows the main menu dialog
	*/
	void showMenuDialog() override;
public:
	WorldOfXeenCDMainMenuContainer();
};

class MenuContainerDialog : public ButtonContainer {
protected:
	MainMenuContainer *_owner;
public:
	/**
	 * Constructor
	 */
	MenuContainerDialog(MainMenuContainer *owner) : ButtonContainer(g_vm), _owner(owner) {}

	/**
	 * Destructor
	 */
	~MenuContainerDialog() override {
		_owner->setOwner(nullptr);
	}

	/**
	 * Draws the dialog
	 */
	virtual void draw() = 0;

	/**
	 * Handles events
	 */
	virtual bool handleEvents() = 0;
};

class MainMenuDialog : public MenuContainerDialog {
public:
	/**
	 * Constructor
	 */
	MainMenuDialog(MainMenuContainer *owner) : MenuContainerDialog(owner) {}

	/**
	 * Destructor
	 */
	~MainMenuDialog() override {}

	/**
	 * Draws the dialog
	 */
	void draw() override = 0;

	/**
	 * Handles events
	 */
	bool handleEvents() override;

};

class CloudsMenuDialog : public MainMenuDialog {
private:
	SpriteResource _buttonSprites;
private:
	/**
	 * Loads buttons for the dialog
	 */
	void loadButtons();
public:
	/**
	 * Constructor
	 */
	CloudsMenuDialog(MainMenuContainer *owner);

	/**
	 * Destructor
	 */
	~CloudsMenuDialog() override;

	/**
	 * Draws the dialog
	 */
	void draw() override;

	/**
	 * Handles events
	 */
	bool handleEvents() override;
};

class DarkSideMenuDialog : public MainMenuDialog {
private:
	SpriteResource _buttonSprites;
	bool _firstDraw;
private:
	/**
	 * Loads buttons for the dialog
	 */
	void loadButtons();
public:
	/**
	 * Constructor
	 */
	DarkSideMenuDialog(MainMenuContainer *owner);

	/**
	 * Destructor
	 */
	~DarkSideMenuDialog() override;

	/**
	 * Draws the dialog
	 */
	void draw() override;

	/**
	 * Handles events
	 */
	bool handleEvents() override;
};

class WorldMenuDialog : public MainMenuDialog {
private:
	SpriteResource _buttonSprites;
private:
	/**
	* Loads buttons for the dialog
	*/
	void loadButtons();
public:
	/**
	 * Constructor
	 */
	WorldMenuDialog(MainMenuContainer *owner);

	/**
	* Destructor
	*/
	~WorldMenuDialog() override;

	/**
	 * Draws the dialog
	 */
	void draw() override;

	/**
	 * Handles events
	 */
	bool handleEvents() override;
};

class OtherOptionsDialog : public MenuContainerDialog {
private:
	SpriteResource _buttonSprites;
private:
	/**
	* Loads buttons for the dialog
	*/
	void loadButtons();
public:
	/**
	 * Constructor
	 */
	OtherOptionsDialog(MainMenuContainer *owner);

	/**
	 * Destructor
	 */
	~OtherOptionsDialog() override;

	/**
	 * Draws the dialog
	 */
	void draw() override;

	/**
	 * Handles events
	 */
	bool handleEvents() override;
};

} // End of namespace WorldOfXeen
} // End of namespace Xeen

#endif /* XEEN_WORLDOFXEEN_WORLDOFXEEN_MENU_H */
