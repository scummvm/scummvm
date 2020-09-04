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

#ifndef GLOBAL_DIALOGS_H
#define GLOBAL_DIALOGS_H

#include "gui/dialog.h"
#include "gui/options.h"
#include "gui/widget.h"

class Engine;

namespace GUI {
class ButtonWidget;
class CommandSender;
class GraphicsWidget;
class SaveLoadChooser;
}

class MainMenuDialog : public GUI::Dialog {
public:
	enum {
		kSaveCmd = 'SAVE',
		kLoadCmd = 'LOAD',
		kPlayCmd = 'PLAY',
		kOptionsCmd = 'OPTN',
		kHelpCmd = 'HELP',
		kAboutCmd = 'ABOU',
		kQuitCmd = 'QUIT',
		kLauncherCmd = 'LNCR',
		kChooseCmd = 'CHOS'
	};

public:
	MainMenuDialog(Engine *engine);
	~MainMenuDialog();

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

	virtual void reflowLayout();

protected:
	void save();
	void load();

protected:
	Engine *_engine;

	GUI::GraphicsWidget  *_logo;

	GUI::ButtonWidget    *_returnToLauncherButton;
	GUI::ButtonWidget    *_loadButton;
	GUI::ButtonWidget    *_saveButton;
	GUI::ButtonWidget    *_helpButton;

	GUI::Dialog          *_aboutDialog;

	GUI::SaveLoadChooser *_loadDialog;
	GUI::SaveLoadChooser *_saveDialog;
};

namespace GUI {

class ConfigDialog : public OptionsDialog {
public:
	ConfigDialog();
	~ConfigDialog() override;

	// OptionsDialog API
	void build() override;
	void apply() override;

private:
	OptionsContainerWidget *_engineOptions;

#ifdef GUI_ENABLE_KEYSDIALOG
public:
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	Dialog *_keysDialog;
#endif
};

class ExtraGuiOptionsWidget : public OptionsContainerWidget {
public:
	ExtraGuiOptionsWidget(GuiObject *widgetsBoss, const Common::String &name, const Common::String &domain, const ExtraGuiOptions &options);
	~ExtraGuiOptionsWidget() override;

	// OptionsContainerWidget API
	void load() override;
	bool save() override;

private:
	typedef Common::Array<CheckboxWidget *> CheckboxWidgetList;

	static Common::String dialogLayout(const Common::String &domain);

	ExtraGuiOptions _options;
	CheckboxWidgetList _checkboxes;
};

} // End of namespace GUI

#endif
