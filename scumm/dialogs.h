/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef SCUMM_DIALOGS_H
#define SCUMM_DIALOGS_H

#include "common/str.h"
#include "gui/about.h"
#include "gui/dialog.h"
#include "gui/options.h"
#include "gui/widget.h"

#ifndef DISABLE_HELP
#include "scumm/help.h"
#endif

namespace GUI {
	class ListWidget;
}


namespace Scumm {

class ScummEngine;

class ScummDialog : public GUI::Dialog {
public:
	ScummDialog(ScummEngine *scumm, int x, int y, int w, int h)
		: GUI::Dialog(x, y, w, h), _scumm(scumm) {}
	
protected:
	typedef Common::String String;

	ScummEngine *_scumm;

	// Query a string from the resources
	const String queryResString(int stringno);
};

class SaveLoadChooser;

class MainMenuDialog : public ScummDialog {
public:
	MainMenuDialog(ScummEngine *scumm);
	~MainMenuDialog();
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	virtual void open();	
	virtual void close();

protected:
	GUI::Dialog		*_aboutDialog;
#ifndef DISABLE_HELP
	GUI::Dialog		*_helpDialog;
#endif
	SaveLoadChooser	*_saveDialog;
	SaveLoadChooser	*_loadDialog;

	void save();
	void load();
};

#ifndef DISABLE_HELP

class HelpDialog : public ScummDialog {
public:
	HelpDialog(ScummEngine *scumm);
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

protected:
	typedef Common::String String;

	GUI::ButtonWidget *_nextButton;
	GUI::ButtonWidget *_prevButton;

	GUI::StaticTextWidget *_title;
	GUI::StaticTextWidget *_key[HELP_NUM_LINES];
	GUI::StaticTextWidget *_dsc[HELP_NUM_LINES];

	int _page;
	int _numPages;

	void displayKeyBindings();
};

#endif

class ConfigDialog : public GUI::OptionsDialog {
protected:
	ScummEngine *_scumm;
#ifdef _WIN32_WCE
	GUI::Dialog		*_keysDialog;
#endif

public:
	ConfigDialog(ScummEngine *scumm);
	~ConfigDialog();

	virtual void open();
	virtual void close();
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

protected:
	GUI::CheckboxWidget *subtitlesCheckbox;
};

class InfoDialog : public ScummDialog {
public:
	// arbitrary message
	InfoDialog(ScummEngine *scumm, const String& message);
	// from resources
	InfoDialog(ScummEngine *scumm, int res);

	virtual void handleMouseDown(int x, int y, int button, int clickCount) { 
		close();
	}
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers) {
		setResult(ascii);
		close();
	}

protected:
	void setInfoText (const String& message);
};

class PauseDialog : public InfoDialog {
public:
	PauseDialog(ScummEngine *scumm);
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers)
		{
			if (ascii == ' ')  // Close pause dialog if space key is pressed
				close();
			else
				ScummDialog::handleKeyDown(ascii, keycode, modifiers);
		}
};

class ConfirmExitDialog : public InfoDialog {
public:
	ConfirmExitDialog(ScummEngine *scumm);
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers);
};

class ConfirmRestartDialog : public InfoDialog {
public:
	ConfirmRestartDialog(ScummEngine *scumm);
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers);
};

#ifdef _WIN32_WCE

class KeysDialog : public ScummDialog {
public:
	KeysDialog(ScummEngine *scumm);

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers);

protected:

	GUI::ListWidget		 *_actionsList;
	GUI::StaticTextWidget *_actionTitle;
	GUI::StaticTextWidget *_keyMapping;
	int				 _actionSelected;
};

#endif

} // End of namespace Scumm

#endif
