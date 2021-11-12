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

#ifndef GUI_LAUNCHER_DIALOG_H
#define GUI_LAUNCHER_DIALOG_H

// Disable the grid for platforms that disable fancy themes
#ifdef DISABLE_FANCY_THEMES
#define DISABLE_LAUNCHERDISPLAY_GRID
#endif
#define kSwitchLauncherDialog -2

#include "gui/dialog.h"
#include "gui/widgets/popup.h"
#include "gui/MetadataParser.h"

#include "engines/game.h"

namespace GUI {

enum LauncherDisplayType {
	kLauncherDisplayList = 1,
	kLauncherDisplayGrid = 2
};

enum GroupingMethod {
	kGroupByNone,
	kGroupByFirstLetter,
	kGroupByEngine,
	kGroupBySeries,
	kGroupByCompany,
	kGroupByLanguage,
	kGroupByPlatform
};

class BrowserDialog;
class CommandSender;
class GroupedListWidget;
class ContainerWidget;
class EntryContainerWidget;
class GridWidget;
class ButtonWidget;
class PicButtonWidget;
class GraphicsWidget;
class StaticTextWidget;
class EditTextWidget;
class SaveLoadChooser;
class PopUpWidget;

class LauncherDialog : public Dialog {
public:
	LauncherDialog(const Common::String &dialogName);
	~LauncherDialog() override;

	void rebuild();

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

	virtual LauncherDisplayType getType() const = 0;

	int run();

	void handleKeyDown(Common::KeyState state) override;
	void handleKeyUp(Common::KeyState state) override;
	void handleOtherEvent(const Common::Event &evt) override;
	bool doGameDetection(const Common::String &path);
	Common::String getGameConfig(int item, Common::String key);
protected:
	EditTextWidget  *_searchWidget;
#ifndef DISABLE_FANCY_THEMES
	GraphicsWidget		*_logo;
	GraphicsWidget		*_searchPic;
#endif
	StaticTextWidget	*_searchDesc;
	ButtonWidget	*_searchClearButton;
	ButtonWidget	*_addButton;
	Widget			*_removeButton;
	Widget			*_startButton;
	ButtonWidget	*_loadButton;
	Widget			*_editButton;
	Common::StringArray		_domains;
	BrowserDialog	*_browser;
	SaveLoadChooser	*_loadDialog;
	PopUpWidget		*_grpChooserPopup;
	StaticTextWidget	*_grpChooserDesc;
	GroupingMethod	_groupBy;
	Common::String	_title;
	Common::String	_search;
	MetadataParser	_metadataParser;

#ifndef DISABLE_LAUNCHERDISPLAY_GRID
	ButtonWidget		*_listButton;
	ButtonWidget		*_gridButton;

	/**
	 * Create two buttons to choose between grid display and list display
	 * in the launcher.
	 */
	void addLayoutChooserButtons();
	ButtonWidget *createSwitchButton(const Common::String &name, const Common::U32String &desc, const Common::U32String &tooltip, const char *image, uint32 cmd = 0);
#endif // !DISABLE_LAUNCHERDISPLAY_GRID

	void reflowLayout() override;

	/**
	 * Fill the list widget with all currently configured targets, and trigger
	 * a redraw.
	 */
	virtual void updateListing() = 0;

	virtual void updateButtons() = 0;

	virtual void build();
	void clean();

	void open() override;
	void close() override;

	/**
	 * Handle "Add game..." button.
	 */
	virtual void addGame();
	void massAddGame();

	/**
	 * Handle "Remove game..." button.
	 */
	void removeGame(int item);

	/**
	 * Handle "Edit game..." button.
	 */
	void editGame(int item);

	/**
	 * Handle "Record..." button.
	 */
	void recordGame(int item);

	/**
	 * Handle "Load..." button.
	 */
	void loadGame(int item);

	/**
	 * Select the target with the given name in the launcher game list.
	 * Also scrolls the list so that the newly selected item is visible.
	 *
	 * @target	name of target to select
	 */
	virtual void selectTarget(const Common::String &target) = 0;
	virtual int getSelected() = 0;
private:
	bool checkModifier(int modifier);
};

class LauncherChooser {
protected:
	LauncherDialog *_impl;

public:
	LauncherChooser();
	~LauncherChooser();

	int runModal();
	void selectLauncher();
};

class LauncherSimple : public LauncherDialog {
public:
	LauncherSimple(const Common::String &title);

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleKeyDown(Common::KeyState state) override;

	LauncherDisplayType getType() const override { return kLauncherDisplayList; }

protected:
	void updateListing() override;
	void groupEntries(const Common::Array<const Common::ConfigManager::Domain *> &metadata);
	void updateButtons() override;
	void selectTarget(const Common::String &target) override;
	int getSelected() override;
	void build() override;
private:
	GroupedListWidget 		*_list;
};

#ifndef DISABLE_LAUNCHERDISPLAY_GRID
class LauncherGrid : public LauncherDialog {
public:
	LauncherGrid(const Common::String &title);

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleKeyDown(Common::KeyState state) override;

	LauncherDisplayType getType() const override { return kLauncherDisplayGrid; }

protected:
	void updateListing() override;
	void groupEntries(const Common::Array<const Common::ConfigManager::Domain *> &metadata);
	void updateButtons() override;
	void selectTarget(const Common::String &target) override;
	int getSelected() override;
	void build() override;
private:
	GridWidget		*_grid;
};
#endif // !DISABLE_LAUNCHERDISPLAY_GRID

} // End of namespace GUI

#endif
