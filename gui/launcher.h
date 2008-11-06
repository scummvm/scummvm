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
 * $URL$
 * $Id$
 */

#ifndef LAUNCHER_DIALOG_H
#define LAUNCHER_DIALOG_H

#include "gui/dialog.h"
#include "engines/game.h"
#include "engines/metaengine.h"
#include "common/str.h"

namespace GUI {

class BrowserDialog;
class ListWidget;
class GraphicsWidget;
class SaveLoadChooser;

Common::String addGameToConf(const GameDescriptor &result);

class LauncherDialog : public Dialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
public:
	LauncherDialog();
	~LauncherDialog();

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	virtual void handleKeyDown(Common::KeyState state);
	virtual void handleKeyUp(Common::KeyState state);

protected:
	ListWidget		*_list;
	ButtonWidget	*_addButton;
	Widget			*_startButton;
	Widget			*_loadButton;
	Widget			*_editButton;
	Widget			*_removeButton;
#ifndef DISABLE_FANCY_THEMES
	GraphicsWidget		*_logo;
#endif
	StringList		_domains;
	BrowserDialog	*_browser;
	SaveLoadChooser	*_loadDialog;

	virtual void reflowLayout();

	void updateListing();
	void updateButtons();

	void open();
	void close();
	virtual void addGame();
	void removeGame(int item);
	void editGame(int item);
	void loadGame(int item);
	
	void selectGame(const String &name);
};

class SaveLoadChooser : public GUI::Dialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
protected:
	GUI::ListWidget		*_list;
	GUI::ButtonWidget	*_chooseButton;
	GUI::ButtonWidget	*_deleteButton;
	GUI::GraphicsWidget	*_gfxWidget;
	GUI::ContainerWidget	*_container;
	GUI::StaticTextWidget	*_date;
	GUI::StaticTextWidget	*_time;
	GUI::StaticTextWidget	*_playtime;

	const EnginePlugin		*_plugin;
	bool					_delSupport;
	bool					_metaInfoSupport;
	bool					_thumbnailSupport;
	bool					_saveDateSupport;
	bool					_playTimeSupport;
	String					_target;
	SaveStateList			_saveList;

	uint8 _fillR, _fillG, _fillB;

	void updateSaveList();
	void updateSelection(bool redraw);
public:
	SaveLoadChooser(const String &title, const String &buttonLabel);
	~SaveLoadChooser();

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	void setList(const StringList& list);
	int runModal(const EnginePlugin *plugin, const String &target);

	const Common::String& getResultString() const;
	void setSaveMode(bool saveMode);

	virtual void reflowLayout();

	virtual void close();
};

} // End of namespace GUI

#endif
